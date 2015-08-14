
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstring>

#include "debugger.h"

static int
FindCTRXClient(in_addr *ClientAddr) {
    char TempBuffer[256];

    int BroadcastSocket = socket(PF_INET, SOCK_DGRAM, 0);
    if(BroadcastSocket < 0)
    {
        printf("Could not create socket\n");
        return -1;
    }

    int optval = 1;
    setsockopt(BroadcastSocket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval));

    sockaddr_in SocketInfo = {};
    SocketInfo.sin_family = AF_INET;
    SocketInfo.sin_port = htons(DEBUGGER_PORT);
    SocketInfo.sin_addr.s_addr = INADDR_BROADCAST;

    sockaddr_in RemoteSocketInfo = {};
    RemoteSocketInfo.sin_family = AF_INET;
    RemoteSocketInfo.sin_port = htons(DEBUGGER_PORT);
    RemoteSocketInfo.sin_addr.s_addr = INADDR_ANY;

    int ReceiveSocket = socket(PF_INET, SOCK_DGRAM, 0);
    if (ReceiveSocket < 0)
    {
        printf("Could not create socket\n");
        return -1;
    }

    if(bind(ReceiveSocket, (struct sockaddr*) &RemoteSocketInfo, sizeof(RemoteSocketInfo)) < 0)
    {
        printf("Could not bind socket\n");
        return -1;
    }
    fcntl(ReceiveSocket, F_SETFL, O_NONBLOCK);


    int Timeout = 100;
    int Length;
    while(Timeout) {
        if(sendto(BroadcastSocket, DEBUGGER_CLIENT_STRING, strlen(DEBUGGER_CLIENT_STRING), 0, (struct sockaddr *)&SocketInfo, sizeof(SocketInfo)) < 0)
        {
            printf("Could not send to broadcast socket\n");
        }
        --Timeout;
        socklen_t socklen = sizeof(RemoteSocketInfo);
        memset(TempBuffer, 0, sizeof(TempBuffer));
        Length = recvfrom(ReceiveSocket, TempBuffer, sizeof(TempBuffer), 0, (struct sockaddr *)&RemoteSocketInfo, &socklen);
        if (Length != -1) {
            if ( strncmp(DEBUGGER_HOST_STRING, TempBuffer, strlen(DEBUGGER_HOST_STRING)) == 0) {
                break;
            }
        }

        usleep(10000);
    }
    if (Timeout == 0) RemoteSocketInfo.sin_addr.s_addr =  INADDR_NONE;
    close(BroadcastSocket);
    close(ReceiveSocket);
    *ClientAddr = RemoteSocketInfo.sin_addr;
    return 0;
}

int main(int argc, char **argv)
{
    printf("Searching for CTRX client...");
    in_addr ClientAddr = {};
    if (FindCTRXClient(&ClientAddr))
    {
        printf("Could not find CTRX client\n");
        return -1;
    }

    if (ClientAddr.s_addr == INADDR_NONE) {
        printf("No response from CTRX\n");
        return 1;
    }

    int listenfd = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in ClientSock = {};
    ClientSock.sin_family = AF_INET;
    ClientSock.sin_port = htons(DEBUGGER_PORT);
    ClientSock.sin_addr.s_addr = INADDR_ANY;
    if (bind(listenfd, (sockaddr *)&ClientSock, sizeof(ClientSock)) < 0)
    {
        printf("Failed binding listen socket\n");
        return -1;
    }

    while (true)
    {
        char recvbuf[256];
        socklen_t fromlen = sizeof(ClientAddr);
        int Length = recvfrom(listenfd, recvbuf, sizeof(recvbuf), 0, (sockaddr *) &ClientSock, &fromlen);
        if (Length != -1)
        {
            printf("%s\n", recvbuf);
        }
    }
}