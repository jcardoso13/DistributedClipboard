#include "clipboard.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
		
	int fd = clipboard_connect(SOCK_PATH);

	if(fd== -1)
	{
		perror("\nClipboard cannot be accessed!\n");
		exit(-1);
	}

	
	char aux[5];
	char* che;
	char* str_aux=NULL;
	int operation_int,r, i = 0, c;
	long int current_size = CHAR_LIMIT;
	str_aux=malloc(sizeof(char)*CHAR_LIMIT);
	strcpy(str_aux,"");
	strcpy(aux,"0");

		
	while((atoi(aux))==0)
	{
	printf("\nDo you want to read (1) from region or copy (0) to region?\n or you want to wait(2)?");
		che=fgets(aux, 5, stdin);
		if (che==NULL)
			exit(0);
		operation_int = atoi(aux);
		// DATA MANIPULATION
		if(operation_int == 0)
		{	
			memset(str_aux,0,current_size);
			printf("\nWhat expression do you wish to save?\n");
			i=0;
			while( (c = getchar()) != '\n' )
			{	
				str_aux[i++] = (char)c;
				if(i == current_size)
				{
					current_size = i + CHAR_LIMIT;
					str_aux = realloc(str_aux, current_size);
				}

			}

			printf("\n\n%li\n\n", current_size);
			if (str_aux!=NULL)
			printf("\n %s", str_aux);
			printf("\nIn which region do you want to save it?\n");
			che=fgets(aux, 5, stdin);
			if (che==NULL)
			exit(0);
			r = atoi(aux);
			clipboard_copy(fd,r,str_aux,current_size);
		}
		else if(operation_int == 1)
		{
			printf("\nFrom what region do you want to read from?\n");
			che=fgets(aux, 5, stdin);
			if (che==NULL)
				exit(0);
			r = atoi(aux);
			memset(str_aux,0,current_size);
			if(clipboard_paste(fd,r,str_aux,current_size) == 1)
			{
				printf("\nNothing saved in this region!\n");
				continue;
			}
			printf("%s\n",str_aux);
		}
		else if(operation_int==2)
		{
			printf("\nFrom what region do you want to read from?\n");
			che=fgets(aux, 5, stdin);
			if (che==NULL)
				exit(0);
			r = atoi(aux);
			memset(str_aux,0,CHAR_LIMIT);
			if(clipboard_wait(fd,r,str_aux,CHAR_LIMIT) == 1)
			{
				printf("\nNothing saved in this region!\n");
				continue;
			}
			printf("%s\n",str_aux);
			
			
			
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




int Check(char* aux)
{
	if (aux==NULL)
		exit(0);
	
	
	
	return 0;
}

