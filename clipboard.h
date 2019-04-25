
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

#define MAXDATASIZE 1000
#define MYPORT "1337"
#define MAX_CALLS 100
#define SOCK_PATH "./socket2"
#define CHAR_LIMIT 100
#define DATA_POSITIONS 10
#define DATA_IS_NULL  5
#define DATA_SEND	4
#define READ		0
#define WRITE		1
#define WRITE_SYNC	3
#define DELETE		2
#define NUM_REGIONS	10
#define WAIT_WRITE  6


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>

/* Structures  */
typedef struct messageStruct message;
typedef struct socketStruct sck;
typedef struct arg argument;
typedef struct INETsocketStruck ad_in;
typedef struct UNIXsocketStruck ad_un;
typedef struct connectList connectionList;


int Check(char*aux);
/* Functions in Library */
int clipboard_connect(char * clipboard_dir);
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count);
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count);
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count);
void clipboard_close(int clipboard_id);

/* Functions related to the UNIX sockets */
struct sockaddr_un afsocket(int *);
void AF_loop(ad_un unix1,int flag);
void* waiting_UNIX(void* a);
void* Communicating_With_friends(void* a);
void* operationCopy(message* m, void* aux, int new_fd, int client, int buffer,long int *size);
void* operationPaste(message* m, void* aux, int new_fd, int client,long int* size);
void* operationWait(message *m, void*aux, int new_fd, int client,long int* size);

/* Functions related to the INET sockets */
struct sockaddr_in insocket(void* a);
void* INET_loop(void* fd);
void* wait_for_sync(void* fd);
int waiting_INET(void* fd);


/*Funtions related to memory management and syncronization*/
void* sync_connect(void* argv); // connects to another clipboard
void* wait_for_friend(void* fd); // waits for something to be written in memory
void* memoryf(int r, int m, void* b, long int* size); // Function that handles memory
void set_time_over(void); // informs sync.c apps that clipboard needs to close
void broadcast(int region); // broadcasts to cond_wait
void block(int region); // blocks until a broadcast
int setflag(int r, int f); // sets up the region to replicate
void sync_my_data(int fd); // sends data to the clipboard that connected to me
void receive_friend_data (int fd); // receives data that the clipboard that i was connected to gave
void* data_check(long int size, long int *data_size,void* buf); // checks data size compared to size and reallocs


/* Functions related to the list of connected clipboards */ 
connectionList* createConnectionList(int file_descriptor); //creates fd list
void* addNewConnection(int file_descriptor,connectionList*head); //creates a new connection
void freeConnectionList(); //frees and shutdowns the file descriptors
connectionList *removeConnection(int file_descriptor); // removes 1 file descriptor
connectionList* gethead(void); // retrieves the head

/* Function related to signal handling */
void* signalHandling(void* arg); 

/*Function related to make sure Distribution of Data is assured */
void SetTop(int h);
int GetTop(void);
