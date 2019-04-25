#include "clipboard.h"




int main()
{
	
		
	int fd = clipboard_connect(SOCK_PATH);

	if(fd== -1)
	{
		perror("\nClipboard cannot be accessed!\n");
		exit(-1);
	}

	FILE* fp=NULL;
	char aux[100];
	char *che=NULL;
	char* str_aux=NULL;
	int operation_int,r;
	long int size=0;
	long int data_size=CHAR_LIMIT;
	char* aux_v=NULL;
	str_aux=(char*)malloc(sizeof(char)*CHAR_LIMIT);
	aux_v=(char*)malloc(sizeof(char)*CHAR_LIMIT);
	char auxie[100];
	strcpy(str_aux,"");
	int i;
	
	strcpy(aux,"0");
		
	while((atoi(aux))==0)
	{
	printf("\nDo you want to read (1) from region or copy (0) to region?\n");
		che=fgets(aux, 5, stdin);
		if (che==NULL)
			exit(0);
		operation_int = atoi(aux);
		// DATA MANIPULATION
		if(operation_int == 0)
		{	
			memset(str_aux,0,CHAR_LIMIT);
			printf("\n What file you want to copy ?\n"); 
			che=fgets(auxie,CHAR_LIMIT,stdin);
			if (che==NULL)
			exit(0);
			sprintf(str_aux,"./Files/%s",auxie);
			str_aux[strlen(str_aux)-1]='\0';
			printf("%s\n",str_aux);
			
			fp=fopen(str_aux,"r");
			if (fp==NULL)
			{
				perror("That isnt a file!- data_teste");
				exit(0);
			}
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			printf("%li\n",size);
			fseek(fp,0,SEEK_SET);
			aux_v=data_check(size,&data_size,aux_v);
			i=0;
			while(feof(fp)==0)
				aux_v[i++] = fgetc(fp);
			
			aux_v[i]='\0';
		

			printf("\nIn which region do you want to save it?\n");
			che=fgets(aux, 5, stdin);
			if (che==NULL)
				exit(0);
			r = atoi(aux);
			clipboard_copy(fd,r,aux_v,size);
			fclose(fp);
		}
		else if(operation_int == 1)
		{
			printf("\nFrom what region do you want to read from?\n");
			che=fgets(aux, 5, stdin);
				if (che==NULL)
					exit(0);
			r = atoi(aux);
			printf("whats the size of the file in KB?\n");
			che=fgets(aux, 100, stdin);
				if (che==NULL)
					exit(0);
			size=atoi(aux)*1024;
			printf("\n What's the extention of the file?\n");
			che=fgets(aux, 10, stdin);
				if (che==NULL)
					exit(0);
			aux_v=data_check(size,&data_size,aux_v);
			aux[strlen(aux)-1]='\0';
			memset(aux_v,0,data_size);
			memset(str_aux,0,CHAR_LIMIT);
			sprintf(str_aux,"./Answer/DATA_TESTE-%d-Copied%s",r,aux);
			fp=fopen(str_aux,"w");
			if(clipboard_paste(fd,r,aux_v,data_size) == 0)
			{
				perror("not enough space in the string \n");
				exit(0);
			}
			fwrite(aux_v,sizeof(char),data_size,fp);
			fclose(fp);
		}
		else
		{
			printf("\nNo operation selected!\n");
			continue;

		}
	
		printf("Continue? Type 0\n");
		che=fgets(aux, 4,stdin);
		if (che==NULL)
			exit(0);
		printf("\n");
		
	}

	close(fd);
	exit(0);
	
	
	
	
	
}
