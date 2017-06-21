#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/rtnetlink.h>    //for rtnetlink    
#include <net/if.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h> //for strstr(), memset()   

/* udp socket */
#define SERVER_PORT 6688
//#define CLIENT_PORT 22334
#define SEND_MSG_MAX 128
#define RECV_MSG_MAX 128
#define DELAY_TIME 60

/* gre tunnel config. */
#define SIZE_OF_SHELL 200
#define REMOTE_IP "118.249.86.223"
#define LOCAL_TUNNEL_IP "10.10.10.1"
#define REMOTE_TUNNEL_IP "10.10.10.2"

/* REQ MSG */
#define GRE_REQUEST_CMD 1000

/* REPLY MSG */
#define GRE_REPLY_CMD 1001
#define GRE_REPLY_FAILED 0
#define GRE_REPLY_SUCCEED 1
#define GRE_REPLY_ESTABLISHED 2

/* get gw ip. */
#define BUFSIZE 2048 

typedef struct msg_head{
    unsigned short length;
    unsigned short command;
    int devid;
}msg_head;

typedef struct gre_req_msg
{
    msg_head head;
    unsigned int uiIP;
}gre_req_msg;

typedef struct gre_rep_msg
{
    msg_head head;
    unsigned int uiTunnelIP;
    unsigned char result;
}gre_rep_msg;

struct route_info{     
 u_int dstAddr;     
 u_int srcAddr;     
 u_int gateWay;     
 char ifName[IF_NAMESIZE];     
};   

int g_sockfd;
unsigned int g_ui_IP;
struct sockaddr_in g_servaddr;
struct sockaddr_in g_cliaddr;
int g_dev_id;
unsigned int g_gateway;
char g_ifName[IF_NAMESIZE];

/* get device id */
extern unsigned int get_devid(void);

int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)     
{     
  struct nlmsghdr *nlHdr;     
  int readLen = 0, msgLen = 0;     
  do{     
    //收到内核的应答     
    if((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0)     
    {     
      perror("SOCK READ: ");     
      return -1;     
    }     
        
    nlHdr = (struct nlmsghdr *)bufPtr;     
    //检查header是否有效     
    if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))     
    {     
      perror("Error in recieved packet");     
      return -1;     
    }     
    
    if(nlHdr->nlmsg_type == NLMSG_DONE)      
    {     
        break;     
    }     
    else     
    {       
      bufPtr += readLen;     
      msgLen += readLen;     
    }       
    if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)      
    {          
        break;     
    }     
  } while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId)); 
  
  return msgLen;     
}     
//分析返回的路由信息     
void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo)     
{     
    struct rtmsg *rtMsg;     
    struct rtattr *rtAttr;     
    int rtLen;     
    char *tempBuf = NULL;     
    struct in_addr dst;     
    struct in_addr gate;     
       
    tempBuf = (char *)malloc(100);     
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);     
  // If the route is not for AF_INET or does not belong to main routing table
  //then return.      
  if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN)
)     
    return;     
       
    rtAttr = (struct rtattr *)RTM_RTA(rtMsg);     
    rtLen = RTM_PAYLOAD(nlHdr);     
    for(;RTA_OK(rtAttr,rtLen);rtAttr = RTA_NEXT(rtAttr,rtLen))
    {     
        switch(rtAttr->rta_type) {     
        case RTA_OIF:     
            if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);     
            break;     
        case RTA_GATEWAY:     
            rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);     
            break;     
        case RTA_PREFSRC:     
            rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);     
            break;     
        case RTA_DST:     
            rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);     
            break;     
        }     
    }     
    dst.s_addr = rtInfo->dstAddr;     
    if (strstr((char *)inet_ntoa(dst), "0.0.0.0"))     
    {     
        g_gateway = ntohl(rtInfo->gateWay);
        memcpy(g_ifName, rtInfo->ifName, strlen(rtInfo->ifName));
    }     
    free(tempBuf);     
    return;     
}     
     
int get_gateway()     
{     
    struct nlmsghdr *nlMsg;     
    struct rtmsg *rtMsg;     
    struct route_info *rtInfo;     
    char msgBuf[BUFSIZE];     
      
    int sock, len, msgSeq = 0;     
     
    if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)     
    {     
        perror("Socket Creation: ");     
        return -1;     
    }     
    
    memset(msgBuf, 0, BUFSIZE);     
            
 nlMsg = (struct nlmsghdr *)msgBuf;     
 rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);          
      
 nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.     
 nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table.     
      
 nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.     
 nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.     
 nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.     
      
      
 if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0){     
    printf("Write To Socket Failed…\n");     
    return -1;     
 }     
      
      
 if((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0) {     
    printf("Read From Socket Failed…\n");     
    return -1;     
 }     
      
    rtInfo = (struct route_info *)malloc(sizeof(struct route_info));     
    for(;NLMSG_OK(nlMsg,len);nlMsg = NLMSG_NEXT(nlMsg,len))
    {     
        memset(rtInfo, 0, sizeof(struct route_info));     
        parseRoutes(nlMsg, rtInfo);     
    }     
    free(rtInfo);     
    close(sock);     
    return 0;     
}     

int IPstr2uint(char *lpStr)
{
    int a = 0, b = 0, c = 0, d = 0;
    unsigned int uiIP = 0;
    if (lpStr == NULL || strlen(lpStr) == 0)
        return 0;

    if (sscanf( lpStr, "%d.%d.%d.%d", &a, &b, &c, &d ) != 4)
        return 0;

    if (a > 0 && a <= 255 && b >= 0 && b <= 255 && c >= 0 && c <= 255 && d >= 0 && d <= 255)
        uiIP = (a << 24) + (b << 16) + (c << 8) + d;
    return uiIP;
}

int ChangeIPtoStr(unsigned int uiIP, char *strIP)  
{  
  unsigned char *bytes = (unsigned char *)&uiIP;  
  sprintf(strIP, "%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
  printf("changeIPtoStr:%u.%u.%u.%u\n", bytes[3], bytes[2], bytes[1], bytes[0]);
  return 0;  
}

int isLocalIP(unsigned int uiCheckIP)
{
    unsigned int uiBegin;
    unsigned int uiEnd;

    uiBegin = 0x0a000000;//A:10.0.0.0
    uiEnd   = 0x0affffff;//A:10.255.255.255

    if((0 == uiCheckIP) || (0x17f7ffd == uiCheckIP))
    {
        printf("the IP %08x is vir1 IP.\n", uiCheckIP);
        return 1;
    }
    
    if((uiBegin < uiCheckIP)&&(uiCheckIP < uiEnd))
    {
        printf("the IP %08x is A local IP.\n", uiCheckIP);
        return 1;
    }

    uiBegin = 0xac100000;//B:172.16.0.0
    uiEnd   = 0xac1fffff;//B:172.31.255.255
    if((uiBegin < uiCheckIP)&&(uiCheckIP < uiEnd))
    {
        printf("the IP %08x is B local IP.\n", uiCheckIP);
        return 1;
    }
    
    uiBegin = 0xc0a80000;//C:192.168.0.0
    uiEnd   = 0xc0a8ffff;//C:192.168.255.255
    if((uiBegin < uiCheckIP)&&(uiCheckIP < uiEnd))
    {
        printf("the IP %08x is C local IP.\n", uiCheckIP);
        return 1;
    }
    

    uiBegin = 0x7f000000;//loop:127.0.0.0
    uiEnd   = 0x7fffffff;//loop:127.255.255.255
    if((uiBegin < uiCheckIP)&&(uiCheckIP < uiEnd))
    {
        printf("the IP %08x is loop local IP.\n", uiCheckIP);
        return 1;
    }

    uiBegin = 0x64400000;//operators level nat:100.64.0.0
    uiEnd   = 0x647fffff;//operators level nat:100.127.255.255
    if((uiBegin < uiCheckIP)&&(uiCheckIP < uiEnd))
    {
        printf("the IP %08x is nat local IP.\n", uiCheckIP);
        return 1;
    }    

    return 0;
}

int greTunnelConfig(char *tunnelName, char *localIP, char *tunnelIP)
{
	char *ipTunnelAdd;
	char *ipLinkSet;
	char *ipAddrAdd;
	//char *ifconfig;

	ipTunnelAdd=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipLinkSet=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipAddrAdd=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	//ifconfig=(char *)malloc(SIZE_OF_SHELL*sizeof(char));

	sprintf(ipTunnelAdd, "ip tunnel add %s mode gre remote %s local %s ttl 255",tunnelName,REMOTE_IP, localIP);
	sprintf(ipLinkSet,"ip link set %s up",tunnelName);
	sprintf(ipAddrAdd,"ip addr add %s peer %s dev %s",tunnelIP, REMOTE_TUNNEL_IP, tunnelName);
	//sprintf(ifconfig,"ifconfig -a|grep %s",tunnelName);

    printf("ipTunnelAdd = %s\n", ipTunnelAdd);
	system(ipTunnelAdd);
	printf("ipLinkSet = %s\n", ipLinkSet);
	system(ipLinkSet);
	printf("ipAddrAdd = %s\n", ipAddrAdd);
	system(ipAddrAdd);
	//system(ifconfig);
	free(ipTunnelAdd);
	free(ipLinkSet);
	free(ipAddrAdd);
}

int greTunnelDel(char *tunnelName){
	char *ipTunnelDel;
	//char *ifconfig;

	ipTunnelDel=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	//ifconfig=(char *)malloc(SIZE_OF_SHELL*sizeof(char));

	sprintf(ipTunnelDel, "ip tunnel del %s", tunnelName);
	//sprintf(ifconfig,"ifconfig -a");
    printf("ipTunnelDel = %s\n", ipTunnelDel);
	system(ipTunnelDel);
	//system(ifconfig);
	free(ipTunnelDel);
}

/* 配置TEE命令，将主动访问WAN口的流量导入GRE隧道 ，将GRE隧道的流量导入默认网关 */
void xt_TEE_cfg(char *strGREIP, char *strIP, char *strGW, char *ifName)
{
    char *pcForward_gre;
	char *pcForward_gw;

	pcForward_gre = (char *)malloc(SIZE_OF_SHELL * sizeof(char));
	pcForward_gw = (char *)malloc(SIZE_OF_SHELL * sizeof(char));

    sprintf(pcForward_gre,"iptables -t mangle -A INPUT -i %s -m state --state new -j TEE --gateway %s", ifName, REMOTE_TUNNEL_IP);
    sprintf(pcForward_gw, "iptables -t mangle -A PREROUTING -i gre1 -s %s -j TEE --gateway %s", strIP, strGW);

    printf("pcForward_gre = %s\n", pcForward_gre);
	system(pcForward_gre);

	printf("pcForward_gw = %s\n", pcForward_gw);
	system(pcForward_gw);  

    free(pcForward_gre);
    free(pcForward_gw);
	return;
}

void xt_TEE_del()
{
    char *iptablesDel;

    iptablesDel = (char *)malloc(SIZE_OF_SHELL * sizeof(char));
    memset(iptablesDel, 0, SIZE_OF_SHELL * sizeof(char));
    sprintf(iptablesDel, "iptables -t mangle -D PREROUTING 1");
    printf("iptablesDel = %s\n", iptablesDel);
    system(iptablesDel);
    memset(iptablesDel, 0, SIZE_OF_SHELL * sizeof(char));
    sprintf(iptablesDel, "iptables -t mangle -D INPUT 1");
    printf("iptablesDel = %s\n", iptablesDel);
    system(iptablesDel);
    free(iptablesDel);
}

unsigned int GetLocalIp()  
{        
    int MAXINTERFACES = 16;  
    unsigned int ip = 0;  
    int fd, intrface, retn = 0;    
    struct ifreq buf[MAXINTERFACES];    
    struct ifconf ifc;    

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)    
    {    
        ifc.ifc_len = sizeof(buf);    
        ifc.ifc_buf = (caddr_t)buf;    
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))    
        {    
            intrface = ifc.ifc_len / sizeof(struct ifreq);    

            while (intrface-- > 0)    
            {    
                if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))    
                {    
                    ip = (((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr.s_addr); 
                    ip = ntohl(ip);
                    printf("ioctl find ip = %x\n", ip);
                    if(!isLocalIP(ip))
                        break;  
                }                        
            }  
        }    
        close (fd);      
    } 
    if(!isLocalIP(ip))
        return ip;   
    else
        return 0;
} 

/* 获取WAN口的IP，如果此IP不是公网IP就返回0 */
unsigned int getWan_ip()
{
    unsigned int uiIP = 0;
    printf("before GetLocalIp() in getWan_ip uiIP = %08x\n\n", uiIP);
    uiIP = GetLocalIp();
    printf("after GetLocalIp() in getWan_ip uiIP = %08x\n", uiIP);

    return uiIP;
}

unsigned int getWan_ip_static()
{
    return 0x01020304;
}

/* change gre para when ip was changed. */
int update_gre_tunnel(unsigned int tunnel_IP)
{   
    char strIP[16];
    char strTunnelIP[16];
    char strGW[16];

    greTunnelDel("gre1");
    
    if(0 != g_ui_IP)
    {
        ChangeIPtoStr(g_ui_IP, strIP);
        ChangeIPtoStr(tunnel_IP, strTunnelIP);
        greTunnelConfig("gre1", strIP, strTunnelIP);
        get_gateway();

        xt_TEE_del();
        ChangeIPtoStr(g_gateway, strGW);
        printf("g_ifName = %s strGW = %s\n", g_ifName, strGW);
        xt_TEE_cfg(LOCAL_TUNNEL_IP, strIP, strGW, g_ifName);
    }
    return 0;
}

/* 根据收到的隧道IP，配置GRE隧道命令 */
int gre_init(unsigned int tunnel_IP)
{
    char strIP[16];
    char strTunnelIP[16];
    char strGW[16];

    //g_ui_IP = getWan_ip_static();
    //g_dev_id = 147258;
    
    greTunnelDel("gre1");
    ChangeIPtoStr(tunnel_IP, strTunnelIP);
    ChangeIPtoStr(g_ui_IP, strIP);
    greTunnelConfig("gre1", strIP, strTunnelIP);

    /* xt_TEE_config */
    get_gateway();
    ChangeIPtoStr(g_gateway, strGW);
    xt_TEE_del();
    printf("g_ifName = %s strGW = %s\n", g_ifName, strGW);
    xt_TEE_cfg(LOCAL_TUNNEL_IP, strIP, strGW, g_ifName);
    
    return 0;
}

/* create global socket fd. */
int socket_init()
{   
    g_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    memset(&g_servaddr, 0, sizeof(g_servaddr));
    //memset(&g_cliaddr, 0, sizeof(g_cliaddr));
    
    g_servaddr.sin_family = AF_INET;
    g_servaddr.sin_port = htons(SERVER_PORT);  
    g_servaddr.sin_addr.s_addr = inet_addr(REMOTE_IP);

    /*
    g_cliaddr.sin_family = AF_INET;
    g_cliaddr.sin_port = htons(CLIENT_PORT);  
    g_cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(g_sockfd, (struct sockaddr*)&g_cliaddr, sizeof(g_cliaddr)); 
    */
    return 0;
}

int sendMsg(void *pbuf, int size)
{
    socklen_t length = sizeof(g_servaddr);
    int len = 0;

    len = sendto(g_sockfd, (void *)pbuf, size, 0, (struct sockaddr*)(&g_servaddr), length);
    return len;
}

int send_req(void *pbuf, unsigned int uiIP)
{
    gre_req_msg *pstReqMsg = NULL;
    int ret = 0;
    pstReqMsg = (gre_req_msg *)pbuf;
    
    pstReqMsg->uiIP = uiIP;
    pstReqMsg->head.length = sizeof(gre_req_msg);
    pstReqMsg->head.command = GRE_REQUEST_CMD;
    pstReqMsg->head.devid = g_dev_id;

    ret = sendMsg(pbuf, pstReqMsg->head.length);
    printf("send_req send len = %d\n", ret);
    return ret;
}

int recv_msg(void *pbuf)
{
    int ret = 0;
    int length = sizeof(g_servaddr);
    ret = recvfrom(g_sockfd, pbuf, RECV_MSG_MAX, 0,(struct sockaddr*)(&g_servaddr),&length);
    return ret;
}

static void handle_reply_msg(void *pbuf)
{
    gre_rep_msg *pstRecvMsg = NULL;
    unsigned char result = 0;
    unsigned int tunnel_local_ip = 0;
    int devid;
    pstRecvMsg = (gre_rep_msg *)pbuf;

    devid = pstRecvMsg->head.devid;
    if(devid != g_dev_id)
    {
        printf("RECV_MSG ERROR:recv devid(%d)!= my_dev_id(%d)", devid, g_dev_id);
        return;
    }

    result = pstRecvMsg->result;

    /* 云端隧道配置已建立，并为客户端分配了隧道IP成功 */
    if(GRE_REPLY_SUCCEED == result)
    {
        tunnel_local_ip = pstRecvMsg->uiTunnelIP;
        printf("gre tunnel bulid success. tunnel_local_ip = %x\n", tunnel_local_ip);
        gre_init(tunnel_local_ip);
    }
    else if(GRE_REPLY_FAILED == result)
    {
        printf("gre tunnel bulid failed\n");
    }
    else if(GRE_REPLY_ESTABLISHED == result)
    {
        printf("gre tunnel has been established before.\n");
    }
    else
    {
        printf("cannot resolve result.\n");
    }
 
    return;
}

static void handle_err(void *pbuf)
{
    printf("cmd err.\n");
    printf("pbuf = %s", (char *)pbuf);
    return;
}

static void handle_msg(void *pbuf)
{
    msg_head *pstHead = NULL;
    unsigned short command;

    pstHead = (msg_head *)pbuf;
    command = pstHead->command;

    switch(command){
    case GRE_REPLY_CMD:
        handle_reply_msg(pbuf);
        break;
    default:
        handle_err(pbuf);
        break;
    }

    return;
}

int send_and_handle_msg(unsigned int sendIP, unsigned char *send_buf, unsigned char *recv_buf)
{
    struct timeval tv_out;
    fd_set rdfdset;
    int ret = 0;

    tv_out.tv_sec = 5;
    tv_out.tv_usec = 0;

    FD_ZERO(&rdfdset);
    FD_SET(g_sockfd, &rdfdset);
    memset(send_buf, 0, SEND_MSG_MAX);
    memset(recv_buf, 0, RECV_MSG_MAX);
    send_req((void *)send_buf, sendIP);
    ret = select(g_sockfd + 1, &rdfdset, NULL, NULL, &tv_out);
    
    if(FD_ISSET(g_sockfd, &rdfdset))
    {
        recv_msg((void *)recv_buf);
        handle_msg((void *)recv_buf);
    }
    
    return ret;
}

/* 开放服务器远程主动访问权限(暂未使用) */
void iptables_close()
{
    char cmd[128] = "iptables -I INPUT 1 -s 118.249.86.223 -j ACCEPT";
    
    printf("iptables cmd = %s\n", cmd);
	system(cmd);
    return;
}

/* 循环向服务器发送请求，直到收到服务器的正确回复并处理结果 */
void get_local_tunnel_ip(unsigned char *send_buf, unsigned char *recv_buf)
{
    int ret = 0;

    while(1){
        g_ui_IP = getWan_ip();
        if(0 == g_ui_IP){
            sleep(DELAY_TIME * DELAY_TIME);
            continue;
        }
        else{
            ret = send_and_handle_msg(g_ui_IP, send_buf, recv_buf);
            if(ret > 0){ 
                break;
            }
            else{
                sleep(DELAY_TIME);
                continue;
            }
        }
    }

    return;
}

void main(void)
{
    unsigned int uiWan_ip;
    unsigned char send_buf[SEND_MSG_MAX];
    unsigned char recv_buf[RECV_MSG_MAX];
    unsigned int uiReply = 0;

    int ret = 0;
    sleep(60);
    
    socket_init(); 
    //iptables_close();    

    g_dev_id = get_devid();
    get_local_tunnel_ip(send_buf, recv_buf);

    while(1)
    {
        uiWan_ip = getWan_ip();
        //uiWan_ip = getWan_ip_static();
        printf("uiWan_ip == %08x\n", uiWan_ip);
        if((g_ui_IP != uiWan_ip) && (0 != uiWan_ip))
        {
            get_local_tunnel_ip(send_buf, recv_buf);
        }
        
        sleep(DELAY_TIME);
        uiReply++;
        if(60 == uiReply)
        {
            uiReply = 0;

            get_local_tunnel_ip(send_buf, recv_buf);
        }
    }
    
}
