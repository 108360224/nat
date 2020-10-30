#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
//中間樞紐獲得A客戶端的外網ip和port傳送給客戶端B，獲得客戶端B的外網ip和port傳送給A
//B通過A打的洞發資料給A，這時候A拒收B的訊息，因為A的nat對映中沒有B的資訊，但是這次通
//信卻在B的網管中添加了對映可以接受A的
//訊息，同理A通過B打的洞發資料給B，這時候由於B可以接受A的訊息，所以資料接收成功且在A
//的對映中加入了B的資訊，從而A與B可以跨伺服器通訊。實現p2p
/* 由於已知的外網伺服器S可能並沒有AB客戶端的對映關係，所以要先建立A與S 還有 B與S之間的對映，這樣才能進行udp穿透。 */

#define ERR_EXIT(m)\
    do{\
        perror(m);\
        exit(1);\
    }while(0)

/* 用來記錄客戶端傳送過來的外網ip+port */
typedef struct {
    struct in_addr ip;
    int port;
}clientInfo;

int main()
{
    /* 一個客戶端資訊結構體陣列，分別存放兩個客戶端的外網ip+port */
    clientInfo info[2];
    /* 作為心跳包需要接收的一個位元組 */
    /* char ch; */
    char str[10] = { 0 };

    /* udp socket描述符 */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
        ERR_EXIT("SOCKET");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    serveraddr.sin_port = htons(8888);
    serveraddr.sin_family = AF_INET;

    int ret = bind(sockfd, (struct sockaddr*) & serveraddr, sizeof(serveraddr));
    if (ret == -1)
        ERR_EXIT("BIND");

    /* 伺服器接收客戶端發來的訊息並轉發 */
    while (1)
    {
        bzero(info, sizeof(clientInfo) * 2);
        /* 接收兩個心跳包並記錄其與此連結的ip+port */
        socklen_t addrlen = sizeof(struct sockaddr_in);
        /* recvfrom(sockfd, &ch, sizeof(ch), 0, (struct sockaddr *)&serveraddr, &addrlen); */
        recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr*) & serveraddr, &addrlen);
        memcpy(&info[0].ip, &serveraddr.sin_addr, sizeof(struct in_addr));
        info[0].port = serveraddr.sin_port;

        printf("A client IP:%s \tPort:%d creat link OK!\n", inet_ntoa(info[0].ip), ntohs(info[0].port));

        /* recvfrom(sockfd, &ch, sizeof(ch), 0, (struct sockaddr *)&serveraddr, &addrlen); */
        recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr*) & serveraddr, &addrlen);
        memcpy(&info[1].ip, &serveraddr.sin_addr, sizeof(struct in_addr));
        info[1].port = serveraddr.sin_port;

        printf("B client IP:%s \tPort:%d creat link OK!\n", inet_ntoa(info[1].ip), ntohs(info[1].port));

        /* 分別向兩個客戶端傳送對方的外網ip+port */
        printf("start informations translation...\n");
        serveraddr.sin_addr = info[0].ip;
        serveraddr.sin_port = info[0].port;
        sendto(sockfd, &info[1], sizeof(clientInfo), 0, (struct sockaddr*) & serveraddr, addrlen);

        serveraddr.sin_addr = info[1].ip;
        serveraddr.sin_port = info[1].port;
        sendto(sockfd, &info[0], sizeof(clientInfo), 0, (struct sockaddr*) & serveraddr, addrlen);
        printf("send informations successful!\n");
    }
    return 0;
}