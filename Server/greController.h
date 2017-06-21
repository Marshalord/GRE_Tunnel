#ifndef __GRECONTROLLER_H_
#define __GRECONTROLLER_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
int greTunnelConfig(unsigned int ID,char *IP,char *remoteTunnelIP);
int greTunnelDel(unsigned int ID,char *IP);
#endif