/* C Port of https://github.com/dhbaird/easywsclient/ by Onat Turkcuoglu */

#ifdef _WIN32
    #if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
        #define _CRT_SECURE_NO_WARNINGS // _CRT_SECURE_NO_WARNINGS for sscanf errors in MSVC2013 Express
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <fcntl.h>
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #pragma comment( lib, "ws2_32" )
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/types.h>
    #include <io.h>
    #ifndef _SSIZE_T_DEFINED
        typedef int ssize_t;
        #define _SSIZE_T_DEFINED
    #endif
    #ifndef _SOCKET_T_DEFINED
        typedef SOCKET socket_t;
        #define _SOCKET_T_DEFINED
    #endif
    #ifndef snprintf
        #define snprintf _snprintf_s
    #endif
    #if _MSC_VER >=1600
        // vs2010 or later
        #include <stdint.h>
    #else
        typedef __int8 int8_t;
        typedef unsigned __int8 uint8_t;
        typedef __int32 int32_t;
        typedef unsigned __int32 uint32_t;
        typedef __int64 int64_t;
        typedef unsigned __int64 uint64_t;
    #endif
    #define socketerrno WSAGetLastError()
    #define SOCKET_EAGAIN_EINPROGRESS WSAEINPROGRESS
    #define SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
#else
    #include <fcntl.h>
    #include <netdb.h>
    #include <netinet/tcp.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <stdint.h>
    #ifndef _SOCKET_T_DEFINED
        typedef int socket_t;
        #define _SOCKET_T_DEFINED
    #endif
    #ifndef INVALID_SOCKET
        #define INVALID_SOCKET (-1)
    #endif
    #ifndef SOCKET_ERROR
        #define SOCKET_ERROR   (-1)
    #endif
    #define closesocket(s) close(s)
    #include <errno.h>
    #define socketerrno errno
    #define SOCKET_EAGAIN_EINPROGRESS EAGAIN
    #define SOCKET_EWOULDBLOCK EWOULDBLOCK
#endif

#include "ws_client.h"

/* Connect and return the socket file descriptor */
socket_t hostname_connect(const char* hostname, int port) {
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *p;
    int ret;
    socket_t sockfd = INVALID_SOCKET;
    char sport[16];
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(sport, 16, "%d", port);
    if ((ret = getaddrinfo(hostname, sport, &hints, &result)) != 0)
    {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
      return 1;
    }
    for(p = result; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == INVALID_SOCKET) { continue; }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) != SOCKET_ERROR) {
            break;
        }
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
    }
    freeaddrinfo(result);
    return sockfd;
}

typedef struct wsconn {
    char* rxbuf;
    char* txbuf;
    char* receivedData;
    size_t rxbuf_size;
    size_t txbuf_size;
    size_t receivedData_size;
    char readyState;
    char pollbuf[POLL_BUFFER_SIZE];
    char useMask;
    socket_t sockfd;
} wsconn;

typedef struct wsheader_type {
    unsigned header_size;
    char fin;
    char mask;
    enum opcode_type {
        CONTINUATION = 0x0,
        TEXT_FRAME = 0x1,
        BINARY_FRAME = 0x2,
        CLOSE = 8,
        PING = 9,
        PONG = 0xa,
    } opcode;
    int N0;
    uint64_t N;
    uint8_t masking_key[4];
} wsheader_type;

struct wsconn* wsconn_init()
{
    struct wsconn* ws = (struct wsconn*)malloc(sizeof(wsconn));
    memset((void*)ws, 0, sizeof(wsconn));
    ws->readyState = OPEN;
    ws->txbuf = (char*)malloc(1024);
    ws->rxbuf = (char*)malloc(1024);
    ws->receivedData = (char*)malloc(1024);
    return ws;
}

void wsconn_stats(struct wsconn* ws)
{
    printf("txbuf_size: %d\n", ws->txbuf_size);
    printf("rxbuf_size: %d\n", ws->rxbuf_size);
    printf("recv_size: %d\n", ws->receivedData_size);
}

void wsconn_shutdown(struct wsconn* ws)
{
    if (ws->rxbuf)
        free(ws->rxbuf);
    if (ws->txbuf)
        free(ws->txbuf);
    if (ws->receivedData)
        free(ws->receivedData);

    free(ws);
}

void poll(wsconn* ws, int timeout) { // timeout in milliseconds
    if (ws->readyState == CLOSED) {
        if (timeout > 0) {
            struct timeval tv = { timeout/1000, (timeout%1000) * 1000 };
            select(0, NULL, NULL, NULL, &tv);
        }
        return;
    }
    if (timeout > 0) {
        fd_set rfds;
        fd_set wfds;
        struct timeval tv = { timeout/1000, (timeout%1000) * 1000 };
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(ws->sockfd, &rfds);
        if (ws->txbuf_size) { FD_SET(ws->sockfd, &wfds); }
        select(ws->sockfd + 1, &rfds, &wfds, NULL, &tv);
    }
    while (1) {
        // FD_ISSET(0, &rfds) will be true
        int N = ws->rxbuf_size;
        ssize_t ret;
        //rxbuf.resize(N + POLL_BUFFER_SIZE);
        //ret = recv(ws->sockfd, ws->rxbuf + N, POLL_BUFFER_SIZE, 0);
        ret = recv(ws->sockfd, ws->pollbuf, POLL_BUFFER_SIZE, 0);
        if (ret < 0 && (socketerrno == SOCKET_EWOULDBLOCK || socketerrno == SOCKET_EAGAIN_EINPROGRESS)) {
            //rxbuf.resize(N);
            break;
        }
        else if (ret <= 0) {
            //rxbuf.resize(N);
            closesocket(ws->sockfd);
            ws->readyState = CLOSED;
            fputs(ret < 0 ? "Connection error!\n" : "Connection closed!\n", stderr);
            break;
        }
        else {
            ws->rxbuf = (char*)realloc(ws->rxbuf, N + ret);
            //printf("Reallocating rxbuf to %d", N + ret);
            memcpy(ws->rxbuf + N, ws->pollbuf, ret);
            ws->rxbuf_size += ret;
            //rxbuf.resize(N + ret);
        }
    }
    while (ws->txbuf_size) {
        int ret = send(ws->sockfd, ws->txbuf, ws->txbuf_size, 0);
        if (ret < 0 && (socketerrno == SOCKET_EWOULDBLOCK || socketerrno == SOCKET_EAGAIN_EINPROGRESS)) {
            break;
        }
        else if (ret <= 0) {
            closesocket(ws->sockfd);
            ws->readyState = CLOSED;
            fputs(ret < 0 ? "Connection error!\n" : "Connection closed!\n", stderr);
            break;
        }
        else {
            memmove(ws->txbuf, ws->txbuf + ret, ws->txbuf_size);
            ws->txbuf_size -= ret;
            if (ws->txbuf_size < 0) {
                ws->txbuf_size = 0;
            }
            //txbuf.erase(txbuf.begin(), txbuf.begin() + ret);
        }
    }
    if (!ws->txbuf_size && ws->readyState == CLOSING) {
        closesocket(ws->sockfd);
        ws->readyState = CLOSED;
    }
}

void sendData(struct wsconn* ws, uint8_t type, void* data, size_t message_size) {
    // TODO:
    // Masking key should (must) be derived from a high quality random
    // number generator, to mitigate attacks on non-WebSocket friendly
    // middleware:
    const uint8_t masking_key[4] = { 0x12, 0x34, 0x56, 0x78 };
    // TODO: consider acquiring a lock on txbuf...
    if (ws->readyState == CLOSING || ws->readyState == CLOSED) { return; }
    uint8_t header[16];
    //std::vector<uint8_t> header;
    //header.assign(2 + (message_size >= 126 ? 2 : 0) + (message_size >= 65536 ? 6 : 0) + (ws->useMask ? 4 : 0), 0);
    char headerSize = 2 + (message_size >= 126 ? 2 : 0) + (message_size >= 65536 ? 6 : 0) + (ws->useMask ? 4 : 0);
    memset(header, 0, headerSize); 
    header[0] = 0x80 | type;
    if (message_size < 126) {
        header[1] = (message_size & 0xff) | (ws->useMask ? 0x80 : 0);
        if (ws->useMask) {
            header[2] = masking_key[0];
            header[3] = masking_key[1];
            header[4] = masking_key[2];
            header[5] = masking_key[3];
        }
    }
    else if (message_size < 65536) {
        header[1] = 126 | (ws->useMask ? 0x80 : 0);
        header[2] = (message_size >> 8) & 0xff;
        header[3] = (message_size >> 0) & 0xff;
        if (ws->useMask) {
            header[4] = masking_key[0];
            header[5] = masking_key[1];
            header[6] = masking_key[2];
            header[7] = masking_key[3];
        }
    }
    else { // TODO: run coverage testing here
        header[1] = 127 | (ws->useMask ? 0x80 : 0);
        header[2] = (message_size >> 56) & 0xff;
        header[3] = (message_size >> 48) & 0xff;
        header[4] = (message_size >> 40) & 0xff;
        header[5] = (message_size >> 32) & 0xff;
        header[6] = (message_size >> 24) & 0xff;
        header[7] = (message_size >> 16) & 0xff;
        header[8] = (message_size >>  8) & 0xff;
        header[9] = (message_size >>  0) & 0xff;
        if (ws->useMask) {
            header[10] = masking_key[0];
            header[11] = masking_key[1];
            header[12] = masking_key[2];
            header[13] = masking_key[3];
        }
    }
    memcpy(ws->txbuf + ws->txbuf_size, header, headerSize);
    ws->txbuf_size += headerSize;
    memcpy(ws->txbuf + ws->txbuf_size, data, message_size);
    ws->txbuf_size += message_size;
    if (ws->useMask) {
        //for (size_t i = 0; i != message_size; ++i) { *(txbuf.end() - message_size + i) ^= masking_key[i&0x3]; }
        size_t i;
        for (i = 0; i != message_size; ++i) { *(&ws->txbuf[ws->txbuf_size]- message_size + i) ^= masking_key[i&0x3]; }
    }
}

void closeconn(struct wsconn *ws) {
    if(ws->readyState == CLOSING || ws->readyState == CLOSED) { return; }
    ws->readyState = CLOSING;
    uint8_t closeFrame[6] = {0x88, 0x80, 0x00, 0x00, 0x00, 0x00}; // last 4 bytes are a masking key
    memcpy(ws->txbuf + ws->txbuf_size, closeFrame, 6);
    ws->txbuf_size += 6;
}

void sendPing(struct wsconn* ws) {
    char empty = 0;
    sendData(ws, wsheader_type::PING, &empty, 0);
}

void sendText(struct wsconn* ws, const char* data) {
    sendData(ws, wsheader_type::TEXT_FRAME, (char*)data, strlen(data));
}

struct wsconn* open_wsconn(const char* url, char useMask, const char* origin) {
    char host[128];
    int port;
    char path[128];
    if (url && strlen(url) >= 128) {
      fprintf(stderr, "ERROR: url size limit exceeded: %s\n", url);
      return NULL;
    }
    if (origin && strlen(origin) >= 200) {
      fprintf(stderr, "ERROR: origin size limit exceeded: %s\n", origin);
      return NULL;
    }
    if (sscanf(url, "ws://%[^:/]:%d/%s", host, &port, path) == 3) {
    }
    else if (sscanf(url, "ws://%[^:/]/%s", host, path) == 2) {
        port = 80;
    }
    else if (sscanf(url, "ws://%[^:/]:%d", host, &port) == 2) {
        path[0] = '\0';
    }
    else if (sscanf(url, "ws://%[^:/]", host) == 1) {
        port = 80;
        path[0] = '\0';
    }
    else {
        fprintf(stderr, "ERROR: Could not parse WebSocket url: %s\n", url);
        return NULL;
    }
    fprintf(stderr, "easywsclient: connecting: host=%s port=%d path=/%s\n", host, port, path);
    socket_t sockfd = hostname_connect(host, port);
    if (sockfd == INVALID_SOCKET) {
        fprintf(stderr, "Unable to connect to %s:%d\n", host, port);
        return NULL;
    }
    {
        // XXX: this should be done non-blocking,
        char line[256];
        int status;
        int i;
        snprintf(line, 256, "GET /%s HTTP/1.1\r\n", path); send(sockfd, line, strlen(line), 0);
        if (port == 80) {
            snprintf(line, 256, "Host: %s\r\n", host); send(sockfd, line, strlen(line), 0);
        }
        else {
            snprintf(line, 256, "Host: %s:%d\r\n", host, port); send(sockfd, line, strlen(line), 0);
        }
        snprintf(line, 256, "Upgrade: websocket\r\n"); send(sockfd, line, strlen(line), 0);
        snprintf(line, 256, "Connection: Upgrade\r\n"); send(sockfd, line, strlen(line), 0);
        if (!origin) {
            snprintf(line, 256, "Origin: %s\r\n", origin); send(sockfd, line, strlen(line), 0);
        }
        snprintf(line, 256, "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"); send(sockfd, line, strlen(line), 0);
        snprintf(line, 256, "Sec-WebSocket-Version: 13\r\n"); send(sockfd, line, strlen(line), 0);
        snprintf(line, 256, "\r\n"); send(sockfd, line, strlen(line), 0);
        for (i = 0; i < 2 || (i < 255 && line[i-2] != '\r' && line[i-1] != '\n'); ++i) { if (recv(sockfd, line+i, 1, 0) == 0) { return NULL; } }
        line[i] = 0;
        if (i == 255) { fprintf(stderr, "ERROR: Got invalid status line connecting to: %s\n", url); return NULL; }
        if (sscanf(line, "HTTP/1.1 %d", &status) != 1 || status != 101) { fprintf(stderr, "ERROR: Got bad status connecting to %s: %s", url, line); return NULL; }
        // TODO: verify response headers,
        while (1) {
            for (i = 0; i < 2 || (i < 255 && line[i-2] != '\r' && line[i-1] != '\n'); ++i) { if (recv(sockfd, line+i, 1, 0) == 0) { return NULL; } }
            if (line[0] == '\r' && line[1] == '\n') { break; }
        }
    }
    int flag = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(flag)); // Disable Nagle's algorithm
#ifdef _WIN32
    u_long on = 1;
    ioctlsocket(sockfd, FIONBIO, &on);
#else
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif
    fprintf(stderr, "Connected to: %s\n", url);
    struct wsconn* ws = wsconn_init();
    ws->sockfd = sockfd;
    ws->useMask = useMask;
    return ws;
}

uint8_t getReadyState(struct wsconn *ws)
{
    return ws->readyState;
}

void dispatch(struct wsconn *wss, dataReceivedFn callback) {
    // TODO: consider acquiring a lock on rxbuf...

    while (1) {
        wsheader_type ws;
        if (wss->rxbuf_size < 2) { return; /* Need at least 2 */ }
        const uint8_t * data = (uint8_t *) wss->rxbuf; // peek, but don't consume
        size_t data_size;
        ws.fin = (data[0] & 0x80) == 0x80;
        ws.opcode = (wsheader_type::opcode_type)(data[0] & 0x0f);
        ws.mask = (data[1] & 0x80) == 0x80;
        ws.N0 = (data[1] & 0x7f);
        ws.header_size = 2 + (ws.N0 == 126? 2 : 0) + (ws.N0 == 127? 8 : 0) + (ws.mask? 4 : 0);
        if (wss->rxbuf_size < ws.header_size) { return; /* Need: ws.header_size - wss->rxbuf_size */ }
        int i;
        if (ws.N0 < 126) {
            ws.N = ws.N0;
            i = 2;
        }
        else if (ws.N0 == 126) {
            ws.N = 0;
            ws.N |= ((uint64_t) data[2]) << 8;
            ws.N |= ((uint64_t) data[3]) << 0;
            i = 4;
        }
        else if (ws.N0 == 127) {
            ws.N = 0;
            ws.N |= ((uint64_t) data[2]) << 56;
            ws.N |= ((uint64_t) data[3]) << 48;
            ws.N |= ((uint64_t) data[4]) << 40;
            ws.N |= ((uint64_t) data[5]) << 32;
            ws.N |= ((uint64_t) data[6]) << 24;
            ws.N |= ((uint64_t) data[7]) << 16;
            ws.N |= ((uint64_t) data[8]) << 8;
            ws.N |= ((uint64_t) data[9]) << 0;
            i = 10;
        }
        if (ws.mask) {
            ws.masking_key[0] = ((uint8_t) data[i+0]) << 0;
            ws.masking_key[1] = ((uint8_t) data[i+1]) << 0;
            ws.masking_key[2] = ((uint8_t) data[i+2]) << 0;
            ws.masking_key[3] = ((uint8_t) data[i+3]) << 0;
        }
        else {
            ws.masking_key[0] = 0;
            ws.masking_key[1] = 0;
            ws.masking_key[2] = 0;
            ws.masking_key[3] = 0;
        }
        if (wss->rxbuf_size < ws.header_size+ws.N) { return; /* Need: ws.header_size+ws.N - wss->rxbuf_size */ }

        // We got a whole message, now do something with it:
        if (
               ws.opcode == wsheader_type::TEXT_FRAME 
            || ws.opcode == wsheader_type::BINARY_FRAME
            || ws.opcode == wsheader_type::CONTINUATION
        ) {
            size_t i;
            if (ws.mask) { for (i = 0; i != ws.N; ++i) { wss->rxbuf[i+ws.header_size] ^= ws.masking_key[i&0x3]; } }
            //wss->receivedData + wss->receivedData_size
            memcpy(wss->receivedData + wss->receivedData_size, wss->rxbuf + ws.header_size, ws.N); 
            wss->receivedData_size += ws.N;
            //receivedData.insert(receivedData.end(), wss->rxbuf+ws.header_size, wss->rxbuf+ws.header_size+(size_t)ws.N);// just feed
            if (ws.fin) {
                callback(wss->receivedData, wss->receivedData_size);
                //callable((const std::vector<uint8_t>) receivedData);
                wss->receivedData_size = 0;
                //receivedData.erase(receivedData.begin(), receivedData.end());
                //std::vector<uint8_t> ().swap(receivedData);// free memory
            }
        }
        else if (ws.opcode == wsheader_type::PING) {
            size_t i;
            if (ws.mask) { for (i = 0; i != ws.N; ++i) { wss->rxbuf[i+ws.header_size] ^= ws.masking_key[i&0x3]; } }
            //std::string data(rxbuf.begin()+ws.header_size, rxbuf.begin()+ws.header_size+(size_t)ws.N);
            char pong_data[256];
            size_t pong_size = ws.N;
            memcpy(pong_data, wss->rxbuf + ws.header_size, pong_size);
            sendData(wss, wsheader_type::PONG, pong_data, pong_size);
        }
        else if (ws.opcode == wsheader_type::PONG) { }
        else if (ws.opcode == wsheader_type::CLOSE) { closeconn(wss); }
        else { fprintf(stderr, "ERROR: Got unexpected WebSocket message.\n"); closeconn(wss); }

        memmove(wss->rxbuf, wss->rxbuf + ws.header_size + ws.N, wss->rxbuf_size - ws.header_size - ws.N);
        wss->rxbuf_size -= ws.header_size + ws.N;
        if (wss->rxbuf_size < 0)
            wss->rxbuf_size = 0;
        //rxbuf.erase(rxbuf.begin(), rxbuf.begin() + ws.header_size+(size_t)ws.N);
    }
}
