#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>

#include "lua.h"
#include "lauxlib.h"

#define BUFFER_SIZE 4096*32

static int sockfd = 0; // Socket file descriptor
static lua_State* L; // Current Lua state
static char buffer[BUFFER_SIZE]; // Buffer the data received will be read into

void error(char *msg) {
    printf("%s", msg);
    exit(0);
}

void receive_data(void* data, size_t size)
{
    lua_getglobal(L, "portReceiveMessageError");
    lua_getglobal(L, "portReceiveMessage");
    if (!lua_isfunction(L, -1))
        return;
    lua_pushlstring(L, (const char*)data, size);
    lua_pcall(L, 1, 0, -2);
    memset(buffer, 0, BUFFER_SIZE);
}

void network_init(struct lua_State* l, const char* url, unsigned port)
{
    struct sockaddr_in serveraddr;
    struct hostent *server;

    L = l;

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(url);
    if (server == NULL) {
        printf("ERROR, no such host as %s:%d\n", url);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
            (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(port);

    /* connect: create a connection with the server */
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) {
        error("ERROR connecting");
    }

    /* Set socket file descriptor for non-blocking reads */
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

}

void nw_send(const char* msg)
{
    int n = write(sockfd, msg, strlen(msg));
    if (n < 0) {
        error("ERROR writing to socket");
    }
}

void network_update()
{
    int n = read(sockfd, buffer, BUFFER_SIZE);
    if (n > 0) {
        receive_data(buffer, n);
    }
}

void network_close()
{
    close(sockfd);
}

void network_setlua(struct lua_State* l)
{
    L = l;
}

