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
/* Structure containing the region to write/read from, the size of the message to be delivered and which operation to do, write (1) or read (0) */
struct messageStruct
{
	int region;
	long int size;
	int mode;
};


/*
Function: clipboard_connect
 
Functionality: This function is called by the application before interacting with the distributed clipboard.
 
Arguments:
 
 • clipboard_dir – this argument corresponds to the directory where the local clipboard was launched
 
 Return Values: The function return -1 if the local clipboard can not be accessed and a positive value in case of success. The returned value will be used in all other functions as clipboard_id.
*/
int clipboard_connect(char *clipboard_dir)
{
    int clipboard_id;  
   	int suc = 1;
	struct sockaddr_un address;
	
	/* Clear memory for the address */
	memset(&address,0,sizeof(address));

	/* Define the type of socket */
	address.sun_family = AF_UNIX;

	/* Define the path of the socket */
	memmove(address.sun_path, clipboard_dir, sizeof(address.sun_path)-1);

	printf("Socket path: %s\n",address.sun_path);

	/* Create the UNIX socket */
	clipboard_id = socket(AF_UNIX,SOCK_STREAM,0);
	
	/* Connect to the UNIX socket */
	suc = connect(clipboard_id,(struct sockaddr*) &address, sizeof(struct sockaddr));

	/* Verify successful connection */
	if(suc == -1)
	{	
		printf("Unable to connect to the clipboard! %d\n ",errno);
		//perror("u suck\n");
		exit(-1);
	}

	printf("Connected to the clipboard\n");

	return clipboard_id;
}
/*
Function: clipboard_copy
 
Functionality: This function copies the data pointed by buf to a region on the local clipboard.
 
Arguments:
 
• clipboard_id – this argument corresponds to the value returned by clipboard_connect
 
• region – This argument corresponds to the identification of the region the user wants to copy the data to. This should be a value between 0 and 9.
 
• buf – pointer to the data that is to be copied to the local clipboard
 
• count – the length of the data pointed by buf.
 
 Return Values: This function returns positive integer corresponding to the number of bytes copied or 0 in case of error (invalid clipboard_id, invalid region or local clipboard unavailable).
 */
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count)
{
	message m;
	long int send_success=0;
	long int buffer=0;
	
	
	memset(&m, 0, sizeof(message));
	
	m.region=region;
	m.mode=10;
	m.size=count;
	
	/* Check if the region is valid */
	if(region < 0 || region > 9)
	{
		printf("Invalid region! Regions available: 0-9!\n");
		return 0;
	}

	/* Send the information of the message to the clipboard and the message to be saved  */
	send_success = send(clipboard_id,&m,sizeof(message),0); 
	if(send_success < 0)
	{
		printf("Unable to send information to the clipoard! Location: library.c - clipboard_copy\n");
		return 0;
	}
	send_success=0;
	while (buffer<m.size)
	{
	send_success = send(clipboard_id,buf+buffer,m.size,0); 
	/* Verify successful send */
	if(send_success < 0)
	{
		printf("Unable to send information to the clipoard! Location: library.c - clipboard_copy\n");
		return 0;
	}
	buffer+=send_success;
	}
	/* Verify successfull reception */
	if((recv(clipboard_id,&m,sizeof(message),0)) < 0) 
	{
		printf("Unable to receive message that the information was received in the clipoard! Location: library.c - clipboard_copy\n");
		return 0;
	}
	
	return m.size;
}

/*
Funcion: clipboard_paste
 
Functionality: This function copies from the system to the application the data in a certain region. The copied data is stored in the memory pointed by buf up to a length of count.
 
Arguments:
 
 • clipboard_id – this argument corresponds to the value returned by
 clipboard_connect
 
 • region – This argument corresponds to the identification of the region the user
 wants to paste data from. This should be a value between 0 and 9. • buf – pointer to the data where the data is to be copied to
 
 • count – the length of memory region pointed by buf
 
Return Values: This function returns a positive integer corresponding to the number of bytes copied or 0 in case of error (invalid clipboard_id, invalid region or local clipboard unavailable).
 
 */
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count)
{
	message m;
	long int buffer=0;
	memset(&m, 0, sizeof(message));
	m.mode=1;
	m.region=region;
	m.size=(long int)count;
	int send_success=0;
	long int recv_success=0;

	/* Check if the region is valid */
	if(region < 0 || region > 9)
	{
		printf("Invalid region! Regions available: 0-9!\n");
		return(0);
	}
	send_success = send(clipboard_id,&m,sizeof(message),0);

	/* Verify successful send */
	if(send_success<=0)
	{
		return 0;
	}
	
	memset(&m,0,sizeof(message));
	/* Receive the information about the message and a message from the app  */
	recv_success = recv(clipboard_id,&m,sizeof(message),0);
	
	if (m.size<=0 && m.region!=region && m.mode!=1)
		return 0;
	
	if(recv_success < 0)
	{
		printf("Unable to receive information! Location: library.c - clipboard_paste\n");
		return 0;
	}
	recv_success=0;
	
	if (m.size>count)
	{
		while (buffer<count)
		{
			recv_success = recv(clipboard_id,buf+buffer,count-buffer,0);
			buffer+=recv_success;
			if(recv_success < 0)
			{
				printf("Unable to receive information! Location: library.c - clipboard_paste\n");
				return 0;
			}
		
		}
		
	}
	else while (buffer<m.size)
		{
			recv_success = recv(clipboard_id,buf+buffer,m.size-buffer,0);
			buffer+=recv_success;
			if(recv_success < 0)
			{
				printf("Unable to receive information! Location: library.c - clipboard_paste\n");
				return 0;
			}
		}
	/* Verify successful receives */
	
	


	return m.size;
}

/*
Funcion: clipboard_wait
 
Functionality: This function waits for a change on a certain region (new copy), and when it happens the new data in that region is copied to memory pointed by buf. The copied data is stored in the memory pointed by buf up to a length of count.
 
Arguments:
 
 • clipboard_id – this argument corresponds to the value returned by
 clipboard_connect
 
 • region – This argument corresponds to the identification of the region the user
 wants to wait for. This should be a value between 0 and 9. • buf – pointer to the data where the data is to be copied to • count – the length of memory region pointed by buf.
 
Return Values: This function returns a positive integer corresponding to the number of bytes copied or 0 in case of error (invalid clipboard_id, invalid region or local clipboard unavailable).
 */
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count)
{
    //TO DO
	message m;
	long int buffer=0;
	memset(&m, 0, sizeof(message));
	m.mode=WAIT_WRITE;
	m.region=region;
	m.size=(long int)count;
	long int send_success=0;
	long int recv_success=0;

	/* Check if the region is valid */
	if(region < 0 || region > 9)
	{
		printf("Invalid region! Regions available: 0-9!\n");
		return(0);
	}
	send_success = send(clipboard_id,&m,sizeof(message),0);

	/* Verify successful send */
	if(send_success<=0)
	{
		return 0;
	}
	

	/* Receive the information about the message and a message from the app  */
	recv_success = recv(clipboard_id,&m,sizeof(message),0);
	
	if (m.size<=0 && m.region!=region && m.mode!=1)
		return 0;
	
	
	if(recv_success < 0)
	{
		printf("Unable to receive information! Location: library.c - clipboard_paste\n");
		return 0;
	}
	recv_success=0;
	
	if (m.size>count)
	{
		while (buffer<count)
		{
			recv_success = recv(clipboard_id,buf+buffer,count-buffer,0);
			buffer+=recv_success;
			if(recv_success < 0)
			{
				printf("Unable to receive information! Location: library.c - clipboard_paste\n");
				return 0;
			}
		
		}
		
	}
	else while (buffer<m.size)
		{
			recv_success = recv(clipboard_id,buf+buffer,m.size-buffer,0);
			buffer+=recv_success;
			if(recv_success < 0)
			{
				printf("Unable to receive information! Location: library.c - clipboard_paste\n");
				return 0;
			}
		}
	/* Verify successful receives */

	return m.size;
}
/*
Funcion: clipboard_close
 
Functionality: This function closes the connection between the application and the local clipboard.
 
Arguments:
 
 • clipboard_id – this argument corresponds to the value returned by clipboard_connect
 
Return Values:  Upon successful completion, a value of 0 is returned.  Otherwise, a value of -1 is returned and the global integer variable errno is set to indi-
 cate the error.
 
 */
void clipboard_close(int clipboard_id)
{
    close(clipboard_id);
}


