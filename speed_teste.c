#include "clipboard.h"




int main()
{
	float ti;
	int i=0;
	int fd = clipboard_connect(SOCK_PATH);
	long int size;
	char aux[100];
	int aux2;
	if(fd== -1)
	{
		perror("\nClipboard cannot be accessed!\n");
		exit(-1);
	}
	
	for (i=0;i<10;i++)
	{
	srand(time(NULL));
	ti=rand()%10;
	ti=ti/1000;
	aux2 = rand()%99;
	sleep(ti+i);
	printf("Sending %d string! Number:%d\n",i, aux2);
	sprintf(aux,"%d-%d",i, aux2);
	size=sizeof(aux);
	clipboard_copy(fd,i,aux,size);
	}
	sleep(10);
	for (i=0;i<10;i++)
	{
		clipboard_paste(fd,i,aux,100);
		printf("%s\n",aux);
	}

	
	
	
return 0;
}
