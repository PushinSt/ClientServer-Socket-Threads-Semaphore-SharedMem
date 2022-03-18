#include <stdio.h>
#include <unistd.h>
#include <cstdlib>  
#include <cstring>
#include <iostream>
#include <algorithm>
#include <errno.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
 
#define SEM_ID	2001      // ключ массива семафоров 
#define SHM_ID	2002      // ключ разделяемой памяти 
#define PERMS	0666        // права доступа 
#define PORT_SOCK	3428    // порт сокета

// сообщения
#define MSG_EMPTY 0         // пустое сообщение 
#define MSG_STR 1           // непустое сообщение                        
#define MSG_EXIT 2          // конечное сообщение                     
#define SIZE_STR 128       //размер сообщения

// структура сообщения, помещаемого в разделяемую память 
typedef struct
{
  int type;
  char string [SIZE_STR];
} mess;