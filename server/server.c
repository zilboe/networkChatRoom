#include "../head.h"
msg_t phead;
int sockfd;

void delNodeFromList(msg_t *phead, int fd)
{
    if (phead == NULL)
        return;
    cli_t *p = &phead->cli;
    cli_t *pDel = NULL;
    while (p->next != NULL)
    {
        if (p->next->fd == fd)
        {
            pDel = p->next;
            break;
        }
        p = p->next;
    }
    p->next = pDel->next;
    pDel->next = NULL;
    free(pDel);
    pDel = NULL;
}

void *pthread_common(void *arg)
{
    cli_t *pNode = (cli_t *)arg;
    printf("客户端(%s:%d)加入聊天室\n", inet_ntoa(pNode->client.sin_addr), ntohs(pNode->client.sin_port));
    if (phead.size > 1)
    {
        cli_t *p = &phead.cli;
        msg_t msg;
        bzero(&msg, 0);
        msg.cli = *pNode;
        msg.type = 0;
        sprintf(msg.message, "客户端(%s:%d)加入聊天室", inet_ntoa(pNode->client.sin_addr), ntohs(pNode->client.sin_port));
        while (p->next != NULL)
        {
            p = p->next;
            if (p->fd == pNode->fd)
                continue;
            send(p->fd, &msg, sizeof(msg), 0);
        }
    }
    while (1)
    {
        msg_t msgs;
        bzero(&msgs, 0);
        int ret = recv(pNode->fd, &msgs, sizeof(msgs), 0);
        pthread_mutex_lock(&pNode->lock);
        if (ret > 0)
        {
            if (!strcmp("quit", msgs.message))
            {
                pthread_mutex_unlock(&pNode->lock);
                phead.size--;
                msgs.type = 0;
                printf("客户端(%s:%d)退出聊天室", inet_ntoa(pNode->client.sin_addr), ntohs(pNode->client.sin_port));
                sprintf(msgs.message, "客户端(%s:%d)退出聊天室", inet_ntoa(pNode->client.sin_addr), ntohs(pNode->client.sin_port));
                cli_t *p = &phead.cli;
                while (p->next != NULL)
                {
                    p = p->next;
                    if (p->fd == pNode->fd)
                    {
                        continue;
                    }
                    send(p->fd, &msgs, sizeof(msgs), 0);
                }
                delNodeFromList(&phead, pNode->fd);
                close(pNode->fd);
                pthread_cancel(pthread_self());
                bzero(&msgs, 0);
                break;
            }
            else
            {
                pthread_mutex_unlock(&pNode->lock);
                msgs.cli = *pNode;
                cli_t *p = &phead.cli;
                msgs.type = 1;
                while (p->next != NULL)
                {
                    p = p->next;
                    if (p->fd == pNode->fd)
                        continue;
                    send(p->fd, &msgs, sizeof(msgs), 0);
                }
            }
        }
        else
        {
            pthread_mutex_unlock(&pNode->lock);
            phead.size--;
            msgs.type = 0;
            printf("客户端(%s:%d)意外退出聊天室", inet_ntoa(pNode->client.sin_addr), ntohs(pNode->client.sin_port));
            sprintf(msgs.message, "客户端(%s:%d)意外退出聊天室", inet_ntoa(pNode->client.sin_addr), ntohs(pNode->client.sin_port));
            cli_t *p = &phead.cli;
            while (p->next != NULL)
            {
                p = p->next;
                if (p->fd == pNode->fd)
                {
                    continue;
                }
                send(p->fd, &msgs, sizeof(msgs), 0);
            }
            delNodeFromList(&phead, pNode->fd);
            pthread_cancel(pthread_self());
            close(pNode->fd);
            bzero(&msgs, 0);
            break;
        }
    }
}

void *pthread_accept(void *arg)
{
    struct sockaddr_in clientSock;
    socklen_t clientLen = sizeof(struct sockaddr_in);
    printf("===========聊天室已创建===========\n");
    while (1)
    {
        int acceptFd = accept(sockfd, (struct sockaddr *)&clientSock, &clientLen);
        cli_t *pNode = (cli_t *)malloc(sizeof(cli_t));
        pNode->client = clientSock;
        pNode->fd = acceptFd;
        pNode->next = NULL;
        if (phead.size > 0)
        {
            cli_t *p = phead.cli.next;
            phead.cli.next = pNode;
            pNode->next = p;
        }
        else if (phead.size == 0)
        {
            phead.cli.next = pNode;
        }
        phead.size++;
        pthread_t commonPthread;
        pthread_create(&commonPthread, NULL, pthread_common, (void *)pNode);
        pthread_detach(commonPthread);
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
    bind(sockfd, (struct sockaddr *)&serverSock, serverLen);

    listen(sockfd, 0);

    pthread_t acceptPthread;
    pthread_create(&acceptPthread, NULL, pthread_accept, NULL);
    while (1)
        ;
    return 0;
}