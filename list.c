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

/* List of connections made to a local clipboard */
struct connectList
{
	int fd;
	struct connectList *next;
};

/* Pointer to the head of the connected sockets' list */
connectionList *headList;

/********************************** FUNCTIONS **********************************/

/*
Function: createConnectionList
 
Functionality: creates a list of connected INET sockets

Arguments:
 
 • file_descriptor – file descriptor of the local socket to which other clipboards connect to
 
 Return Values: pointer to the head of the list;
*/
connectionList* createConnectionList(int file_descriptor)
{
	headList=NULL;
	connectionList *creator;
	
	creator =(connectionList*)malloc(sizeof(connectionList));
	if(creator == NULL)
	{
		perror("Unable to create list! function: createConnectionList\n");
		exit(-1);
	}

	creator->fd = file_descriptor;
	creator->next = NULL;

	/*connectionList *aux = creator;

	printf("\nMy fd: %d\nList: \n", file_descriptor);

	while(aux != NULL)
	{
		printf("\n\t%d\n", aux->fd);
		aux = aux->next;
	}*/
	headList=creator;
	return(creator);
}

/*
Function: addNewConnection
 
Functionality: after receiving a new connection, add the file descriptor of the created socket to the list

Arguments:
 
 • file_descriptor – file descriptor of the new connected socket

 Return Values: N/A;
*/
void* addNewConnection(int file_descriptor, connectionList* head)
{

	connectionList *new_connection = NULL;
	connectionList *guide = headList;
	

	new_connection =(connectionList*)malloc(sizeof(connectionList));
	if(new_connection == NULL)
	{
		perror("Unable to add the new connection! function: newConnection()\n");
		exit(-1);
	}

	while(guide->next != NULL)
	{
		guide = guide->next;
	}

	guide->next = new_connection;
	new_connection->fd = file_descriptor;
	new_connection->next = NULL;



	/*connectionList *aux = headList;
	while(aux != NULL)
	{
		printf("\n\fd:%d\n", aux->fd);
		aux = aux->next;
	}
	*/
	
	return head;
}

/*
Function: freeConnectionList
 
Functionality: clear the list of connected sockets

Arguments: N/A

 Return Values: N/A
*/
void freeConnectionList()
{
	connectionList *aux = NULL;
	connectionList *aux2=headList;

	while(aux2 != NULL)
	{
		aux = aux2;
		shutdown(aux->fd,SHUT_RDWR);  //closes sockets
		aux2 = aux2->next;
		free(aux);

	}
	
	headList=NULL;
	
	
}

/*
Function: removeConnection
 
Functionality: remove the connected socked identified by file_descriptor

Arguments: 

 • file_descriptor – file descriptor of the socket to be removed

 Return Values: head of the list
*/
connectionList *removeConnection(int file_descriptor)
{
	connectionList *aux = headList;
	connectionList *head=headList;
	connectionList *aux2=NULL;

	/* Remove the headList of the list */
	
	if (file_descriptor==GetTop())
		SetTop(-1);
	
	if (headList!=NULL)
	{
		if(aux->fd == file_descriptor)
		{
			headList = headList->next;
			free(aux);
		} else
		{	
				
			aux2 = aux->next;
			if (aux2!=NULL)
			{
			/*1st element after headList to be removed*/
				if(aux2->fd == file_descriptor)
				{
					head->next = aux2->next;
					aux2->next = NULL;
					free(aux2);
				}
				else
				{
					while(aux2->fd != file_descriptor)
					{
						aux = aux->next;
						
						if(aux2->next==NULL)
						{
							return headList;
						}
						aux2 = aux2->next;
					}
					aux->next = aux2->next;
					close(aux2->fd);
					aux2->next = NULL;
					free(aux2);
				}
			}
		}
	}
	
	return headList;
}


