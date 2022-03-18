#include "header.h" 

void sys_err (const char *msg) 
{
  perror(msg);
  exit (1);
}


using namespace std;
int main()
{
    int s_cl=0, // Сокет для клиента 
        s_serv=0, //Сокет для сервера
        bytes_read=0, //фактический размер сообщения
        result=0;
    struct sockaddr_in addr; //Структура для хранения адреса
    char buf[1024]; //сообщение

    s_serv = socket(AF_INET, SOCK_STREAM, 0); //создание сокета
    if(s_serv < 0)
        sys_err ("Prog2: Not socket\n");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_SOCK);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // inet_addr("127.0.0.1"); htonl(INADDR_ANY); htonl(INADDR_LOOPBACK);
    if(bind(s_serv, (struct sockaddr *)&addr, sizeof(addr)) < 0) //имменование сокета
    {
        sys_err ("Prog2: Not bind\n");
    }

    listen(s_serv, 1); //ожидание запросов
    printf ("Wait data:\n");

    while(1)
    {
        bytes_read=1; //размер сообщения
        s_cl = accept(s_serv, NULL, NULL); //сокет нового клиента
        if(s_cl < 0)
        {
            sys_err ("Prog2: Not accept\n");
        }

        while (1)
        {
            bytes_read = recv(s_cl, buf, 1024, 0);
            if(bytes_read <= 0) break;
            
            try
            {
                result=stoi(buf); 
            }
            catch (...)
            {cout << "Prog2: Not int\n";}

            if (((result >= 10) or (result <=-10)) and (result % 32 == 0))
                printf ("Correct (%d)\n", result);
            else 
                printf ("No correct (%d)\n", result);
        }
        close(s_cl);
    }

    return 0;
}
