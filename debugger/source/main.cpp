
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>

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

static int
FileExists(const char* Path)
{
    return access(Path, F_OK) != -1;
}

static int
FileSize(const char *Path)
{
    FILE *File = fopen (Path, "rb");
    fseek (File, 0, SEEK_END);
    int Length = ftell (File);
    fseek (File, 0, SEEK_SET);
    fclose (File);
    return Length;
}

static char *
SlurpFile(const char* Path)
{
    char *Buffer = 0;
    long Length;
    FILE *File = fopen (Path, "rb");
    fseek (File, 0, SEEK_END);
    Length = ftell (File);
    fseek (File, 0, SEEK_SET);
    Buffer = (char *)malloc (Length);
    fread (Buffer, 1, Length, File);
    fclose (File);
    return Buffer;
}

#include <sys/poll.h>

static int
HasInputWaiting()
{
    struct pollfd fds;
    int ret;
    fds.fd = 0; /* this is STDIN */
    fds.events = POLLIN;
    return poll(&fds, 1, 0);
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

    int sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0)  perror("create connection socket");

    sockaddr_in s;
    s.sin_family = AF_INET;
    s.sin_port = htons(DEBUGGER_PORT);
    s.sin_addr.s_addr = ClientAddr.s_addr;

    if (connect(sock,(struct sockaddr *)&s,sizeof(s)) < 0 ) {
        return -1;
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


    char Input[512];
    printf("\n> ");
    fflush(stdout);

    while (true)
    {

        if (HasInputWaiting())
        {
            for (int i = 0; i < 512; ++i)
            {
                Input[i] = 0;
            }

            scanf("%511s", Input);
            if (strncmp(Input, "load", 4) == 0)
            {
                for (int i = 0; i < 512; ++i)
                {
                    Input[i] = 0;
                }

                scanf("%511s", Input);
                printf("Loading kernel: %s\n", Input);
                int Size = FileSize(Input);
                if (FileExists(Input))
                {
                    printf("Slurping kernel of size %d bytes\n", Size);
                    char *Data = SlurpFile(Input);
                    printf("Sending to CTRX...\n");
                    printf("Sending Size...\n");
                    send(sock, &Size, sizeof(Size), 0);
                    printf("Sending Kernel...\n");
                    send(sock, Data, Size, 0);
                    printf("Done\n> ");
                    fflush(stdout);
                }
                else
                {
                    printf("File doesn't exist!\n> ");
                    fflush(stdout);
                }
            }
        }

//        {
//            char recvbuf[256];
//            socklen_t fromlen = sizeof(ClientAddr);
//            int Length = recvfrom(listenfd, recvbuf, sizeof(recvbuf), 0, (sockaddr *) &ClientSock, &fromlen);
//            if (Length != -1)
//            {
//                printf("\nClient: %s\n", recvbuf);
//            }
//        }

    }
}

