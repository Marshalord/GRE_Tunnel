#ifndef __MYSQLCONTROLLER_H_
#define __MYSQLCONTROLLER_H_
#include <stdio.h>
int FindIP(unsigned int id);
int FindByID(unsigned int id);
int InsertParm(unsigned int id, char *tunnel_name, unsigned int remote_public_ip,unsigned int remote_tunnel_ip);
int DeleteParm(unsigned int id);
int rebuildGRE();
#endif