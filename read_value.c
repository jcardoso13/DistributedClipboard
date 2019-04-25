#include "clipboard.h"




int main()
{
	int i=0;
	int fd = clipboard_connect(SOCK_PATH);
	char aux[100];
	char*che;
	if(fd== -1)
	{
		perror("\nClipboard cannot be accessed!\n");
		exit(-1);
	}
	printf("000\n");
	che=fgets(aux,5,stdin);
	if (che==NULL)
		return 0;
	for (i=0;i<10;i++)
	{
		clipboard_paste(fd,i,aux,100);
		printf("%s\n",aux);
	}

	clipboard_close(fd);
	sleep(10);
	
	
return 0;
}
