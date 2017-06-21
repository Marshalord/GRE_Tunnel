#ifndef __IPEXCHANGE_H_
#define __IPEXCHANGE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int ChangeIPtoStr(unsigned int uiIP, char *strIP);
unsigned int ChangeStrToIP(char *strIP);
int CountIP();
#endif