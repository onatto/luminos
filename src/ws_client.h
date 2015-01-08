#pragma once

#include <stdlib.h>
#include <stdint.h>

enum { CLOSING, CLOSED, CONNECTING, OPEN };
#define POLL_BUFFER_SIZE 1500

typedef struct wsconn wsconn;
struct wsconn* wsconn_init();
void wsconn_shutdown(struct wsconn* ws);
void poll(wsconn* ws, int timeout);
void sendData(struct wsconn* ws, uint8_t type, void* data, size_t message_size);
void closeconn(struct wsconn *ws);
void sendPing(struct wsconn* ws);
void sendText(struct wsconn* ws, const char* data);
uint8_t getReadyState(struct wsconn *ws);
struct wsconn* open_wsconn(const char* url, char useMask, const char* origin);

typedef void (*dataReceivedFn)(void* data, size_t size);
void dispatch(struct wsconn *wss, dataReceivedFn callback);
