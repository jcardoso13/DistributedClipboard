#include "clipboard.h"

/*******************************************************
 * Title: Distributed Clipboard
 * 
 * Date: 1/6/2018
 * 
 * Purpose: Project for System Programming
 * 
 * Authors:
 * 
 * 	- João Cardoso, nº84096 MEEC
 * 	- João Sebastião, nº84087 MEEC
 * 
 * 
 * 
*******************************************************/

/* Structure containing the IP address, port and the socket file descriptor of the/created by the local clipboard to be used by INET connections */
struct arg
{
	char* IP;
	char* port;
	int*   fd;
};

/* Structure containing the UNIX socket file descriptor and the socket information */
struct UNIXsocketStruck
{
	int fd;
	struct sockaddr_un addr;
}; // ad_un

/* Structure containing the INET socket file descriptor and the socket information */
struct INETsocketStruck
{
	int fd;
	struct sockaddr_in addr;
}; //ad_in


ad_un unix1; // unix struct

int main(int argc, char**argv)
{
	argument argz;
	
	
	pthread_t inet; // thread that starts INET_Loop
	pthread_t server; // thread that starts sync_connect
	
	//ad_un unix1;
	ad_in net;
	
	//EXPERIMENTAL
	char* IPbuffer;
	int connected=0;
	struct hostent *host_entry;
	char hostbuffer[256];
	
	gethostname(hostbuffer, sizeof(hostbuffer));
	host_entry=gethostbyname(hostbuffer);
	IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
	printf("Local IP:%s\n",IPbuffer);
	
	if(argc>4 || argc==2 || argc==3)
	{
		printf("Invalid number of arguments!\n");
		exit(-1);
	}


	/* Variables and functions regarding signal handling */
	pthread_t  sig_thr_id;
	sigset_t signal_mask;

	sigemptyset(&signal_mask);
	/* Block every signal */
	sigfillset(&signal_mask);
	/* Just block signal SIGINT (CTRL-C) */ 
    //sigaddset (&signal_mask, SIGINT);
	pthread_sigmask(SIG_SETMASK, &signal_mask, NULL);
	pthread_create(&sig_thr_id, NULL, signalHandling, (void *)&signal_mask);


	/* Create INET socket and UNIX socket*/
	net.addr = insocket(&net.fd); 

	unix1.addr=afsocket(&unix1.fd); 
	
	
	if (argc>1)
	{
		if (strcmp(argv[1],"-c")==0) //connected mode!
		{
			argz.fd=&net.fd;
			argz.IP=argv[2];
			connected=1;
			argz.port=argv[3];
			pthread_create(&server,NULL,sync_connect,(void*)&argz); //connects to the other clipboard
			
		}	
	}
	
	printf("Created INET socket and UNIX socket!\n");
	pthread_create(&inet,NULL,INET_loop,(void*)&net);  // waits for INET connections
	AF_loop(unix1,0); // waits for APP connections
	
	pthread_join(inet,NULL);
	pthread_join(sig_thr_id,NULL);
	
	if (connected==1) // if it started in connected mode, it needs sync_Connect to join
	{
		pthread_join(server,NULL);
	}
	printf("EXITING!\n");
	
	
	
	exit(0);	
}

void *signalHandling(void *arg) //Handles all Signals
{
	sigset_t *set = (sigset_t *)arg;
	int signal_caught;
	int error;
	
	while(1)
	{

		error = sigwait(set, &signal_caught);  //waits for all signals

		if(error != 0)
		{
			printf("\nError: %d", error);
			printf("\nSignal[%d]: %s\n", signal_caught, sys_siglist[signal_caught]);
		}

		if(signal_caught == SIGINT || signal_caught == SIGTERM)
		{
			set_time_over(); // to avoid race condition between freeConection and Remove conection!
			SetTop(-1);
			broadcast(10); //broacasts to all cond_Wait's
			freeConnectionList();
			AF_loop(unix1,1); //closes AF_Loop
			close(unix1.fd);
			memoryf(0,DELETE,NULL,NULL); //Free the memory
			break;
		}
	}
	return NULL;
}
