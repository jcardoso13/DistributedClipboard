#include "clipboard.h"




int main()
{
	

	int fd = clipboard_connect(SOCK_PATH);
	long int size=0;
	void* aux=NULL;
	long int data_size=0;
	char* aux2=NULL;
	int i=0;
	srand(time(NULL));
	int r=0;
	char str_aux[100];

	if(fd== -1)
	{
		perror("\nClipboard cannot be accessed!\n");
		exit(-1);
	}
	
	
	FILE* fp=NULL;
	
		fp=fopen("./Files/vid1.mp4","r");
			if (fp==NULL)
			{
				perror("That isnt a file!- data_teste");
				sleep(1);				exit(0);
			}
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			printf("%li\n",size);
			aux=data_check(size,&data_size,aux);
			aux2=aux;
			fseek(fp,0,SEEK_SET);
			i=0;
			while(feof(fp)==0)
				aux2[i++] = fgetc(fp);
				
			aux2[i]='\0';
			
			r=rand()%10;
		
			printf("data_size-%li\n",data_size);
			clipboard_copy(fd,r,aux,data_size);
			
			clipboard_paste(fd,r,aux,data_size);
			memset(str_aux,0,100);
			memset(aux,0,data_size);
			sprintf(str_aux,"./Answer/SET-VIDEO-%d-Copied.mp4",r);
			fp=fopen(str_aux,"w");
			clipboard_paste(fd,r,aux,data_size);
			fwrite(aux,sizeof(char),data_size,fp);
			fclose(fp);
			clipboard_close(fd);
			return 0;
}
