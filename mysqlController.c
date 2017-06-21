#include "mysqlController.h"
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <unistd.h>
#include <mysql.h>
#include "greController.h"
#include "ipExchange.h"

int FindIP(unsigned int id)
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

    //然后查询插入删除之后的数据
    sprintf(query_str,"select * from GRE_Tunnel_parameter where id = '%d'",id);
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
    return atoi(row[3]);
}
int FindByID(unsigned int id)
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

    //然后查询插入删除之后的数据
    sprintf(query_str,"select * from GRE_Tunnel_parameter where id = '%d'",id);
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
    //printf("The total rows is: %d\n", rows);
    if (rows==0)
    {
        mysql_free_result(res);
        mysql_close(&mysql);
        return 0;
    }
    mysql_free_result(res);
    mysql_close(&mysql);
    return rows;
}

int InsertParm(unsigned int id, char *tunnel_name, unsigned int remote_public_ip,unsigned int remote_tunnel_ip)
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
    sprintf(query_str,"insert into GRE_Tunnel_parameter (id,tunnel_name,remote_public_ip) values ('%d','%s','%d')",id,tunnel_name,remote_public_ip);
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        mysql_free_result(res);
        mysql_close(&mysql);
        return -1;
    }
    memset(query_str, 0, 200);
    
    mysql_free_result(res);
    mysql_close(&mysql);
    return 0;
}

int DeleteParm(unsigned int id){
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

    //执行删除请求
    sprintf(query_str,"delete from GRE_Tunnel_parameter where id='%d'",id);
    //printf("%s\n", query_str);
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        mysql_free_result(res);
        mysql_close(&mysql);
        return -1;
    }
  
    mysql_free_result(res);
    mysql_close(&mysql);
    return 0;
}

int rebuildGRE(){
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
       for(t=0; t<mysql_num_fields(res); t++)
        {
            if (t%3==1)
            {   id=atoi(row[t-1]);

                remoteIP=atoi(row[t]);
                unsigned char *bytes_old = (unsigned char *) &remoteIP;
                unsigned char strIP_old[15]; 
                sprintf (strIP_old, "%d.%d.%d.%d", bytes_old[3], bytes_old[2], bytes_old[1], bytes_old[0]);
                //printf("remoteIP:%s\n", strIP_old);

                //distribution REMOTE_TUNNEL_IP
                unsigned int remoteTunnelIP=atoi(row[t+1]);
                unsigned char *bytes_remoteTunnelIP = (unsigned char *) &remoteTunnelIP;
                unsigned char strIP_remoteTunnel[15]; 
                sprintf (strIP_remoteTunnel, "%d.%d.%d.%d", bytes_remoteTunnelIP[3], bytes_remoteTunnelIP[2], bytes_remoteTunnelIP[1], bytes_remoteTunnelIP[0]);

                greTunnelConfig(id,strIP_old,strIP_remoteTunnel);
                printf("%d,%s,%s\n", id,strIP_old,strIP_remoteTunnel);

            }
        }
     }
     
     
     mysql_close(&mysql);
}