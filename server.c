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
//�����ϯ���oA�Ȥ�ݪ��~��ip�Mport�ǰe���Ȥ��B�A��o�Ȥ��B���~��ip�Mport�ǰe��A
//B�q�LA�����}�o��Ƶ�A�A�o�ɭ�A�ڦ�B���T���A�]��A��nat��M���S��B����T�A���O�o���q
//�H�o�bB�����ޤ��K�[�F��M�i�H����A��
//�T���A�P�zA�q�LB�����}�o��Ƶ�B�A�o�ɭԥѩ�B�i�H����A���T���A�ҥH��Ʊ������\�B�bA
//����M���[�J�FB����T�A�q��A�PB�i�H����A���q�T�C��{p2p
/* �ѩ�w�����~�����A��S�i��èS��AB�Ȥ�ݪ���M���Y�A�ҥH�n���إ�A�PS �٦� B�PS��������M�A�o�ˤ~��i��udp��z�C */

#define ERR_EXIT(m)\
    do{\
        perror(m);\
        exit(1);\
    }while(0)

/* �ΨӰO���Ȥ�ݶǰe�L�Ӫ��~��ip+port */
typedef struct {
    struct in_addr ip;
    int port;
}clientInfo;

int main()
{
    /* �@�ӫȤ�ݸ�T���c��}�C�A���O�s���ӫȤ�ݪ��~��ip+port */
    clientInfo info[2];
    /* �@���߸��]�ݭn�������@�Ӧ줸�� */
    /* char ch; */
    char str[10] = { 0 };

    /* udp socket�y�z�� */
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

    /* ���A�������Ȥ�ݵo�Ӫ��T������o */
    while (1)
    {
        bzero(info, sizeof(clientInfo) * 2);
        /* ������Ӥ߸��]�ðO����P���s����ip+port */
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

        /* ���O�V��ӫȤ�ݶǰe��誺�~��ip+port */
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