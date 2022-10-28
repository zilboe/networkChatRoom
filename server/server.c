#include "../head.h"
msg_t phead;
int sockfd;

// void delNodeFromList(msg_t *list, cli_t *pNode)
// {
//     if (pNode == NULL || list == NULL)
//     {
//         perror("list is null\n");
//         return;
//     }
//     if (list->size == 0 || list->cli.next == NULL)
//     {
//         return;
//     }
//     cli_t *p = list->cli.next;
//     if (p->next == NULL)
//     {
//         if (p->fd != pNode->fd)
//         {
//             return;
//         }
//         else
//         {
//             list->cli.next = NULL;
//             free(p);
//             p = NULL;
//             return;
//         }
//     }
//     else if (p->next->next == NULL)
//     {
//         if (p->fd == pNode->fd)
//         {
//             list->cli.next = p->next;
//             p->next = NULL;
//             free(p);
//             p = NULL;
//         }
//         else if (p->next->fd == pNode->fd)
//         {
//             cli_t *pDel = p->next;
//             p->next = NULL;
//             free(pDel);
//             pDel = NULL;
//         }
//         else
//         {
//         }
//         return;
//     }
//     else
//     {
//         while (p->next != NULL)
//         {
//             if (list->cli.next == p)
//             {
//                 if (p->fd == pNode->fd)
//                 {
//                     cli_t *pDel = p;
//                     p = p->next;
//                     list->cli.next = p;
//                     free(pDel);
//                     pDel = NULL;
//                     break;
//                 }
//             }
//             else if (p->next->fd == pNode->fd)
//             {
//                 if (p->next)
//                 cli_t *pDel = p->next;
//                 p->next = pDel->next;
//                 free(pDel);
//                 pDel = NULL;
//             }
//             p = p->next;
//         }
//     }
// }

void *pthread_common(void *arg)
{
    cli_t *pNode = (cli_t *)arg;
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
        if (ret > 0)
        {
            if (!strcmp("quit", msgs.message))
            {
                phead.size--;
                msgs.type = 0;
                sprintf(msgs.message, "客户端(%s:%d)退出聊天室", inet_ntoa(pNode->client.sin_addr), ntohs(pNode->client.sin_port));
                cli_t *p = &phead.cli;
                cli_t *pDel = NULL;
                while (p->next != NULL)
                {
                    p = p->next;
                    if (p->fd == pNode->fd)
                    {
                        pDel = p;
                        continue;
                    }
                    send(p->fd, &msgs, sizeof(msgs), 0);
                }
                p = &phead.cli;
                while (p->next!=pDel) // p节点跑到删除节点前一节点
                    p = p->next;
                if (pDel->next!=NULL)
                {
                    cli_t *pNext = pDel->next;
                    p->next = pNext;
                    free(pDel);
                    pDel = NULL;
                }
                else
                {
                    p->next = NULL;
                    free(pDel);
                    pDel = NULL;
                }
                // delNodeFromList(&phead, pNode);
                bzero(&msgs, 0);
                break;
            }
            else
            {
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
        cli_t *p = &phead.cli;
        while (p->next != NULL)
        {
            p = p->next;
            printf("id = %d\n", p->fd);
        }
        pthread_t commonPthread;
        pthread_create(&commonPthread, NULL, pthread_common, (void *)pNode);
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