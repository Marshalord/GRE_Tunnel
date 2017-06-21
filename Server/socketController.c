#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <unistd.h>
#include "socketController.h"
#include "mysqlController.h"
#include "greController.h"
#include "ipExchange.h"

#define HELLO_WORLD_SERVER_PORT    6688 
#define LENGTH_OF_LISTEN_QUEUE 20
#define SIZE_OF_RECVBUFFER 12
#define SIZE_OF_SENDBUFFER 12

int listenBySocket()
{
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
 
    int server_socket = socket(AF_INET,SOCK_DGRAM,0);
    if( server_socket < 0)
    {
        printf("Create Socket Failed!");
        close(server_socket);
        exit(1);
    }

	//int opt =1;
	//setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
     
    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT); 
        close(server_socket);
        exit(1);
    }

    while (1)
    {
        unsigned char recvBuffer[SIZE_OF_RECVBUFFER];
        unsigned char sendBuffer[SIZE_OF_SENDBUFFER];
        SOCKETBUFFERUPLOAD *Upload;
        SOCKETBUFFERRESULT *Result;
        //Upload=(SOCKETBUFFERUPLOAD *)malloc(sizeof(SOCKETBUFFERUPLOAD));
        //Result=(SOCKETBUFFERRESULT *)malloc(sizeof(SOCKETBUFFERRESULT));
        memset(Upload, 0, sizeof(SOCKETBUFFERUPLOAD));
        memset(Result, 0, sizeof(SOCKETBUFFERRESULT));
        socklen_t length = sizeof(server_addr);
         
        //length = recv(server_socket,recvBuffer,SIZE_OF_RECVBUFFER,0);
        struct sockaddr_in remoteAddr;
        int nAddrLen = sizeof(remoteAddr); 
        length = recvfrom(server_socket, recvBuffer, SIZE_OF_RECVBUFFER, 0, (struct sockaddr *)&remoteAddr, &nAddrLen);
        memcpy(Upload, recvBuffer, SIZE_OF_RECVBUFFER);
        if (length < 0)
        {
            printf("Server Recieve Data Failed!\n");
            close(server_socket);
            break;
        }

        //command
        unsigned short recvCommand=0;
        recvCommand=Upload->Command;
        if (recvCommand!=1000)
        {
            continue;
        }

        //is recived IP illegal
        printf("recv IP = %08x\n", Upload->IP);
        int isLocalIPFlag=isLocalIP(Upload->IP);
        if (isLocalIPFlag==1)
        {
            continue;
        }

        //convert the received IP from unsigned int to char* 
        unsigned char *bytes = (unsigned char *) &Upload->IP; 
        unsigned char strIP[15]; 
        sprintf (strIP, "%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);

        int isFindByID=0;
        isFindByID=FindByID(Upload->Serial);
        //printf("isFindByID:%d\n", isFindByID);

        char *tunnelName;
        tunnelName=(char *)malloc(6*sizeof(char));
        sprintf(tunnelName,"t_%d",Upload->Serial);

        //find if there is old record
        if (isFindByID==1){
            printf("It's already build ID=%d\n",Upload->Serial);

            //convert IP in mysql from unsigned int to char* 
            unsigned int oldIP=FindIP(Upload->Serial);
            unsigned char *bytes_old = (unsigned char *) &oldIP;
            unsigned char strIP_old[15]; 
            sprintf (strIP_old, "%d.%d.%d.%d", bytes_old[3], bytes_old[2], bytes_old[1], bytes_old[0]);

            //delete rule(route) table and dev
            greTunnelDel(Upload->Serial,strIP_old);

            int isDeleteParm=0;
            //delete old mysql row
            isDeleteParm=DeleteParm(Upload->Serial);
            //confirm old mysql row delete
            if (isDeleteParm==0){
                printf("Success to delete ID=%d\n",Upload->Serial);

                //distribution REMOTE_TUNNEL_IP
                unsigned int remoteTunnelIP=0xA000000+CountIP();
                unsigned char *bytes_remoteTunnelIP = (unsigned char *) &remoteTunnelIP;
                unsigned char strIP_remoteTunnel[15]; 
                sprintf (strIP_remoteTunnel, "%d.%d.%d.%d", bytes_remoteTunnelIP[3], bytes_remoteTunnelIP[2], bytes_remoteTunnelIP[1], bytes_remoteTunnelIP[0]);

                //build new rule(route) table and dev
                greTunnelConfig(Upload->Serial,strIP,strIP_remoteTunnel);
                //insert new mysql row
                InsertParm(Upload->Serial,tunnelName,Upload->IP,remoteTunnelIP);
            }else
            {
                printf("Fail to delete ID=%d\n",Upload->Serial);
            }
        }else if (isFindByID==0)
        {   //distribution REMOTE_TUNNEL_IP
            unsigned int remoteTunnelIP=0xA000000+CountIP();
            unsigned char *bytes_remoteTunnelIP = (unsigned char *) &remoteTunnelIP;
            unsigned char strIP_remoteTunnel[15]; 
            sprintf (strIP_remoteTunnel, "%d.%d.%d.%d", bytes_remoteTunnelIP[3], bytes_remoteTunnelIP[2], bytes_remoteTunnelIP[1], bytes_remoteTunnelIP[0]);

            //insert new mysql row
            greTunnelConfig(Upload->Serial,strIP,strIP_remoteTunnel);
            InsertParm(Upload->Serial,tunnelName,Upload->IP,remoteTunnelIP);
        }
        //confirm new row in mysql
        isFindByID=FindByID(Upload->Serial);
        //Result->Result:0,Fail;1,Success;2,Rebuild
        if (isFindByID==1){
            Result->Result=1;
            printf("Success to rebuild ID=%d with IP=%s\n",Upload->Serial,strIP);
        }
        else{
            Result->Result=0;
            printf("Fail to rebuild ID=%d with IP=%s\n",Upload->Serial,strIP);
        }

        Result->Command=1001;
        Result->Serial=Upload->Serial;
        Result->Length=SIZE_OF_SENDBUFFER;
        memcpy(sendBuffer,Result,SIZE_OF_SENDBUFFER);
        
        //send result
        int retSend=0;
        //retSend=send(server_socket,sendBuffer,SIZE_OF_SENDBUFFER,0);
        retSend=sendto(server_socket, sendBuffer, SIZE_OF_SENDBUFFER, 0, (struct sockaddr *)&remoteAddr, nAddrLen);   
        //free(Upload);
        //free(Result);
        memset(Upload, 0, sizeof(SOCKETBUFFERUPLOAD));
        memset(Result, 0, sizeof(SOCKETBUFFERRESULT));
        free(tunnelName); 
    }
    close(server_socket);

    return 0;
}