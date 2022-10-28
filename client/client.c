#include "../head.h"
int sockfd;
int quit = 0;
void *pthread_recv(void *arg)
{
    msg_t msg;
    bzero(&msg, 0);
    while (1)
    {
        int ret = recv(sockfd, &msg, sizeof(msg), 0);
        if (msg.type == 1)
        {
            printf("客户端(%s:%d):%s\n", inet_ntoa(msg.cli.client.sin_addr), ntohs(msg.cli.client.sin_port), msg.message);
        }
        else
        {
            printf("%s\n", msg.message);
        }
    }
}

void *pthread_send(void *arg)
{
    msg_t msg;
    bzero(&msg, 0);
    while (1)
    {
        scanf("%s", msg.message);
        if (getchar() == '\n' && msg.message[0] != '\n')
        {
            send(sockfd, &msg, sizeof(msg), 0);
            if (!strcmp(msg.message, "quit"))
            {
                quit = true;
                break;
            }
            memset(msg.message, 0, sizeof(msg.message));
        }
    }
}

int main()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverSock;
    socklen_t serverLen = sizeof(struct sockaddr_in);
    serverSock.sin_addr.s_addr = htonl(INADDR_ANY);
    serverSock.sin_port = htons(PORT);
    serverSock.sin_family = AF_INET;

    connect(sockfd, (struct sockaddr *)&serverSock, serverLen);
    msg_t msg;
    bzero(&msg, 0);
    pthread_t pthreadSend;
    pthread_create(&pthreadSend, NULL, pthread_send, NULL);
    pthread_t pthreadRecv;
    pthread_create(&pthreadRecv, NULL, pthread_recv, NULL);
    while (!quit);
    close(sockfd);
    return 0;
}