#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql.h>
int ChangeIPtoStr(unsigned int uiIP, char *strIP)  
{  
  unsigned char *bytes = (unsigned char *) &uiIP;  
  sprintf (strIP, "%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
  printf("changeIPtoStr:%u.%u.%u.%u\n", bytes[3], bytes[2], bytes[1], bytes[0]);
  return 0;  
}  

unsigned int ChangeStrToIP(char *strIP)
{
    int a = 0, b = 0, c = 0, d = 0;
    unsigned int uiIP = 0;
    if (strIP == NULL || strlen(strIP) == 0)
        return 0;

    if (sscanf(strIP, "%d.%d.%d.%d", &a, &b, &c, &d ) == EOF)
        return 0;

    if (a > 0 && a <= 255 && b >= 0 && b <= 255 && c >= 0 && c <= 255 && d >= 0 && d <= 255)
        uiIP = (a << 24) + (b << 16) + (c << 8) + d;
    return uiIP;
}
int CountIP()
{
    MYSQL           mysql;
    MYSQL_RES       *res = NULL;
    MYSQL_ROW       row;
    char            query_str[200];
    int             rc, i, fields;
    int             rows;

    if (NULL == mysql_init(&mysql)) {
        printf("mysql_init(): %s\n", mysql_error(&mysql));
        mysql_free_result(res);
        mysql_close(&mysql);
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
    //执行插入请求
    sprintf(query_str,"select count(*) from GRE_Tunnel_parameter");
    printf("%s\n", query_str);
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        mysql_free_result(res);
        mysql_close(&mysql);
        return -1;
    }
    res = mysql_store_result(&mysql);
    if (NULL == res) {
         printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
        mysql_free_result(res);
        mysql_close(&mysql);
         return -1;
    }
    rows = mysql_num_rows(res);
    if (rows==0)
    {
        mysql_free_result(res);
        mysql_close(&mysql);
        return 0;
    }
    fields = mysql_num_fields(res);
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    mysql_close(&mysql);
    return atoi(row[0]);
}

int isLocalIP(unsigned int uiCheckIP)
{
    unsigned int uiBegin;
    unsigned int uiEnd;

    uiBegin = 0x0a000000;//A:10.0.0.0
    uiEnd   = 0x0affffff;//A:10.255.255.255
    
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