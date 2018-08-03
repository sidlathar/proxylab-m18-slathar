#include "csapp.h"
#include <stdbool.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE (1024*1024)
#define MAX_OBJECT_SIZE (100*1024)
#define HOSTLEN 256
#define SERVLEN 8

// static const char *header_user_agent = "Mozilla/5.0"
//                                     " (X11; Linux x86_64; rv:45.0)"
//                                     " Gecko/20180601 Firefox/45.0";

/* Information about a connected client.  - Taken from tiny.c */ 
typedef struct {
    struct sockaddr_in addr;    // Socket address
    socklen_t addrlen;          // Socket address length
    int connfd;                 // Client connection file descriptor
    char host[HOSTLEN];         // Client host
    char serv[SERVLEN];         // Client service (port)
} client_info;

/* URI parsing results. - Taken from tiny.c */
typedef enum {
    PARSE_ERROR,
    PARSE_STATIC,
    PARSE_DYNAMIC
} parse_result;


/* Function prototypes */
void serve(client_info *client);
parse_result parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
bool read_requesthdrs(rio_t *rp);
void get_filetype(char *filename, char *filetype);
void sigpipe_handler(int sig);


/*
######################################
#									 #
#     / Begin Implementation /       #
#									 #
######################################
*/

void sigpipe_handler(int signal) 
{
	(void)signal;
    return;
}

int main(int argc, char** argv) 
{
   	int listenfd;
   	printf("hererer \n");
  
    /* Need a port number */
    if(argc != 2)
    {
    	fprintf(stderr, "Require's port number\n");
    	return 1;
    }

    printf("hererer \n");

    Signal(SIGPIPE, sigpipe_handler);

    listenfd = Open_listenfd(argv[1]);

    /* Taken from tiny.c */
	while (1) {
        /* Allocate space on the stack for client info */
        client_info client_data;
        client_info *client = &client_data;

        /* Initialize the length of the address */
        client->addrlen = sizeof(client->addr);

        /* Accept() will block until a client connects to the port */
        client->connfd = Accept(listenfd,
                (SA *) &client->addr, &client->addrlen);

        /* Connection is established; serve client */
        serve(client);
        Close(client->connfd);
    }
    return 0;
}

/*
 * serve - handle one HTTP request/response transaction
 * -Taken and modified from tiny.c */
void serve(client_info *client) {
    // Get some extra info about the client (hostname/port)
    // This is optional, but it's nice to know who's connected
    Getnameinfo((SA *) &client->addr, client->addrlen,
            client->host, sizeof(client->host),
            client->serv, sizeof(client->serv),
            0);
    printf("Accepted connection from %s:%s\n", client->host, client->serv);

    rio_t rio;
    rio_readinitb(&rio, client->connfd);

    /* Read request line */
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        return;
    }

    printf("%s", buf);

    /* Parse the request line and check if it's well-formed */
    char method[MAXLINE];
    char uri[MAXLINE];
    char version;

    /* sscanf must parse exactly 3 things for request line to be well-formed */
    /* version must be either HTTP/1.0 or HTTP/1.1 */
    if (sscanf(buf, "%s %s HTTP/1.%c", method, uri, &version) != 3
            || (version != '0' && version != '1')) 
    {
        fprintf(stderr, "400 Bad Request. PROXY received a malformed request");
        return;
    }

    /* Check that the method is GET */
    if (strncmp(method, "GET", sizeof("GET"))) 
    {
        fprintf(stderr, "501 Not Implemented. Tiny does not implement this method");
        return;
    }

    /* Check if reading request headers caused an error */
    if (read_requesthdrs(&rio)) 
    {
        return;
    }

    /* Parse URI from GET request */
    char filename[MAXLINE], cgiargs[MAXLINE];
    parse_result result = parse_uri(uri, filename, cgiargs);
    if (result == PARSE_ERROR) {
        fprintf(stderr, "400 Bad Request Tiny could not parse the request URI");
        return;
    }

    /* Attempt to stat the file */
    struct stat sbuf;
    if (stat(filename, &sbuf) < 0) {
        fprintf(stderr, "404 Not found Tiny couldn't find this file");
        return;
    }

    serve_static(client->connfd, filename, sbuf.st_size);

    return;
}

/*
 * parse_uri - parse URI into filename and CGI args
 *
 * uri - The buffer containing URI. Must contain a NUL-terminated string.
 * filename - The buffer into which the filename will be placed.
 * cgiargs - The buffer into which the CGI args will be placed.
 * NOTE: All buffers must hold MAXLINE bytes, and will contain NUL-terminated
 * strings after parsing.
 *
 * Returns the appropriate parse result for the type of request.
 */
parse_result parse_uri(char *uri, char *filename, char *cgiargs) {
    /* Check if the URI contains "cgi-bin" */
    if (strstr(uri, "cgi-bin")) { /* Dynamic content */
        char *args = strchr(uri, '?');  /* Find the CGI args */
        if (!args) {
            *cgiargs = '\0';    /* No CGI args */
        } else {
            /* Format the CGI args */
            if (snprintf(cgiargs, MAXLINE, "%s", args + 1) >= MAXLINE) {
                return PARSE_ERROR; // Overflow!
            }

            *args = '\0';   /* Remove the args from the URI string */
        }

        /* Format the filename */
        if (snprintf(filename, MAXLINE, ".%s", uri) >= MAXLINE) {
            return PARSE_ERROR; // Overflow!
        }

        return PARSE_DYNAMIC;
    }

    /* Static content */
    /* No CGI args */
    *cgiargs = '\0';

    /* Check if the client is requesting a directory */
    bool is_dir = uri[strnlen(uri, MAXLINE) - 1] == '/';

    /* Format the filename; if requesting a directory, use the home file */
    if (snprintf(filename, MAXLINE, ".%s%s",
                uri, is_dir ? "home.html" : "") >= MAXLINE) {
        return PARSE_ERROR; // Overflow!
    }

    return PARSE_STATIC;
}

/*
 * serve_static - copy a file back to the client
 */
void serve_static(int fd, char *filename, int filesize) {
    int srcfd;
    char *srcp;
    char filetype[MAXLINE];
    char buf[MAXBUF];
    size_t buflen;

    get_filetype(filename, filetype);

    /* Send response headers to client */
    buflen = snprintf(buf, MAXBUF,
            "HTTP/1.0 200 OK\r\n" \
            "Server: Tiny Web Server\r\n" \
            "Connection: close\r\n" \
            "Content-Length: %d\r\n" \
            "Content-Type: %s\r\n\r\n", \
            filesize, filetype);
    if (buflen >= MAXBUF) {
        return; // Overflow!
    }

    printf("Response headers:\n%s", buf);

    if (rio_writen(fd, buf, buflen) < 0) {
        fprintf(stderr, "Error writing static response headers to client\n");
    }


    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    if (rio_writen(fd, srcp, filesize) < 0) {
        fprintf(stderr, "Error writing static file \"%s\" to client\n",
                filename);
    }
}

/*
 * read_requesthdrs - read HTTP request headers
 * Returns true if an error occurred, or false otherwise.
 */
bool read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];

    do {
        if (rio_readlineb(rp, buf, MAXLINE) <= 0) {
            return true;
        }

        printf("%s", buf);
    } while(strncmp(buf, "\r\n", sizeof("\r\n")));

    return false;
}

/*
 * get_filetype - derive file type from file name
 *
 * filename - The file name. Must be a NUL-terminated string.
 * filetype - The buffer in which the file type will be storaged. Must be at
 * least MAXLINE bytes. Will be a NUL-terminated string.
 */
void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html")) {
        strcpy(filetype, "text/html");
    } else if (strstr(filename, ".gif")) {
        strcpy(filetype, "image/gif");
    } else if (strstr(filename, ".png")) {
        strcpy(filetype, "image/png");
    } else if (strstr(filename, ".jpg")) {
        strcpy(filetype, "image/jpeg");
    } else {
        strcpy(filetype, "text/plain");
    }
}


