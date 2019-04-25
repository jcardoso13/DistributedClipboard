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

// INET SOCKET Functions 

//It holds all the FUnctions associated with INET Sockets!




/*********************** STRUCTURES AND GLOBAL VARIABLES ***********************/

/* Structure containing the IP address, port and the socket file descriptor of the/created by the local clipboard to be used by INET connections */
struct arg
{
	char* IP;
	char* port;
	int*   fd;
};


/* Structure containing the INET socket file descriptor and the socket information */
struct INETsocketStruck
{
	int fd;
	struct sockaddr_in addr;
};


/* Pointer to the head of the connected sockets' list */
connectionList *headSocket = NULL;


/********************************** FUNCTIONS **********************************/

/*
Function: insocket
 
Functionality: creates an INET type socket, creates a list of INET sockets connected to it, assigns an address to it and listens to incoming connections
 
Arguments:
 
 • b – void pointer to a structure that contains the file descriptor of the INET socket and the address
 
 Return Values: NULL;
*/
struct sockaddr_in insocket(void* b)
{
	
	
	int a;
	int i=0;
	srand(time(NULL));
	int suc;

	//socklen_t addr_size=sizeof(addr);
	ad_in *net=(ad_in*)b;

	/* Function to create the INET socket */
	net->fd=socket(AF_INET,SOCK_STREAM,0);

	/* Verify successful creation */
	if(net->fd < 0)
	{
		printf("Unable to create the INET socket! Location: AF_INET.c - insocket\n");
		exit(-1);
	}
	
	/* Function to create the list of connected INET sockets */
	headSocket = createConnectionList(net->fd);


	/* Clear memory for the address */
	memset(&net->addr,0,sizeof(net->addr));

	/* Define address information */
	net->addr.sin_family = AF_INET;
	net->addr.sin_port=htons(a);
	net->addr.sin_addr.s_addr=INADDR_ANY;
	
	
	suc=-1;
	/* Assign an address to the socket */
	while (suc==-1)
	{
		a = rand()%101;
		a=a+8000;
		net->addr.sin_port=htons(a);
		suc = bind (net->fd, (struct sockaddr*)&(net->addr), sizeof(struct sockaddr));
		i++;
		if (i>10)
			break;
	}
	
	printf("My Port:%d\n",a);
	/* Verify successful bind */
	if(suc == -1)
	{
		perror("Unable to bind an address to the socket! Location: AF_INET.c - insocket\n");
		exit(-1);
	}

	/* Listen to incoming connections */
	suc = listen(net->fd, MAX_CALLS);

	/* Verify successful listen */
	if(suc == -1)
	{
		perror("Unable to start listening to connections! Location: AF_INET.c - insocket\n");
		exit(-1);
	}

	/*  */
	b=net;

	return net->addr;
}


/*
Function: INET_loop
 
Functionality: loop that waits for incoming connections and 2 threads per connection to exchange information between clipboards

 
Arguments:
 
 • data – void pointer to a structure that contains the file descriptor of the INET socket and the address
 
 Return Values: NULL;
*/
void* INET_loop(void* data)
{
	pthread_t *sync;
	pthread_t *r;
	int new_fd=0;
	int i=0;
	int clip_clients=0;
	int c=0;
	
	sync=(pthread_t*)malloc(DATA_POSITIONS*sizeof(pthread_t));
	if (sync==NULL)
		{
			perror("memory alloc-INET_LOOP\n");
			exit(-1);
		}
	r=(pthread_t*)malloc(DATA_POSITIONS*sizeof(pthread_t));
	if (r==NULL)
		{
			perror("memory alloc-INET_LOOP\n");
			exit(-1);
		}
	c=DATA_POSITIONS;
	
	while(1) //Infinite INET_Loop
	{
		new_fd = waiting_INET(data);
		if (new_fd==0) // if new_fd==0, it means clipboard needs to shutdown 
			break;
		printf("Clipboard #%d Connected to me\n",clip_clients);
		sync_my_data(new_fd);
		pthread_create(&sync[clip_clients],NULL,wait_for_sync,(void*)&new_fd); // waits for data from new_fd
		pthread_create(&r[clip_clients],NULL,wait_for_friend,(void*)&new_fd); // waits for data from local_apps to send to the Top
		clip_clients++;
			if (clip_clients==c) // Aloc more threads
			{
				sync=(pthread_t*)realloc(sync,(c+DATA_POSITIONS)*sizeof(pthread_t));
				if (sync==NULL)
					{
						perror("memory alloc-INET_LOOP\n");
						exit(-1);
					}
				r=(pthread_t*)realloc(r,(c+DATA_POSITIONS)*sizeof(pthread_t));
				if (r==NULL)
					{
						perror("memory alloc-INET_LOOP\n");
						exit(-1);
					}
				c+=DATA_POSITIONS;
			}
	
	}
	//Closing closing INET_loop
	if (clip_clients!=0)
		{
			for (i=0;i<clip_clients;i++)
			{
			pthread_join(r[i],NULL);
			pthread_join(sync[i],NULL);
			}
		}
		
	free(sync);
	free(r);
	return NULL;
}



/*
Function: waiting_INET
 
Functionality: function that accepts a connection from an INET type socket and adds the new connection to the list
 
Arguments:
 
 • data – void pointer to a structure that contains the file descriptor of the INET socket and the address
 
 Return Values: returns the file descriptor of the newly established connection
*/
int waiting_INET(void* data) 
{
	ad_in* net=data;
	socklen_t addr_size= sizeof(struct sockaddr_in);
	int new_fd = 0;
	
	/* Establish connection to the local socket */
	while (new_fd==0)
	{
		new_fd = accept(net->fd, (struct sockaddr*)&(net->addr), &addr_size);
	}
	/* Verify successful connection */
	if((new_fd)==-1) // break the cycle and waits to end the INET conections
	{
		return 0;
	}

	printf("\nAccepted new connection!\n");

	/* Add new connection to the list */
	headSocket=addNewConnection(new_fd,headSocket);
	
	return new_fd;
}


/*
Function: sync_connect
 
Functionality: function that connects to another clipboard through the input "-c IP Port Number"
* 
* If successfull it ads the fd to the Fd List
Arguments:
 
 • data – void pointer to a structure that contains the file descriptor of the INET socket and the address
 
 Return Values: NULL
*/
void* sync_connect(void* argv)
{
	struct sockaddr_in addr;
	int port;
	int suc=1;
	argument* argv2;
	pthread_t re; //to receive
	argv2=(argument*)argv;
	int fd=0;
	
	/* Clear memory for the address */
	memset(&addr,0,sizeof(addr));

	/* Define address of the socket */
	addr.sin_family=AF_INET;
	port=atoi(argv2->port);
	addr.sin_port=htons(port);
	inet_aton(argv2->IP,&addr.sin_addr);

	printf("IP address:%s\n",argv2->IP);
	printf("Port:%d\n", port);

	/* Create the INET socket */
	fd = socket(AF_INET,SOCK_STREAM,0);

	while(suc!=0)
	{
		/* Connect to the INET socket */
		suc = connect(fd,(const struct sockaddr*)&addr,sizeof(addr));
		if (suc==-1)
		{
			perror("cant connect to other clipboard -SYNC CONNECT \n");
			return NULL;
		}
		addNewConnection(fd,NULL);
		SetTop(fd);
		receive_friend_data(fd);
		pthread_create(&re,NULL,wait_for_friend,(void*)&fd);
		wait_for_sync(&fd);
	}
	
	
	pthread_join(re,NULL);

	return NULL;
}

/*
Function: gethead
 
Functionality: function that returns the head of the list
 
Arguments: N/A
 
 Return Values: pointer to the head of the list of fd's
*/
connectionList* gethead(void)
{
	return headSocket;
}




