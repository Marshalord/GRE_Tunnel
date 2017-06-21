#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <mysql.h>


#define SIZE_OF_SHELL 200
#define LOCAL_IP "118.249.86.223"
#define LOCAL_TUNNEL_IP "10.10.10.2"
#define REMOTE_TUNNEL_IP "10.10.10.1"
int greTunnelConfig(unsigned int ID,char *IP,char *remoteTunnelIP){

	MYSQL           mysql;
    MYSQL_RES       *res = NULL;
    MYSQL_ROW       row;
    char            query_str[200];
    int             rc, i, fields;
    int             rows;
    if (NULL == mysql_init(&mysql)) {
        printf("mysql_init(): %s\n", mysql_error(&mysql));
        return -1;
    }
    if (NULL == mysql_real_connect(&mysql,
                "localhost",
                "root",
                "wiair666",
                "wiair",
                3306,
                NULL,
                0)) {
        printf("mysql_real_connect(): %s\n", mysql_error(&mysql));
        mysql_free_result(res);
        mysql_close(&mysql);
        return -1;
    }
    //printf("Connected MySQL successful! \n");

     //show all data
    int t;
    int id;
    unsigned int remoteIP;
    sprintf(query_str,"select id,remote_public_ip,local_private_ip from GRE_Tunnel_parameter");
    rc = mysql_real_query(&mysql, query_str, (unsigned int)strlen(query_str));
    
    res = mysql_store_result(&mysql); //
    while(row=mysql_fetch_row(res))
    {

        if(mysql_num_fields(res)<1)
        {
       		char *ipRuleAddF;
			char *ipRouteAddF;
            

           	ipRuleAddF=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
			ipRouteAddF=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
            

            sprintf(ipRuleAddF,"sudo ip rule add from 10.10.1.253 table 1");
			sprintf(ipRouteAddF,"sudo ip route add default via 192.168.11.2 table 1");
			
                
                //get root permission
            int ruidF,euidF,suidF;
            getresuid(&ruidF,&euidF,&suidF);
            setresuid(ruidF,suidF,suidF);

            system(ipRuleAddF);
            system(ipRouteAddF);
                

                //return to user permission
            getresuid(&ruidF,&euidF,&suidF);
            setresuid(ruidF,ruidF,suidF);

            free(ipRuleAddF);
            free(ipRouteAddF);
        }
    }
    mysql_close(&mysql);

	char *ipTunnelAdd;
	char *ipLinkSet;
	char *ipAddrAdd;
	char *ifconfig;
	char *ipRuleAddRight;
	//char *ipRouteAddRight;
	char *ipRuleAddLeft;
	char *ipRouteAddLeft;
	char *ipRuleDelF;

	ipTunnelAdd=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipLinkSet=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipAddrAdd=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ifconfig=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipRuleAddRight=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	//ipRouteAddRight=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipRuleAddLeft=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipRouteAddLeft=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipRuleDelF=(char *)malloc(SIZE_OF_SHELL*sizeof(char));

	sprintf(ipTunnelAdd, "sudo ip tunnel add gre%d mode gre remote %s local %s ttl 255",ID,IP,LOCAL_IP);
	sprintf(ipLinkSet,"sudo ip link set gre%d up",ID);
	sprintf(ipAddrAdd,"sudo ip addr add %s peer %s dev gre%d",LOCAL_TUNNEL_IP,remoteTunnelIP,ID);
	sprintf(ifconfig,"ifconfig -a|grep gre%d",ID);
	sprintf(ipRuleAddRight,"sudo ip rule add dev gre%d table 1",ID);
	//sprintf(ipRouteAddRight,"sudo ip route add default via 192.168.1.140 table 1");
	sprintf(ipRuleAddLeft,"sudo ip rule add from %s table %d",IP,ID);
	sprintf(ipRouteAddLeft,"sudo ip route add default dev gre%d table %d",ID,ID);
	sprintf(ipRuleDelF,"sudo ip rule del from 10.10.1.253 table 1");

	//get root permission
    int ruid,euid,suid;
    getresuid(&ruid,&euid,&suid);
    setresuid(ruid,suid,suid);

	system(ipTunnelAdd);
	system(ipLinkSet);
	system(ipAddrAdd);
	system(ifconfig);
	system(ipRuleAddRight);
	//system(ipRouteAddRight);
	system(ipRuleAddLeft);
	system(ipRouteAddLeft);
	system(ipRuleDelF);

	//return to user permission
   	getresuid(&ruid,&euid,&suid);
    setresuid(ruid,ruid,suid);

    free(ipTunnelAdd);
    free(ipLinkSet);
    free(ipAddrAdd);
    free(ifconfig);
    free(ipRuleAddRight);
    free(ipRuleAddLeft);
    free(ipRouteAddLeft);
    free(ipRuleDelF);

    return 0;
}

int greTunnelDel(unsigned int ID,char *IP){
	char *ipTunnelDel;
	char *ifconfig;
	char *ipRuleDelRight;
	char *ipRuleDel;
	char *ipRouteDel;

	ipTunnelDel=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ifconfig=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
    ipRuleDelRight=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipRuleDel=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipRouteDel=(char *)malloc(SIZE_OF_SHELL*sizeof(char));

	sprintf(ipTunnelDel, "sudo ip tunnel del gre%d",ID);
	sprintf(ifconfig,"ifconfig -a|grep gre%d",ID);
	sprintf(ipRuleDelRight,"sudo ip rule del dev gre%d table 1",ID);
	sprintf(ipRuleDel,"sudo ip rule del from %s/32",IP);
	sprintf(ipRouteDel,"sudo ip route del default dev gre%d table %d",ID,ID);

	//get root permission
    int ruid,euid,suid;
    getresuid(&ruid,&euid,&suid);
    setresuid(ruid,suid,suid);

	system(ipTunnelDel);
	system(ifconfig);
	system(ipRuleDelRight);
	system(ipRuleDel);
	system(ipRouteDel);

	//return to user permission
   	getresuid(&ruid,&euid,&suid);
    setresuid(ruid,ruid,suid);

    free(ipTunnelDel);
    free(ifconfig);
    free(ipRuleDelRight);
    free(ipRuleDel);
    free(ipRouteDel);

    return 0;
}