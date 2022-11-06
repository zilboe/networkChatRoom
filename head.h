#ifndef __HEAD_H__
#define __HEAD_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip.h>
#include <pthread.h>
#define IP "127.0.0.1"
#define PORT 5212

#ifndef BOOL
#define BOOL int
#define OK 1
#define ERROR 0
#endif
typedef struct client
{
    struct sockaddr_in client;
    int fd;
    struct client *next;
}cli_t;

typedef struct msg
{
    pthread_mutex_t lock;
    char message[512];
    int type;//用来检测是否第一次登陆数据
    cli_t cli;
    int size;
}msg_t;

#define SQLName "sqlite.db"

#ifndef true
#define true 1
#define false 0
#endif

#endif