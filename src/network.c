#ifndef MSVC
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
#include <errno.h>

#include "lua.h"
#include "lauxlib.h"

#include <strings.h>

#define BUFFER_SIZE 1024*128

static int sockfd = 0; // Socket file descriptor
static lua_State* L; // Current Lua state
static char buffer[BUFFER_SIZE]; // Buffer the data received will be read into

void error(char *msg) {
    printf("%s", msg);
    exit(0);
}

void receiveData(void* data, size_t size)
{
    lua_getglobal(L, "portReceiveMessageError");
    lua_getglobal(L, "portReceiveMessage");
    if (!lua_isfunction(L, -1))
        return;
    lua_pushlstring(L, (const char*)data, size);
    lua_pcall(L, 1, 0, -2);
    memset(buffer, 0, BUFFER_SIZE);
}

void networkInit(struct lua_State* l, const char* url, unsigned port)
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
        printf("ERROR, no such host as %s:%d\n", url, port);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], 
            (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(port);

    /* connect: create a connection with the server */
    if (connect(sockfd, (__CONST_SOCKADDR_ARG)&serveraddr, sizeof(serveraddr)) < 0) {
        error("ERROR connecting");
    }

    /* Set socket file descriptor for non-blocking reads */
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

}

static char wbuffer[4096];
static size_t wbuffer_len = 0;
int nw_send(const char* msg)
{
    if (sockfd == 0) {
        return -1;
    }
    int len = strlen(msg);
    memcpy(wbuffer + wbuffer_len, msg, len);
    wbuffer[wbuffer_len + len] = 4;
    wbuffer_len += len+1;
    printf("Sending msg: %s\n", msg);
    return 0;
}

// Flush writes at the end of the frame
void networkFlushWrites()
{
    if (wbuffer_len > 0) {
        int n = send(sockfd, wbuffer, wbuffer_len, 0);
        if (n < 0) {
            printf("Error writing to socket...\n");
        }
        else
        {
            wbuffer_len = 0;
        }
    }
}

void networkUpdate()
{
    if (sockfd == 0) {
        return;
    }

    int n = read(sockfd, buffer, BUFFER_SIZE);
    if (n > 0) {
        receiveData(buffer, n);
    }
}

void networkShutdown()
{
    close(sockfd);
}

void networkSetLua(struct lua_State* l)
{
    L = l;
}
#endif
