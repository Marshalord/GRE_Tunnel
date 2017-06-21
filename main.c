#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mysqlController.h"
#include "ipExchange.h"
#include "socketController.h"
#include "greController.h"
#include <unistd.h>
#include <sys/types.h>

#define SIZE_OF_TUNNEL_NAME 40
#define SIZE_OF_SHELL 200

/*void thread(void){
	listenBySocket();
}*/
void initIPTable(){
	char *iptablesIn;
	char *iptablesOut;
	char *ipRuleAdd;
	char *ipRouteAdd;

	iptablesIn=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	iptablesOut=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipRuleAdd=(char *)malloc(SIZE_OF_SHELL*sizeof(char));
	ipRouteAdd=(char *)malloc(SIZE_OF_SHELL*sizeof(char));

	sprintf(iptablesIn, "sudo iptables -t mangle -A PREROUTING ! -i eno4 ! -p gre -j TEE --gateway 192.168.12.2");
	sprintf(iptablesOut,"sudo iptables -t mangle -A PREROUTING -i eno4 -j TEE --gateway 192.168.12.2");
	sprintf(ipRuleAdd,"sudo ip rule add from 10.10.1.253 table 1");
	sprintf(ipRouteAdd,"sudo ip route add default via 192.168.11.2 table 1");


	//get root permission
    int ruid,euid,suid;
    getresuid(&ruid,&euid,&suid);
    setresuid(ruid,suid,suid);

	system(iptablesIn);
	system(iptablesOut);
	system(ipRuleAdd);
	system(ipRouteAdd);

	//return to user permission
   	getresuid(&ruid,&euid,&suid);
    setresuid(ruid,ruid,suid);

    free(iptablesIn);
    free(iptablesOut);
    free(ipRuleAdd);
    free(ipRouteAdd);
}

/* usage :the obvious */
void usage (int exit_code)
{
  printf ("\
run from reboot:	sudo ./main -r \n\
only listening:		sudo ./main -l\n\
options:\n\
	-r		rebuild GRE tunnel from mysql\n\
	-l 		start listening\n\
	-h 		this cruft\n\
remember that you must run this program on su.\n");
  exit (exit_code);
}

int main(int argc,char** argv){
	//lower the permission to user level 
    int ruid,euid,suid;
    getresuid(&ruid,&euid,&suid);
    setresuid(ruid,euid,suid);

	int x;
	 /* if no args given at all*/
 	if (argc == 1)usage (1);	
     /* optarg, optind = next-argv-component [i.e. flag arg]; optopt = last-char */
	while ((x = getopt (argc, argv, "rlh")) != EOF)
    {
      switch (x)
		{
		case 'r':		/* rebuild GRE tunnel from mysql */
		  initIPTable();
		  rebuildGRE();
		  listenBySocket();
		  break;
		case 'l':		/* start listening */
		  listenBySocket();
		  break;
		case 'h':
		  usage (0);		/* exits by itself */
		default:
		  printf ("Try main -h for help\n");
		}			/* switch x */
    }				/* while getopt */
	
	/*pthread
	pthread_t id;
	int ret;
	ret=pthread_create(&id,NULL,(void *)thread,NULL);
	if (ret!=0)
	{
		printf("Create pthread error!\n");
		exit(1);
	}
	pthread_join(id,NULL);
	*/
	exit (0);
}