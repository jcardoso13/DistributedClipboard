# PSISProj

System Programming Project in C

 Destributed Clipboard.

Goal: To make a Clipboard application that connected over the internet to other clipboard applications in which all stayed synchronized. 10 Data regions without Maximum Data size or data defined. Can transfer whole mp4 files for example.

Test apps that connect to these clipboards were created in folder ./testes (TESTE1 and TESTE2)

To make an App that can connect to the Clipboard application, it must follow it's API. (clipboard_connect, clipboard_copy, clipboard_paste, clipboard_Wait and clipboard_close) described in library.c and clipboard.h

To compile the tests and clipboard do:

> make

to run the clipboard do:

> ./clipboard 


in single mode (1st clipboard created)


> ./clipboard -c "IP of other Clipboard" "Port"


in connected mode 

