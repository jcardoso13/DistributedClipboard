#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define CLIPBOARD_MAX 100

int main()
{
int i=0;
system("gnome-terminal -e ../clipboard");
for (i=0;i<CLIPBOARD_MAX;i++)
{
	system ("gnome-terminal -e ../set_value");
}
return 0;
}
