#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE (1024*1024)
#define MAX_OBJECT_SIZE (100*1024)

static const char *header_user_agent = "Mozilla/5.0"
                                    " (X11; Linux x86_64; rv:45.0)"
                                    " Gecko/20180601 Firefox/45.0";

int main(int argc, char** argv) {
    printf("%s", header_user_agent);
    return 0;
}

