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
/*********************** STRUCTURES AND GLOBAL VARIABLES ***********************/

/* message struct, it has the strings parameters*/
struct messageStruct
{
	int region;
	long int size;
	int mode;
};

/* list of connected clipboards*/
struct connectList
{
	int fd;
	struct connectList *next;
};


void* buffer = NULL; /* string to be sent*/
void **Data = NULL; /* the 10 regions of the */
long int*Size=NULL; /* the 10 sizes of the regions*/
int written[10];
int changed=0;
int Top=-1; // the Top is the clipboard created in Single Mode
connectionList *head = NULL; /* head of the list*/

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER; /* Mutex that makes the threads wait for a change in memory to send that write to other clipboards*/
pthread_mutex_t mutex2=PTHREAD_MUTEX_INITIALIZER; /* Only 1 thread can read/write in memory*/
pthread_cond_t cond=PTHREAD_COND_INITIALIZER; /* the cond variable for mutex1, waits for a broadcast from memoryf to unlock*/
pthread_cond_t cond2=PTHREAD_COND_INITIALIZER; /* cond of clipboard_wait*/
pthread_mutex_t mutex_wait=PTHREAD_MUTEX_INITIALIZER; /* MUTEX of clipboard_wait*/
pthread_mutex_t mutex3=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond3=PTHREAD_COND_INITIALIZER;
int time_is_over2=0; // time to shutdown the clipboard
/********************************** FUNCTIONS **********************************/

/*
Function: memoryf
 
Functionality: This function is called by different threads to read or write into memory.
 
Arguments:
 
 int r- region to be read or write
 int m- mode- read/write/delete/sync write. the latter is just to write to memory without broadcasting to threads on "wait_for_friend"
 char* b- string to put into memory or to read the value into it.
 
 Return Values: returns the string
 * 
*/


void * memoryf(int r, int m, void *b, long int *size)
{
	int i=0;
	
	int region;
	int mode;
	void*buf; 
	void* aux;
	
	region=r;
	mode=m;
	buf=b;
	
	
	//mode: =0 read, =1 write, =2 delete, =3 sync write
	pthread_mutex_lock(&mutex2); //locks the function to other threads
	
	if(Data == NULL) // Memory allocation if it wasnt set!
	{
		Data = (void**)malloc(NUM_REGIONS*sizeof(void*));

		if( Data == NULL)
		{
			perror("Error in memory allocation! Function: memoryf");
			exit(-1);
		}
		
		Size=(long int*)malloc(NUM_REGIONS*sizeof(long int));
		
		if( Size == NULL)
		{
			perror("Error in memory allocation! Function: memoryf");
			exit(-1);
		}
		

		for (i = 0; i < NUM_REGIONS; i++)
		{
			Data[i] =(void*)malloc(CHAR_LIMIT);
			if( Data[i] == NULL)
			{	
				perror("Error in memory allocation! Function: memoryf");
				exit(-1);
			}
			Size[i]=CHAR_LIMIT;
			written[i]=0;
			memset(Data[i], 0, CHAR_LIMIT);
		}
	}
	
	
	if (mode==DELETE) // Free the data
	{
		for(i=0;i<NUM_REGIONS;i++)
		{
			free(Data[i]);
		}
		free(Size);
		free(Data);
		
		Data=NULL;
		Size=NULL;

	}
	
	if (mode==WRITE || mode==WRITE_SYNC) // write data
	{	
		

		aux = Data[region];
		Data[region]=data_check(*size,&Size[region],aux);
		
		
		memcpy(Data[region], buf,*size);

		printf("Data SET: %d! Size:%li\n", region,Size[region]	);
		written[region]=1;
		
		buffer=Data[region];		
		if (mode==WRITE)
		{
			setflag(region,1); // BROADCAST!
			buffer=Data[region];
			broadcast(region);
		}
		if (mode==WRITE_SYNC)
		{
		pthread_cond_broadcast(&cond2);
		changed=region;
		}
	
	}
	if (mode==READ) //read data
	{
		b=data_check(Size[region],size,b);
		memcpy(b,Data[region],Size[region]);
		*size=Size[region];
	}
	
	pthread_mutex_unlock(&mutex2); //unlocks the function to another thread waiting
	
	return b;
}



/*  BROADCAST
 * 
 * function used to unblock several pthread_cond_wait
 * 
 * 
 * */

void broadcast(int region)
{
	pthread_cond_broadcast(&cond); // unblocks wait_for_friend 
	changed=region;
	pthread_cond_broadcast(&cond2); //unblocks clipboard_wait if the region=changed
	
	if (changed==10)
		pthread_cond_broadcast(&cond3);
}

/*  BLOCK
 * 
 * function used to block OperationWait, that is responsable for
 * implementing the clipboard side for clipboard_wait
*/
void block(int region)
{
	do
	{
	pthread_mutex_lock(&mutex_wait);
	pthread_cond_wait(&cond2,&mutex_wait);
	pthread_mutex_unlock(&mutex_wait);
	}while (changed!=region && changed!=10);
}

/*
Function: wait_for_friend
 
Functionality: This function waits for a change in memory to send to other clipboards
 
Arguments: identifier of the clipboard that is connected
 
 Return Values: None
*/

void* wait_for_friend(void* fd)
{
	int* fd2=(int*)fd;
	int local_fd=*fd2;
	int region;
	message m;
	int aux2;
	long int size=0;
	void* aux=NULL;
	int flag=1;
	memset(&m,0,sizeof(message));
	
	while(Top!=-1 && local_fd!=Top)
	{
		pthread_mutex_lock(&mutex3);
		pthread_cond_wait(&cond3,&mutex3);
		pthread_mutex_unlock(&mutex3);
	}
	
	if (local_fd==Top || Top==-1)
	{
	while(flag>=0)
	{
		flag=0;
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond,&mutex); //waits for a broadcast from memoryf
		if(time_is_over2==0)
		{
			region=setflag(0,0); //get which region to send
			m.region=region;
			aux=memoryf(region,READ,aux,&size);
			if (Size!=NULL)
			m.size=Size[region];
			else break;
		
			m.mode=WRITE_SYNC;
			flag=0;
			flag=send(local_fd,&m,sizeof(message),0);
			if (flag<=0)
			{
				//leave Wait for friend
				break;
			}
			flag=0;
			while (flag<m.size)
			{
				aux2=send(local_fd,aux+flag,m.size-flag,0);
				flag+=aux2;
				if (aux2<=0)
				{
					break;
				}
			}
			pthread_mutex_unlock(&mutex);
		}
		else break;
	}
	pthread_mutex_unlock(&mutex);
	
	}
	free(aux);
	return NULL;
}
/* Sets a variable to true, meaning that the clipboard is exiting*/

void set_time_over(void)
{
	time_is_over2=1;
}

/*
Function: wait_for_sync
 
Functionality: This function waits for a message from other clipboards and writes the changes.
 
Arguments: identifier of the clipboard that is connected
 
 Return Values: None
*/
void *wait_for_sync(void* fd)
{
	message *m =malloc(sizeof(message));
	connectionList *aux = gethead();
	long int size;
	void *text = NULL;
	long int data_size=CHAR_LIMIT;
	int * fd2=(int*)fd;
	int local_fd=*fd2;
	int flag=0;
	int aux2;
	void* ptr=NULL;
	
	text = (void *)malloc(CHAR_LIMIT);
	if (text==NULL)
		{
		perror("Memory alloc-Memoryf\n");
		exit(-1);
		}

	aux=aux->next;

	while(1)
	{
		m->size=0;
		m->region=0;
		m->mode=DATA_IS_NULL;
		if(recv( local_fd, m, sizeof(message),0) <= 0)
		{
			if (time_is_over2==0) // to avoid trying to remove twice
			removeConnection(local_fd);
			if (local_fd==Top)
				Top=-1;
			free(m);
			free(text);

			return NULL;
		}


		size = m->size;
		text = data_check(size,&data_size,text);
		flag=0;
		while(flag<size)
		{
			ptr=text+flag;
			aux2 = recv(local_fd,ptr, size-flag,0);
			flag+=aux2;
			if(aux2<= 0)
			{
				if (time_is_over2==0)  // to avoid trying to remove twice
				removeConnection(local_fd);
				free(m);
				free(text);
				return NULL;
			}
		}
		
		
		
		if (time_is_over2==0)
		{
		aux=gethead();
		aux=aux->next;
		}
		else {
				free(m);
				free(text);
				return NULL;
			}
			
			
		if (Top!=-1 && local_fd!=Top) //im not the head && ive not received orders from Top
		{
		send(Top,m,sizeof(message),0);
		send(Top,text,size,0);
		}
		else while(aux != NULL) // I'm the head!
		{
			if(aux->fd == local_fd && Top!=-1)
			{
				aux = aux->next;
			}
			else
			{
				if(send(aux->fd, m, sizeof(message), 0) == -1)
				{
					if (time_is_over2==0)
						removeConnection(local_fd);
						free(m);
						free(text);
						return NULL;
				}

				if(send(aux->fd, text, size, 0) == -1)
				{
					if (time_is_over2==0)
						removeConnection(local_fd);
						free(m);
						free(text);
				return NULL;
				}

				aux = aux->next;
			}
		}
		if(local_fd==Top || Top==-1)
		memoryf(m->region,WRITE_SYNC,text, &m->size); // write to memory
	}

	free(m);
	free(text);
	return NULL;
	
}
	

/*
Function: setflag
 
Functionality: This function writes the region which to send, if f=0, just returns the region, if f=1, it writes the region

 
Arguments: region and flag
 
 Return Values: None
*/
int setflag(int r,int f)
{
	static int region=0;
	
	if (f==1)
	{
		region=r;
	}
	
	return region;
}


/*
Function: sync_my_data
 
Functionality: This function syncs data with the other clipboard that connects to this one
 
Arguments: identifier of the clipboard that is connected
 
 Return Values: None
*/


void sync_my_data(int fd)
{
	int i=0;
	message m;
	long int flag=0;
	long int buffer=0;
	memset(&m,0,sizeof(message));
	m.region=i;
	m.mode=DATA_SEND;
	m.size=0;

	for(i=0;i<DATA_POSITIONS;i++)
	{
		if (Data!=NULL) //if data was initiated
		{ 
			if (written[i]==1) // and actually has a value
			{
				buffer=0;
				m.region=i;
				m.mode=DATA_SEND;
				m.size=Size[i];
				flag=send(fd,&m,sizeof(message),0);
				if (flag<=0)
				{
					perror("data not sent-sync_my_data");
					return;
				}
				while (buffer<m.size)
				{
					flag=send(fd,Data[i],m.size,0);
					buffer+=flag;
					if (flag<=0)
					{
						perror("data not sent-sync_my_data");
						return;
					}
				}	
			}
		}
		else
		{
			m.mode=DATA_IS_NULL;
			send(fd,&m,sizeof(message),0);
			break;
		}
					
	}
	
	if (m.mode!=DATA_IS_NULL)
	{
		m.mode=DATA_IS_NULL;
		send(fd,&m,sizeof(message),0);
	}
}

/*
Function: receive_friend_data
 
Functionality: This function waits receives the clipboard im connecting to
* 
* 
Arguments: identifier of the clipboard that is connected
 
 Return Values: None
*/

void receive_friend_data (int fd)
{
	
	int i=0;
	message m;
	long int flag=0;
	long int aux2=0;
	m.mode=0;
	char*buf=NULL;
	long int data_size=0;

	while(i==0)
	{
		recv(fd,&m,sizeof(message),0);
		if (m.mode==DATA_SEND)
		{
			flag=0;
			buf=data_check(m.size,&data_size,buf);
			while(flag<m.size)
			{
					aux2 = recv(fd,buf+flag,m.size-flag,0);
					flag+=aux2;
					if(aux2<= 0)
					{	
					
						break;
					}
			}
			memoryf(m.region, WRITE, buf, &m.size);
		}
		

		if (m.mode==DATA_IS_NULL)
		{
			break;
		}
	}
	
	free(buf);
}


/*
Function: data_check
 
Functionality: This function sees if theres a need for allocation of more memory
 
Arguments: 
* size- minimum size
*  data size- size of buf
* buf- future holder of data
 
 Return Values: None
*/

void* data_check(long int size,long int *data_size, void* buf)
{


	if(buf==NULL)
	{
		buf=(void*)malloc(size);
		if (buf==NULL)
		{
			perror("Data allocation went wrong- data_check");
			exit(-1);
		}
		*data_size=size;
	}
	else if(*data_size>size+CHAR_LIMIT)
	{
		buf=(void*)realloc(buf,size+CHAR_LIMIT);
		if (buf==NULL)
		{
			perror("Data allocation went wrong- data_check");
			exit(-1);
		}
		*data_size=size+CHAR_LIMIT;
	}
	else if(*data_size<size)
	{
		buf=(void*)realloc(buf,size);
		if (buf==NULL)
		{
			perror("Data allocation went wrong- data_check");
			exit(-1);
		}
		*data_size=size;
	}
	
	
	memset(buf,0,*data_size);
	
	return buf;
	
}

/* Sets the head of this clipboard as Top 
 * 
 * the clipboard that has Top=-1 is a clipboard in single mode
 * */

void SetTop(int fd)
{
	
Top=fd;	
pthread_cond_broadcast(&cond3);
return;
}



int GetTop(void)
{
	return Top;
}
