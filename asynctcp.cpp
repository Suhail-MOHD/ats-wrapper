#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<vector>
#include<mutex>
#include<unistd.h>
#include<stdlib.h>
#include<iostream>
#include<time.h>
using  namespace std;

#ifndef ASYNCTCP
#define ASYNCTCP
#define EXIT 1
#define ERROR 2
#define WARN 3
#define LOG 4
#define DEBUG 5
template <class T>
class client
{
  int fd;
  public:
  T client_data;
  client(int fd_1,struct sockaddr_in client_addr)
  {
    fd_1 = fd;
    //TODO: Something needs to be done with client_addr here
    // Otherwise remove it
  }
};
template <class T>
class asyncTcp{
  private:
    // File descriptor for socket on which we are accepting connections
    int sock_fd;
    int debug_level;
    int max_connections;
    pthread_t accept_loop_thread,event_dispatcher_thread;
    struct sockaddr_in server_addr;
    // port number to listen on
    int port_no;
    // client data
    vector<client<T>> clients;
    // mutex for access to clients vector
    mutex clients_access;
    static void* accept_loop(void  *data);
    static void* event_dispatcher(void *data);

  public:
    asyncTcp(int port_no_1,int max_connections_1 = 100,int debug_level_1 = LOG)
      {
      port_no = port_no_1;
      bzero((char *)&server_addr,sizeof(server_addr));
      server_addr.sin_family = AF_INET;
      server_addr.sin_port = htons(port_no);
      server_addr.sin_addr.s_addr = INADDR_ANY;
      debug_level = debug_level_1;
      }
    // start server
    void start_server();
    // close server
    void close_server();
    // close specific client connection
    void close_connection(client<T> target_client);
    void print_message(string message,int level = LOG)
      {
        time_t now;
        struct tm *timeinfo;
        string msg;
        char time_stamp[11];
        if(level<=debug_level)
        { time(&now);
          timeinfo = localtime(&now);
          strftime(time_stamp,80,"<%H:%M:%S>",timeinfo);
          switch(level)
          {
            case EXIT:msg="ERROR ";
            break;
            case ERROR:msg="ERROR ";
            break;
            case WARN:msg="WARNING ";
            break;
            case LOG:msg="LOG ";
            break;
            case DEBUG:msg="DEBUG ";
            break;
          }
          msg=msg+time_stamp+" : "+message;
          cout<<msg<<endl;
          if(level == EXIT)
          exit(0);
        }
      }

    //  following functions must be overloaded
    void handle_new_client(int newsockfd,struct sockaddr_in client_addr) = 0;
    void on_recieve()=0;
};
template <typename T>
void asyncTcp<T>::start_server()
  {
    int res;
    // creating socket
    sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd<0)
    {
      print_message("Could not create Socket",EXIT);
    }
    print_message("Created Socket",LOG);

    // Binding socket to port
    if( bind(sock_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0 )
    print_message("Could not bind socket",EXIT);
    print_message("Socket successfully bound to port",LOG);
    listen(sock_fd,max_connections);

    // starting accept thread
    pthread_create(&accept_loop_thread,NULL,accept_loop,this);
    pthread_join(accept_loop_thread,NULL);




    //close(sock_fd);
  }
  template <typename T>
void* asyncTcp<T>::accept_loop(void  *data)
  {
    asyncTcp* server = (asyncTcp *)data;
    bool should_break = false;
    int newsockfd;
    struct sockaddr_in client_addr;
    socklen_t cli_len;
    cli_len = sizeof(client_addr);
    while(!should_break)
    {
      newsockfd = accept(server->sock_fd,(struct sockaddr*)&client_addr,&cli_len);
      if(newsockfd < 0)
      {
        server->print_message("Error accepting client",ERROR);
      }
      server->print_message("Accepted client",LOG);
      server->clients_access.lock();
      server->clients.push_back(client<T>(newsockfd,client_addr));
      server->clients_access.unlock();
      server->handle_new_client(newsockfd,client_addr);
    }
  }

template <typename T>
void* asyncTcp<T>::event_dispatcher(void *data)
  {
    //print_message("Hello",LOG);
  }
#endif
int main(int argc,char *argv[])
{
  asyncTcp<int> my_server(atoi(argv[1]));
  my_server.start_server();
  return 0;
}
