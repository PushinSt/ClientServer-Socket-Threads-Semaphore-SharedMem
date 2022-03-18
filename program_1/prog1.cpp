
#include "header.h"

void sys_err (const char *msg) 
{
  perror (msg);
  exit (1);
}

bool comp2 (char a,char  b) { return (a>b); }   // сравнение, для сортировки

void* thread1(void *args)
{
  int semid;            // идентификатор семафора 
  int shmid;            // идентификатор разделяемой памяти 
  mess *msg_p;          // адрес сообщения в разделяемой памяти                                
  char str[SIZE_STR];
  std::string my_str;
  size_t found;

  // SEM_ID 
  if ((semid = semget (SEM_ID, 1, 0)) < 0)
    sys_err ("Prog1.Th1: can not get semaphore\n");

  // получение доступа к сегменту разделяемой памяти 
  if ((shmid = shmget (SHM_ID, sizeof (mess), 0)) < 0)
    sys_err ("Prog1.Th1: can not get shared memory segment\n");

  // получение адреса сегмента 
  if ((msg_p = (mess *) shmat (shmid, 0, 0)) == NULL)
    sys_err ("Prog1.Th1: shared memory attach error\n");

  printf ("Prog1.Th1: Enter data:\n");
  while (1)
  {
    
    scanf ("%s", str);
    my_str=std::string(str);

    // обработка строки
    if (!((my_str.find_first_not_of("0123456789", 0) != std::string::npos)  or (my_str.length()>64)))
    {
      sort (my_str.begin(), my_str.end(),comp2);
      found = my_str.find_first_of("02468", 0);
      while (found!=std::string::npos)
      {
        my_str[found]='K';
        my_str.insert(found+1,"B");
        found = my_str.find_first_of("02468", found+2);
      }
    }
    else
    {
       my_str=std::string("q");   // признак выхода
    } 
    strcpy(str, my_str.c_str());

    // 1- блок. 0 - неблок
    while (semctl (semid, 0, GETVAL, 0) || msg_p->type != MSG_EMPTY)
      /*
      *   если сообщение не обработано или сегмент блокирован - ждать
      *                                                             */
    ;
    semctl (semid, 0, SETVAL, 1);   // блокировать 
    if (str[0] != 'q')
    {
      // записать сообщение
      msg_p->type = MSG_STR;
      strncpy (msg_p->string, str, SIZE_STR);
    }
    else
    {
      // пометить сообщение как конечное 
      msg_p->type = MSG_EXIT;
    }
    semctl (semid, 0, SETVAL, 0);   // отменить блокировку 
    if (str[0]== 'q')
      break;
  }
  shmdt (msg_p);    // отсоединить сегмент разделяемой памяти 
  return NULL;
}

void* thread2(void *args)
{
  size_t found;
  int sum=0;
  std::string my_str;
  int semid;            // идентификатор семафора 
  int shmid;            // идентификатор разделяемой памяти 
  mess *msg_p;          // адрес сообщения в разделяемой памяти 
  
  // создание массива семафоров из одного элемента 
  if ((semid = semget (SEM_ID, 1, PERMS | IPC_CREAT)) < 0)
    sys_err ("Prog1.Th2: can not create semaphore\n");

  // создание сегмента разделяемой памяти 
  if ((shmid = shmget (SHM_ID, sizeof (mess), PERMS | IPC_CREAT)) < 0)
    sys_err ("Prog1.Th2: can not create shared memory segment\n");

  // подключение сегмента к адресному пространству процесса 
  if ((msg_p = (mess *) shmat (shmid, 0, 0)) == NULL)
    sys_err ("Prog1.Th2: shared memory attach error\n");

  semctl (semid, 0, SETVAL, 0);   // установка семафора разблок 
  msg_p->type = MSG_EMPTY;

  while (1)
  {
    if (msg_p->type != MSG_EMPTY)
    {
      if (semctl (semid, 0, GETVAL, 0))   // если занято, то в конец цикла  
        continue;
      semctl (semid, 0, SETVAL, 1);   // установить блокировку 

      // обработка сообщения 
      if (msg_p->type == MSG_STR)
      {
        printf ("Prog1.Th2: %s\n", msg_p->string); 
        my_str=std::string(msg_p->string);
      }
      if (msg_p->type == MSG_EXIT)
        break;
      msg_p->type = MSG_EMPTY;    // сообщение обработано, очищаем память 
      semctl (semid, 0, SETVAL, 0);   // снять блокировку 

      sum=0;
      found = my_str.find_first_of("0123456789", 0);
      while (found!=std::string::npos)
      {
        sum=sum+int(my_str[found]-'0');
        my_str[found]='*';
        found = my_str.find_first_of("0123456789", found+1);
      }

      // soket
      int sock;
      struct sockaddr_in addr;
      sock = socket(AF_INET, SOCK_STREAM, 0);   // создание сокета
      if(sock < 0)
      {
        sys_err ("Prog1.Th2: Not socket\n");
      }

      addr.sin_family = AF_INET;
      addr.sin_port = htons(PORT_SOCK); 
      addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // inet_addr("127.0.0.1"); htonl(INADDR_ANY); htonl(INADDR_LOOPBACK);
      if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)   // подключаемся
      {
        sys_err ("Prog1.Th2: Not connect\n");
      }
      char message[SIZE_STR];
      sprintf(message, "%d", sum);
      send(sock, message, SIZE_STR, 0);
      close(sock);
    }
  }

  // удаление массива семафоров 
  if (semctl (semid, 0, IPC_RMID, (struct semid_ds *) 0) < 0)
    sys_err ("Prog1.Th2: semaphore remove error\n");

  // удаление сегмента разделяемой памяти 
  shmdt (msg_p);
  if (shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0) < 0)
    sys_err ("Prog1.Th2: shared memory remove error\n");

    return NULL;
}



int main ()
{

    pthread_t threads[2];    // массив потоков
    
    pthread_create(&threads[0], NULL, thread2, NULL);
    sleep(1);   // задержка, чтобы второй поток запустился первым и создал семафоры и сегмент памяти
    pthread_create(&threads[1], NULL, thread1, NULL);



    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }


  return 0;
}