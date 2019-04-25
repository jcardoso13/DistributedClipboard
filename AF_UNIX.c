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

// this has AF_UNIX Socket functions

/*********************** STRUCTURES AND GLOBAL VARIABLES ***********************/

/* Structure containing the host's socket file descriptor, the file descriptor of the socket created and the "ID" of the client */
struct socketStruct
{
 int fd;
 int host_fd;
 int client_number;
};

/* message struct, it has the Data parameters*/
struct messageStruct
{
	int region;
	long int size;
	int mode;
};

/* Structure containing the UNIX socket file descriptor and the socket information */
struct UNIXsocketStruck
{
	int fd;
	struct sockaddr_un addr;
};

int time_is_over=0;
int* fd_num=NULL;
int fd2=0;

/********************************** FUNCTIONS **********************************/


/*
Function: afsocket
 
Functionality: creates a UNIX type socket

Arguments:
 
 • a – void pointer to the UNIX socket's file descriptor
 
 Return Values: address of the UNIX socket;
*/
struct sockaddr_un afsocket(int* a)
{
	int my_fd; 
	struct sockaddr_un addr;
	int suc;

	
	/* Function to create the UNIX socket */
	my_fd = socket(AF_UNIX,SOCK_STREAM,0);
	
	/* Verify successful creation */
	if(my_fd < 0)
	{
		perror("Unable to create the UNIX socket! Location: AF_UNIX.c - afsocket\n");
		exit(-1);
	}

	/* Clear memory for the address */
	memset(&addr,0,sizeof(addr));

	/* Define address information */
	addr.sun_family = AF_UNIX;
	memcpy(addr.sun_path, SOCK_PATH, strlen(SOCK_PATH));

	unlink(SOCK_PATH);

	
 	/* Assign an address to the socket */
	suc = bind (my_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
		//suc=bind(my_fd,(struct sockaddr*)&addr,addrlen);
	
	/* Verify successful bind */
	if(suc < 0)
	{
		unlink(addr.sun_path);
		perror("Bind unsuccessful\n");
		unlink(SOCK_PATH);
		close(my_fd);
		exit(1);
	}

	/* Listen to incoming connections */
	suc = listen(my_fd, MAX_CALLS);

	if(suc == -1)
	{
		perror("Unable to start listening to connections! Location: AF_UNIX.c - afsocket\n");
		exit(-1);
	}

	printf("Local clipboard: Ready to be used in applications!\n");
	
	*a=my_fd;
	
	return addr;	
}


/*
Function: AF_loop
 
Functionality: loop that waits for incoming connections and creates 1 thread per connection to exchange information between clipboard and application
 
Arguments:
 
 • unix1 – structure that contains the file descriptor and the address of the UNIX socket 
 
 Return Values: NULL;
*/
void AF_loop(ad_un unix1,int flag)
{
	
	sck par;
	int i=0;
	static pthread_t wa;
	int client_counter = 0;
	int number=0;
	
	
	pthread_t *c;
	

	
	if (flag==1) // Signal Handler comes into AF_Loop and cancels the "accept" call to get out of the Loop
		{
		time_is_over=1;
		pthread_cancel(wa);
		return;
		}

	fd_num=(int*)malloc(DATA_POSITIONS*sizeof(int));
	if (fd_num==NULL)
				{
					perror("Memory ALocation fail in AF_UNIX Loop");
					exit(-1);
				}
	c=(pthread_t*)malloc(DATA_POSITIONS*sizeof(pthread_t));
	if (c==NULL)
				{
					perror("Memory ALocation fail in AF_UNIX Loop");
					exit(-1);
				}
	number=DATA_POSITIONS;
	memset(fd_num,0,DATA_POSITIONS*sizeof(int));
	memset(&par,0,sizeof(sck));
	
	printf("Waiting for UNIX connections!\n");

	while(1)
	{
		par.host_fd = unix1.fd;
		//par.fd =waiting_UNIX(unix1);
		pthread_create(&wa,NULL,waiting_UNIX,(void*)&unix1);
		pthread_join(wa,NULL);
		
		if(time_is_over==0)
		{
			par.fd=fd2;
		}
		if (time_is_over==1)
		{
			for (i=0;i<client_counter;i++)
				{
					if (fd_num[i]!=0)
						shutdown(fd_num[i],SHUT_RDWR); 
					fd_num[i]=0;
				}
			break;
		}
		if (client_counter<number)
		fd_num[client_counter]=par.fd;
		else // Aloc more threads for Local Apps!
		{
			fd_num=(int*)realloc(fd_num,(DATA_POSITIONS+number)*sizeof(int));
				if (fd_num==NULL)
				{
					perror("Memory ALocation fail in AF_UNIX Loop");
					exit(-1);
				}
			c=(pthread_t*)realloc(c,(DATA_POSITIONS+number)*sizeof(pthread_t));	
				if (c==NULL)
				{
					perror("Memory ALocation fail in AF_UNIX Loop");
					exit(-1);
				}
			memset(fd_num+number,0,DATA_POSITIONS*sizeof(int));
			number+=DATA_POSITIONS;
			fd_num[client_counter]=par.fd;
		}
		printf("Local clipboard: new application, application number #%d!\n", client_counter);
		par.client_number = client_counter;
		pthread_create(&c[client_counter], NULL, Communicating_With_friends, (void*)&par);
		
		client_counter+=1;
	}	
	
	if (time_is_over==1) // if its time to exit- waits for pthreads data
	{
		if (client_counter>0)
		for (i=0;i<client_counter;i++)
		{
			pthread_join(c[i],NULL);
		}
		free(fd_num);
		free(c);
	}
}

/*
Function: waiting_UNIX
 
Functionality: function that accepts a connection from an AF_UNIX type socket and adds the new connection to the list
 
Arguments:
 
 • unix1 – structure that contains the file descriptor and the address of the UNIX socket
 
 Return Values: returns the file descriptor of the newly established connection
*/
void* waiting_UNIX(void* a) // 
{
	struct sockaddr_un addr;
	
	ad_un*un=(ad_un*)a;
	ad_un unix1=*un;
	
	addr=unix1.addr;
	socklen_t addr_size = sizeof(struct sockaddr_un);
	int new_fd = -1;
		
	/* Wait for a new connection */
	new_fd = accept(unix1.fd, (struct sockaddr*)&(addr),&addr_size);
	
	
	/* Verify successful connection */ 
	if(new_fd==-1)
	{
		//perror("Unable to accept new connection! Location: AF_UNIX.c - waiting_UNIX\n");
		perror("Accept failed- Closing AF_UNIX\n");
		return NULL;
	}
	
	fd2=new_fd;
	return NULL;
}

/*
Function: Communicating_With_friends
 
Functionality: communication with the application
 
Arguments:
 
 • a – structure that contains the file descriptor of the socket, the file descriptor of the host socket and the client number
 
 Return Values: NULL
*/
void* Communicating_With_friends(void* a)
{
	int new_fd;
	int client=0;
	int buffer = 0;
	void* aux = NULL; // Where our temporary data is stored
	long int size=0;
	sck* ptr = (sck*) a;

	message* m = (message*)malloc(sizeof(message));
	if (m==NULL)
		{
			perror("Memory Alocation fail in Comunicating with App_Local");
			exit(-1);
		}
	memset(m,0,sizeof(message));


	client = ptr->client_number;
	new_fd = ptr->fd;
	
	while(1)
	{
		m->mode=0;
		m->size=0;
		m->region=0;

		/* Check if the connection was lost */
		if(((recv(new_fd,m, sizeof(message),0))<=0 )|| (time_is_over==1))
		{
			free(m);
			if (aux!=NULL)
				free(aux);
			if (fd_num!=NULL)
			if (fd_num[client]!=0)
				{
					close(new_fd);
					fd_num[client]=0;
				}
			printf("Local application number #%d has disconnected from the clipboard!\n", client);
			return NULL;
		}

		while(m->mode != 0)
		{
			
			/* User wants to save information */
			if(m->mode == 1) //Paste		
			{
				aux=operationPaste(m,aux, new_fd, client,&size);
			}
			else if (m->mode ==10 ) //Copy - User wants to write information 
			{	
				aux=operationCopy(m, aux, new_fd, client, buffer,&size);
			}
			else if (m->mode==WAIT_WRITE)
			{
				aux=operationWait(m,aux,new_fd,client,&size);
			}
			m->mode=0;
		}
	}

	/* Free allocated memory */
	free(m);
	if (aux!=NULL)
		free(aux);
	

	return NULL;
}

/*Function: OperationPaste
 
Functionality: Getting the Data requested from App_Local and Sends it
 
Arguments:
 
 • m– message struct
 * aux- Data to be sent
 * new_fd - file descriptor of the Local_App
 * size- Size of aux
 
 Return Values: Aux
*/

void* operationPaste(message *m, void*aux, int new_fd, int client, long int *size)
{
	int c;
	int buffer=0;
	/* Read information from memory */
	aux=(void*)memoryf(m->region,READ,aux,size);

		


	/* Send the message struct */
	m->size=*size;
	if((send(new_fd,m, sizeof(message),0))==-1)
	{
		perror("send"); 
		printf("Local application number #%d has disconnected from the clipboard!\n", client);
		return aux;
	}

	/* Send the information */
	buffer=0;
	
	if (*size>=m->size)
		{
			while (buffer<m->size)
				{
					c=send(new_fd,aux+buffer,m->size-buffer,0);
						if(c==-1)
							{
								perror("send"); 
								printf("Local application number #%d has disconnected from the clipboard!\n", client);
								return aux;
							}
						buffer+=c;
				}
		}
		else while (buffer<*size)
			{
				c=send(new_fd,aux+buffer,*size-buffer,0);
					if(c==-1)
						{
							perror("send"); 
							printf("Local application number #%d has disconnected from the clipboard!\n", client);
							return aux;
						}
					buffer+=c;
			}
	
		
		
		return aux;
}

/*Function: OperationCopy
 
Functionality: Getting the Data from from App_Local and storing it.
 
Arguments:
 
 • m– message struct
 * aux- Data received
 * new_fd - file descriptor of the Local_App
 * size- Size of aux
 * buffer- to be sure that every data is received
 
 Return Values: Aux
*/


void* operationCopy(message *m, void*aux, int new_fd, int client, int buffer,long int *size)
{
	int c;
	/* Check if there is enough memory to store the information retrieved */
	if (m->size<0 || m->region>9 || m->region<0)
		return 0;
	
	if(aux == NULL)
	{
		aux = (void*)realloc(aux,m->size);
		*size=m->size;
		memset(aux, 0, m->size);
	}
	if (*size<m->size)
	{
		aux=realloc(aux,m->size);
		*size=m->size;
	}

	/* Receive message */
	buffer=0;
	while (buffer<m->size)
	{
		c=recv(new_fd,aux+buffer,m->size-buffer,0);
		if(c==-1)
				{
					perror("recv-Local application number has disconnected from the clipboard"); 
					return aux;
				}
		buffer+=c;
	}
	/* Store the information in memory */
	memoryf(m->region,WRITE,aux,  size);

	/* Notification of successful write */
	send(new_fd,m,sizeof(message),0); //success
	
	return aux;
}

/*Function: OperationWait
 
Functionality: Waiting for an alteration on a certain region and then 
* Getting the Data requested from App_Local and Sending it 
* 
* Very similar to OperationPaste with the inclusion of block() which 
* is a function that blocks till the region is written on
 
Arguments:
 
 • m– message struct
 * aux- Data to be sent
 * new_fd - file descriptor of the Local_App
 * size- Size of aux
 
 Return Values: Aux
*/


void* operationWait(message *m, void*aux, int new_fd, int client, long int* size)
{
	int c;
	int buffer=0;
	/* Read information from memory */
	block(m->region);
	if (time_is_over==0)
	{
		aux=(void*)memoryf(m->region,READ,aux, size);



		m->size=*size;
		/* Send the message struct */
		if((send(new_fd,m, sizeof(message),0))==-1)
		{
			perror("send"); 
			printf("Local application number #%d has disconnected from the clipboard!\n", client);
			return aux;
		}

		/* Send the information */
		buffer=0;
		if (*size>=m->size)
			{
				while (buffer<m->size)
					{
						c=send(new_fd,aux+buffer,m->size-buffer,0);
							if(c==-1)
								{
									perror("send"); 
									printf("Local application number #%d has disconnected from the clipboard!\n", client);
									return aux;
								}
							buffer+=c;
					}
			}
		else while (buffer<*size)
			{
				c=send(new_fd,aux+buffer,*size-buffer,0);
					if(c==-1)
						{
							perror("send"); 
							printf("Local application number #%d has disconnected from the clipboard!\n", client);
							return aux;
						}
					buffer+=c;
			}
			
	}
		return aux;
}

