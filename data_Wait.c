#include "clipboard.h"

#define MAX_MEM 45000000


int main()
{
	

		FILE* fp;	
	int fd = clipboard_connect(SOCK_PATH);
	void* aux=malloc(MAX_MEM);
	long int data_size=MAX_MEM;
	srand(time(NULL));
	int r=0;
	char str_aux[100];

	if(fd== -1)
	{
		perror("\nClipboard cannot be accessed!\n");
		exit(-1);
	}
	
			r=rand()%10;
			memset(str_aux,0,100);
			memset(aux,0,data_size);
			sprintf(str_aux,"./Answer/-DATA-WAIT-%d-Copied.mp4",r);
			fp=fopen(str_aux,"w");
			printf(" i want region -> %d \n",r);
			clipboard_wait(fd,r,aux,data_size);
			fwrite(aux,sizeof(char),data_size,fp);
			fclose(fp);
			clipboard_close(fd);
			
			return 0;
			
}
