#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstring>
#include "debugger.h"

#include <3ds.h>
#include <malloc.h>



static int udpfd = -1;
static int listenfd = -1;
static int datafd = -1;
static sockaddr_in host_dbg;

static void *SOC_buffer = NULL;

u8 OutputBuffer[4 * 1024];

static int
CTRInitNetwork()
{
    SOC_buffer = memalign(0x1000, 0x100000);
    if(SOC_buffer == NULL)
        return -1;

    Result ret = SOC_Initialize((u32 *)SOC_buffer, 0x100000);
    if(ret != 0)
    {
        // need to free the shared memory block if something goes wrong
        SOC_Shutdown();
        free(SOC_buffer);
        SOC_buffer = NULL;
        return -1;
    }
    return 0;
}

static int
CTRShutdownNetwork()
{
    Result ret = SOC_Shutdown();
    if(ret != 0)
        return -1;
    return 0;
}

static int
set_socket_nonblocking(int sock) {

    int flags = fcntl(sock, F_GETFL);

    if(flags == -1) return -1;

    int rc = fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    if(rc != 0) return -1;
    
    return 0;
}

int DebuggerOpen()
{
    if (CTRInitNetwork())
    {
        printf("Could not start network\n");
    }

    if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Could not create socket\n");
        return -1;
    }

    sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(DEBUGGER_PORT);

    if (bind(udpfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Failed binding UDP socket\n");
        return -1;
    }

    if (set_socket_nonblocking(udpfd) == -1)
    {
        printf("Could not set nonblocking\n");
        return -1;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(listenfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Failed binding listen socket\n");
        return -1;
    }

    if (set_socket_nonblocking(listenfd) == -1)
    {
        printf("Could not set nonblocking\n");
        return -1;
    }

    if (listen(listenfd, 10) != 0)
    {
        printf("Could not listen on socket\n");
        return -1;
    }

    printf("Waiting for debugger to connect...");

    char recvbuf[256];
    socklen_t fromlen = sizeof(host_dbg);
    while (aptMainLoop())
    {
        int len = recvfrom(udpfd, recvbuf, sizeof(recvbuf), 0, (sockaddr*) &host_dbg, &fromlen);
        if (len!=-1) {
            if (strncmp(recvbuf, DEBUGGER_CLIENT_STRING, strlen(DEBUGGER_CLIENT_STRING)) == 0) {
                host_dbg.sin_family=AF_INET;
                host_dbg.sin_port=htons(DEBUGGER_PORT);
                sendto(udpfd, DEBUGGER_HOST_STRING, strlen(DEBUGGER_HOST_STRING), 0, (sockaddr*) &host_dbg, sizeof(host_dbg));
                break;
            }
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
    }

    printf("connected\n");

    return 0;
}

int DebuggerGetCommand(dbg_command *Cmd)
{
    if (datafd < 0)
    {
        datafd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    }
    Cmd->Cmd = DEBUGGER_CMD_NONE;
    int DataLength;
    int Length = recv(datafd, &DataLength, sizeof(DataLength), 0);
    if (Length != -1)
    {
        Cmd->Data = malloc(DataLength);
        Length = recv(datafd, Cmd->Data, DataLength, 0);
        if (Length != -1)
        {
            Cmd->PayloadSize = Length;
            Cmd->Cmd = DEBUGGER_CMD_LOAD_KERNEL;
            return Cmd->Cmd;
        }
    }
    return 0;
}

int DebuggerClose()
{
    if (listenfd >= 0)
    {
        closesocket(listenfd);
        listenfd = -1;
    }

    if (udpfd >= 0)
    {
        closesocket(udpfd);
        udpfd = -1;
    }

    return CTRShutdownNetwork();
}

int DebuggerPrint(const char *str)
{
    if (udpfd < 0) return -1;
    memset(OutputBuffer, 0, sizeof(OutputBuffer));
    unsigned int Length = strlen(str) + 1;
    Length = (Length >= sizeof(OutputBuffer) ? sizeof(OutputBuffer) : Length);
    memcpy(OutputBuffer, str, Length);
    sendto(udpfd, OutputBuffer, Length, 0, (sockaddr *)&host_dbg, sizeof(host_dbg));
    return 0;
}
