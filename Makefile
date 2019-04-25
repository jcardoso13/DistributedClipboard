#-----VARIABLES--------------------------------------


#Compiler

CC = gcc

#Compiler Flags

CLIPBOARD_FLAGS = -g -O3 -Wall -pthread

APPLICATION_FLAGS = -g -O3 -Wall 

#-----EXECUTABLES------------------------------------------------

all: clipboard speed_teste app_teste2 set_value data_teste read_value set_video data_Wait

clipboard: clipboard.c AF_UNIX.c AF_INET.c list.c sync.c clipboard.h
	$(CC) $(CLIPBOARD_FLAGS) -o clipboard clipboard.c AF_UNIX.c AF_INET.c list.c sync.c


speed_teste: speed_teste.c library.c clipboard.h
	$(CC) $(APPLICATION_FLAGS) -o speed_teste speed_teste.c library.c
	
app_teste2: app_teste2.c library.c clipboard.h
	$(CC) $(APPLICATION_FLAGS) -o app_teste2 app_teste2.c library.c
	
set_value: set_value.c library.c clipboard.h
	$(CC) $(APPLICATION_FLAGS) -o set_value set_value.c library.c

read_value: read_value.c library.c clipboard.h
	$(CC) $(APPLICATION_FLAGS) -o read_value read_value.c library.c
	
data_teste: data_teste.c library.c sync.c list.c AF_INET.c clipboard.h
	$(CC) $(CLIPBOARD_FLAGS) -o data_teste data_teste.c library.c sync.c list.c AF_INET.c


set_video: set_video.c library.c sync.c list.c AF_INET.c clipboard.h
	$(CC) $(CLIPBOARD_FLAGS) -o set_video set_video.c library.c sync.c AF_INET.c list.c

data_Wait:data_Wait.c library.c sync.c list.c AF_INET.c clipboard.h
	$(CC) $(CLIPBOARD_FLAGS) -o data_Wait data_Wait.c library.c sync.c list.c AF_INET.c



.PHONY: clean
clean: 
	rm clipboard  speed_teste app_teste2 set_value data_teste set_video read_value data_Wait
