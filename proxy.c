/* Proxy:
 * The proxy uses threading to make concurrent requests to servers and stores
  * suitable payload contents in the cache for later refernce. Whenever client
  * requests a page, the proxy first checks in the cache. If a match is found
  * it then sends that cached copy to client.
  * If no match is found it requests the page from server and cache's it 
  * for later reference */

/* Cache: 
 * The cache usues a circular linked list of 'nodes' to keep copy of payloads 
 * recieved from server. Every node also has host, filename and port as 
 * identifiers to match when client requests a file 
 * The cache starts as empty list of two nodes 'start' and 'end' which 
 * server as boundaries for other nodes.
 * New nodes are added to the front of the list to mark them as most recent 
 * If the cache exceeds the max size, last node in the list is removed to make
 * space for new more recent nodes */

#include "csapp.h"
#include <stdbool.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE (1024*1024)
#define MAX_OBJECT_SIZE (100*1024)
#define HOSTLEN 256
#define SERVLEN 8

/* Information about a connected client.  - Taken from tiny.c */ 
typedef struct {
    struct sockaddr_in addr;    // Socket address
    socklen_t addrlen;          // Socket address length
    int connfd;                 // Client connection file descriptor
    char host[HOSTLEN];         // Client host
    char serv[SERVLEN];         // Client service (port)
} client_info;

typedef struct cnode {
	char* payload;
	char* host;
	int* port;
	char* filename;
	size_t* payload_size;
	struct cnode *next;
	struct cnode *prev;
} c_node;


typedef struct clist {
	size_t cache_size;
	c_node *start;
	c_node *end;
} cache_list;

/* Function protoypes - proxy */
int parse_uri(char *uri, char* server, char *filename);
void sigpipe_handler(int sig);
void *concurrent_init(void *vargp);
void serve(int connfd);
void send_request_to_server(rio_t *rio_client, int srcfd);
void forward_content_to_client(rio_t *rio_server, int connfd, char *server, 
									int *port, char *filename);
/* Function protoypes - cache */
cache_list *cache_init();
c_node *build_new_node(char *host, char *filename, int *port, char *payload, 
							size_t *payload_size);
void remove_lru_node(cache_list *c_list);
void add_fresh_node(cache_list *c_list, c_node *fresh);
void update_to_fresh(cache_list *c_list, c_node *update);
c_node *find_fit(cache_list *c_list, char *host, int *port, char *filename);
char *get_payload(cache_list *c_list, char *host, int *port, char *filename, 
					size_t *size);
void add_new_node(cache_list *cl, char *host, int *port, char *filename, 
					char* payload, size_t payload_size);
sem_t mutex;

/* Global variables */
cache_list *c_list = NULL;

/*
######################################
#									 #
#     / Begin Proxy Implementation / #
#									 #
######################################
*/

void sigpipe_handler(int signal) 
{
	(void)signal;
    return;
}

/* Main routine opens connection to client at specified port,
 * handles making of new threads for all concurrent connections */
int main(int argc, char** argv) 
{
   	int listenfd;

  
    /* Need a port number */
    if(argc != 2)
    {
    	fprintf(stderr, "Require's port number\n");
    	return 1;
    }

    pthread_t tid;
   	c_list = cache_init();
    listenfd = Open_listenfd(argv[1]);

    /* Taken from tiny.c */
	while (1) 
	{
        /* Allocate space on the stack for client info */
        client_info client_data;
        client_info *client = &client_data;

        /* Initialize the length of the address */
        client->addrlen = sizeof(client->addr);

        /* Accept() will block until a client connects to the port */
        client->connfd = Accept(listenfd,
                (SA *) &client->addr, &client->addrlen);

        /* Make a new thread for every new request */
        int *cfd = malloc(sizeof(int));
        *cfd = client->connfd;

        Pthread_create(&tid, NULL, concurrent_init, cfd);
    }
    exit(0);
}

/* Handles detatching of every thread and then initiates routines for serving 
 *the client and then closes the connection after request is complete */
void *concurrent_init(void *vargp)
{
	/* Detach thread from main thread and free vargp to avoid race condition */
	int cfd = *((int*)vargp);
	Pthread_detach(pthread_self());
	free(vargp);

	/* Install signal handler for SIGPIPE */
	Signal(SIGPIPE, sigpipe_handler);

	/* Connection is established; serve client */
    serve(cfd);
    Close(cfd);

    return NULL;
}

/* Takes in client identifier, then parses requests from client, 
 *ensures method used by client is GET.
 * It then requests content from cache if available otherwise connects to 
 * server, get content and stores in cache and then sends to client to 
 * complete the request */
void serve(int connfd)
{
	rio_t client_rio, server_rio;
	char buf[MAXLINE];
	char uri[MAXLINE];
	char method[MAXLINE];
	char version;

	/* Associate connfd with client buffer */
	Rio_readinitb(&client_rio, connfd);

	/* Read request header from client buffer */
	if (Rio_readlineb(&client_rio, buf, MAXLINE) <= 0) 
	{
        return;
    }

    /* Taken from tiny.c */
    /* sscanf must parse exactly 3 things for request line to be well-formed */
    /* version must be either HTTP/1.0 or HTTP/1.1 */
    if (sscanf(buf, "%s %s HTTP/1.%c", method, uri, &version) != 3
            || (version != '0' && version != '1')) 
    {
        fprintf(stderr, "400 Bad Request. PROXY received a malformed request");
        return;
    }

     /* Check that the method is GET - Taken from tiny.c*/
    if (strncmp(method, "GET", sizeof("GET"))) 
    {
        fprintf(stderr, "501 Not Implemented.\n");
        fprintf(stderr, "iny does not implement this method\n");
        return;
    }

    /* Parse URI from GET request - Taken from tiny.c */
    char filename[MAXLINE], server[MAXLINE];
    size_t size;
 	/* Get server port, server name, filename by parsing uri */
    int server_port = parse_uri(uri, server, filename);
    /* Search and get paylaod from cache, NULL if not found*/
    char *payload = get_payload(c_list, server, &server_port, filename, &size);

    /* Cache hit - write to client */
    if(payload != NULL)
    {
    	Rio_writen(connfd, payload, size);

    	free(payload);
    }
    /* Cache Miss - request from server and store in cache */
    else
    {
    	/* Connect to server */
	    int srcfd;
	    char port[MAXLINE];
	    sprintf(port, "%d", server_port);;

	    srcfd = Open_clientfd(server, port);

	    /* Send the header line*/
	    char buf[MAXLINE];
		sprintf(buf, "GET /%s HTTP/1.1\r\n", filename);
		Rio_writen(srcfd, buf, strlen(buf));

	    /* Send request received from client to server */
	    send_request_to_server(&client_rio, srcfd);
	    /* Send response from server to client */
	    Rio_readinitb(&server_rio, srcfd);
	    forward_content_to_client(&server_rio, connfd, server, &server_port, 
	    	filename);

	    /* Job done, close connection */
		Close(srcfd);

		return;
    }

}

/* Takes in server identifier, reads request from client buffer and 
 * then writes it to server */
void send_request_to_server(rio_t *rio_client, int srcfd)
{
	char buf[MAXLINE];
	
	/* Forward request header to server */
	while (strcmp(buf, "\r\n") != 0 )
	{
		if ((Rio_readlineb(rio_client, buf, MAXLINE)) == 0)
		{
			return;
		}
		Rio_writen(srcfd, buf, strlen(buf));
	}	
	/* Send final line "\r\n" */
	Rio_writen(srcfd, buf, strlen(buf));

	return;
}

/* Takes in client identifier, server name, port and filename then reads 
 * request header from server buffer and then writes it to client */
void forward_content_to_client(rio_t *rio_server, int connfd, char *server, 
								int *port, char *filename)
{
	char buf[MAX_OBJECT_SIZE];
	int num_bytes;
	char payload[MAX_OBJECT_SIZE];
	size_t bytes_read = 0;
	bool overflow = false;

	/* Read file from server buf and send to client */
	while ((num_bytes = Rio_readnb(rio_server, buf, MAXLINE)) != 0)
	{
		Rio_writen(connfd, buf, num_bytes);
		/* Copy to cache memory as long as bytes_read <= MAX_OBJ_SIZE */
		if(bytes_read <= MAX_OBJECT_SIZE)
		{
			memcpy(payload + bytes_read, buf, sizeof(char)*num_bytes);
			bytes_read += num_bytes;
		}
		/* Set overflow, otherwise */
		else
		{
			overflow = true;
		}
	}

	/* If no overflow occured, make new node with new payload & add to list */
	if(!overflow && c_list != NULL)
	{
		add_new_node(c_list, server, port, filename, payload, bytes_read);
	}
	
	return;
}

/* Parses request uri from client into server -> hostname, 
 * filename -> path of the requested file and
 * returns the service port on the server */
int parse_uri(char* uri, char* server, char* filename)
{
	char *serv;
    char *serv_tmp;
    char *file;

    /* Remove 'http:// from uri */
    serv = uri + strspn(uri, "http://");
    /* Find end of server name */
    serv_tmp = strpbrk(serv, ":/");

    /* Extract server name and copy to server pointer */
    char *ptr = strchr(serv, ':');
    int len = ptr - serv;
    strncpy(server, serv, len);
    
    /* Extract filename */
    file = strchr(serv, '/') + 1;
	strcpy(filename, file);

	/* Extract port number and return it */
    int server_port = 80;
	server_port = atoi(serv_tmp + 1);

    return server_port;
}

/*
######################################
#									 #
#     / Begin Cache Implementation / #
#									 #
######################################
*/

/* Initialize cache_list */
cache_list *cache_init()
{
	cache_list *c_list = (cache_list*)malloc(sizeof(cache_list));

	c_list -> start = (c_node*)malloc(sizeof(c_node));
	c_list -> end = (c_node*)malloc(sizeof(c_node));

	/* Size of cache is zero at start */
	c_list -> cache_size = 0;

	/* Initially, Make start and end nodes point to each other */
	c_list -> start -> next = c_list -> end;
	c_list -> start -> prev = c_list -> end;

	c_list -> end -> next = c_list -> start;
	c_list -> end -> prev = c_list -> start;

	/* Unblock cache */
	V(&mutex);

	return c_list;
}


/* Takes in host, port and filename then makes and returns new node with 
 * specified parameters */
c_node *build_new_node(char *host, char *filename, int *port, 
		char *payload, size_t *payload_size)
{
	c_node *new_node = (c_node*)malloc(sizeof(c_node));
	size_t size = *(payload_size);

	/* Copy the parameters into memory */
	if(host != NULL)
	{
		new_node -> host = (char*)(malloc(sizeof(char*) * strlen(host)));
		strcpy(new_node -> host, host);
	}

	if(filename != NULL)
	{
		new_node -> filename = (char*)(malloc(sizeof(char*)*strlen(filename)));
		strcpy(new_node -> filename, filename);
	}

	if(port != NULL)
	{
		new_node -> port = (int*)(malloc(sizeof(int*)));
		*(new_node -> port) = *port;
	}

	if(payload != NULL)
	{
		new_node -> payload = (char*)(malloc(sizeof(char) * size));
		memcpy(new_node -> payload, payload, sizeof(char)* size);
	}
	
 	if(payload_size != NULL)
 	{
 		new_node -> payload_size = (size_t*)(malloc(sizeof(size_t)));
		*(new_node -> payload_size) = size;
 	}
	
	return new_node;

}

/* Remove (lru) last node from the list */
void remove_lru_node(cache_list *c_list)
{
	if(c_list == NULL)
	{
		return;
	}

	c_node *nd = NULL;
	nd = c_list -> end -> prev;

	/* If list is empty, do nothing */
	if(nd == c_list -> start)
	{
		return;
	}

	/* Remove node by changing connections */
	c_list -> end -> prev = nd -> prev;
	nd -> prev -> next = c_list -> end;

	/* Update size of cache */
	c_list -> cache_size = c_list -> cache_size - *(nd -> payload_size);

	/* Free mempry used by that node */
	free(nd -> host);
	free(nd -> filename);
	free(nd -> port);
	free(nd -> payload);
	free(nd -> payload_size);
	free(nd);

	return;
}

/* Takes in a new node and adds it to the front of the cache list */
void add_fresh_node(cache_list *c_list, c_node *fresh)
{
	if(c_list == NULL || fresh == NULL)
	{
		return;
	}

	c_list -> start -> next -> prev = fresh;
	fresh -> prev = c_list -> start;
	fresh -> next = c_list -> start -> next;
	c_list -> start -> next = fresh;

	return;
}

/* Takes in old node and makes it most recent used by moving it to front */
void update_to_fresh(cache_list *c_list, c_node *update)
{
	if(c_list == NULL || update == NULL)
	{
		return;
	}

	c_node *nd = update;
	if(nd == c_list -> start || nd == c_list -> end)
	{
		return;
	}

	nd -> prev -> next = nd -> next;
	nd -> next -> prev = nd -> prev;

	add_fresh_node(c_list, nd);

	return;
}

/* Takes in host, port and filename and find a matching node.
 * Returns NULL if no such node is found */
c_node *find_fit(cache_list *c_list, char *host, int *port, char *filename)
{
	if(c_list == NULL)
	{
		return NULL;
	}
	c_node *node = c_list -> start;

	/* Iterate through the list and try to find matching parameters */
	for(node = node -> next; node != c_list -> end; node = node -> next)
	{
		if(node != NULL)
		{
			if(!strcmp(node -> host, host) && 
				!strcmp(node -> filename, filename) &&
					*(node -> port) == *port)
			{
				return node;
			}
		}
		else
		{
			continue;
		}
	}
	return NULL;
}

/* Takes in host, port and filename and finds a matching node.
 * It then returns the payload of that matching node to be sent to client */
char *get_payload(cache_list *c_list, char *host, int *port, char *filename, 
		size_t *size)
{
	c_node *node = NULL;

	/* start blocking cache */
	P(&mutex);

	node = find_fit(c_list, host, port, filename);


	if(node == NULL)
	{	
		/* Unblock cache */
		V(&mutex);
		return NULL;
	}
	else
	{
		size_t payload_size = *(node -> payload_size);
		/* Copy payload to new string */
		char *payload = (char*)malloc(sizeof(char)*payload_size);
		memcpy(payload, node -> payload, sizeof(char)*payload_size);
		/* Set size to be read by caller */
		*size = payload_size;

		/* Update lru info of list */
		update_to_fresh(c_list, node);

		/* Unblock cache */
		V(&mutex);

		return payload;
	}
}
/* Takes in host, port and filename and builds a new node.
 * It then adds the node to this list, removing lru node if neccassary */
void add_new_node(cache_list *c_list, char *host, int *port, char *filename, 
					char* payload, size_t payload_size)
{
	/* Start blocking cache */
	P(&mutex);

	/* Make node with the specified paramters */
	c_node *node = NULL;
	node = build_new_node(host, filename, port, payload, &payload_size);

	/* If cache size is less then MAX_CACHE_SIZE then add node without 
	 * removing any other node */
	if(c_list -> cache_size + payload_size <= MAX_CACHE_SIZE)
	{
		add_fresh_node(c_list, node);
		c_list -> cache_size += payload_size;
	}
	/* Remove a node to make space and then add the new node */
	else
	{
		remove_lru_node(c_list);
		add_fresh_node(c_list, node);
		c_list -> cache_size += payload_size;
	}
	/* Unblock Cache */
	V(&mutex);

	return;
}