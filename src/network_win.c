#ifdef MSVC
#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "lua.h"
#include "lauxlib.h"

#define BUFFER_SIZE 1024*128

static SOCKET ConnectSocket = INVALID_SOCKET; // Socket file descriptor
static lua_State* L; // Current Lua state
static char buffer[BUFFER_SIZE]; // Buffer the data received will be read into
static char networkConnected = 0;

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
    networkConnected = 0;
    L = l;
    WSADATA wsaData;
    struct addrinfo *result = NULL,
		    *ptr = NULL,
		    hints;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
	printf("WSAStartup failed with error: %d\n", iResult);
	return 1;
    }

    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
	printf("socket failed with error: %ld\n", WSAGetLastError());
	WSACleanup();
	return 1;
    }

    struct hostent *host;
    if ((host = gethostbyname(url)) == NULL)
    {
	printf("Failed to resolve hostname.\r\n");
	WSACleanup();
	return;
    }

    SOCKADDR_IN SockAddr;
    SockAddr.sin_port = htons(port);
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

    // Attempt to connect to server
    if (connect(ConnectSocket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
    {
	printf("Failed to establish connection with server\r\n");
	WSACleanup();
	return;
    }

    // If iMode!=0, non-blocking mode is enabled.
    u_long iMode = 1;
    ioctlsocket(ConnectSocket, FIONBIO, &iMode);
    networkConnected = 1;
}

static char wbuffer[4096];
static size_t wbuffer_len = 0;
void nw_send(const char* msg)
{
    if (!networkConnected)
	return;

    int len = strlen(msg);
    memcpy(wbuffer + wbuffer_len, msg, len);
    wbuffer[wbuffer_len + len] = 4;
    wbuffer_len += len+1;
    printf("Sending msg: %s\n", msg);
}

// Flush writes at the end of the frame
void networkFlushWrites()
{
    int n = 0;
    if (networkConnected && wbuffer_len > 0) {
	n = send(ConnectSocket, wbuffer, wbuffer_len,0);
    }
    if (n < 0) {
        printf("Error writing to socket...\n");
    }
    else
    {
        wbuffer_len = 0;
    }
}

void networkUpdate()
{
    if (!networkConnected)
	return;
    int n = recv(ConnectSocket, buffer, BUFFER_SIZE, 0);
    if (n > 0) {
        receiveData(buffer, n);
    }
}

void networkShutdown()
{
    int iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
	printf("Shutdown failed with error: %d\n", WSAGetLastError());
	closesocket(ConnectSocket);
	WSACleanup();
    }
}

void networkSetLua(struct lua_State* l)
{
    L = l;
}
#endif
