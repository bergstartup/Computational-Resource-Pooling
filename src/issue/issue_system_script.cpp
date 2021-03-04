/*
About issue_system_script
1.Gets activated by issue_init
2.Get cmd line arguments and parse it[debug,port,noofreplicas]
3.TCP listen@port
4.Get multiple connections, and execute syscall handlers async for each connections
5.Have a common syscall_capture vector
6.Use the FT approch to reply to syscalls
exec:
./issue -p -r -d //p for port, r for no of replicas, d for debug
*/

/*
###############################################################
Proposing change in structure
A thread to listen for connections
If got one,run a thread that handles syscalls
By keeping syscall_capture vector common to all handler threads
###############################################################
*/

/*
-------------------------
Libraries import
-------------------------
*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread.h>
#include <getopt.h>
/*
-------------------------
Global declarations
-------------------------
*/
bool debug=false;
vector<int> syscall_capture;//common to handler threads require mutex

/*
-------------------------
Function descriptions
-------------------------
*/

//TCP socket listner@port
int socket_listen(int port,int replicas){
  /*
  Let master thread run this socket listener
  On connection, create a thread and run syscall_handler with the new sock as parameter
  And listen for connections again
  */
  //decl(s)
  int serverfd,new_socket;
  int opt=1;
  struct sockaddr_in address;
  int addrlen=sizeof(address);

  //server socker def
  serverfd=socket(AF_INET,SOCK_STREAM,0); //create a socket
  //addr definition
  address.sin_family=AF_INET;
  address.sin_addr.s_addr=INADDR_ANY;
  address.sin_port=htons(port);
  //Force the address on socket
  setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt));
  //bind
  bind(serverfd, (struct sockaddr * )&address,sizeof(address));
  //listen
  listen(serverfd,replicas);
  //accept the new connection(s)
  while (true){
    new_socket=accept(serverfd,(struct sockaddr * )&address,(socklen_t *)&addrlen);
    if (debug)
      printf("Connected with \n");//print as well as the address
    //create thread with new_socket as parameter
    std::thread th(syscall_handler,new_socket);
  }
}

//Syscall handlers
void syscall_handler(int sock){
  //necessary variable declarations
  char recv_buf[100];
  int *syscallid,*syscall,*size,*fd,*flag,*mode;
  long long int *dirfd;
  char *msg,buf[100],*makemsg;
  int ret,sock;
  struct stat send_stat;

  //read msg from socket, contains [syscall,args...]
  read(sock,recv_buf,100);
  //Extract syscall from msg
  syscall = (int *)recv_buf;
  //Loop till recv syscall as -1
  while(*syscall!=-1){
  if(debug)
    printf("Syscall %d\n",*syscall);

   switch(*syscall){
     //genral structure
     /*
     case syscall:
      1. Get the argument
      2. Execute the syscall
      3. Compose the return message
      4. Send to server
     */
     //read
     case 0:
      // int fd,int size
      fd = (int *)(recv_buf+sizeof(int)+1);
      size = (int *)(recv_buf+2*sizeof(int)+2);
      ret=read(*fd,buf,*size);
      if(debug)
        printf("Syscall: read | Attrib: [int : fd] %d , [int: size] %d | ret: [int] %d\n",*fd,*size,ret);
      makemsg=(char *)malloc(sizeof(int)+ret+1);
      memcpy(makemsg,&ret,sizeof(int));
      memcpy(makemsg+sizeof(int)+1,buf,ret);
      send(sock,makemsg,sizeof(int)+ret+1,0);
      break;


    //write
     case 1:
      // int fd, int size, string msg
      fd=(int *)(recv_buf+sizeof(int)+1);
      size = (int *)(recv_buf+2*sizeof(int)+2);
      msg = recv_buf+3*sizeof(int)+3;
      ret=write(*fd,msg,*size);
      if(debug)
        printf("Syscall: write | Attrib: [int : fd] %d , [int: size] %d | ret: [int] %d\n",*fd,*size,ret);
      for(int i=0;i<2;i++)
      send(sock,&ret,sizeof(int),0);
      break;

    //close
    case 3:
      //close fd
      fd=(int *)(recv_buf+sizeof(int)+1);
      ret=close(*fd);
      if(debug)
        printf("Syscall: close | Attrib: [int : fd] %d | ret: [int] %d\n",*fd,ret);
      send(sock,&ret,sizeof(int),0);
      break;

    case 5:
      //fstat fd
      fd=(int *)(recv_buf+sizeof(int)+1);
      fstat(*fd,&send_stat);
      msg=(char *)&send_stat;
      if(debug)
        printf("Syscall: fstat | Attrib : [int : fd] %d\n",*fd);
      send(sock,msg,sizeof(struct stat),0);
      break;

    case 257:
      //openat dirfd,flags,filename
      dirfd=(long long int *)(recv_buf+sizeof(int)+1);
      flag = (int *)(recv_buf+1*sizeof(int)+sizeof(long long int)+2);
      mode = (int *)(recv_buf+2*sizeof(int)+sizeof(long long int)+3);
      msg = recv_buf+sizeof(long long int)+3*sizeof(int)+4;
      //printf("All parameters: %lld , %d , %d ,%s\n",*dirfd,*flag,*mode,msg);
      ret=openat(*dirfd,msg,*flag,*mode);
      if(debug)
        printf("Syscall: openat | Attrib: [dirfd : long long int] %lld , [flag : int] %d , [mode : int ] %d\n",*dirfd,*flag,*mode);
      send(sock,&ret,sizeof(int),0);
      break;

    default:
      break;
   }//end of switch
   read(sock,recv_buf,100);
   syscall = (int *)recv_buf;
  }//end of while
  //close socket
}

//Main function
int main(int args,char *argv[]){
  //Parsing cmd line arguments; use getopt
  for(int i=1;i<args;i++){
    if(strcmp(argv[i],"-d")==0)
      debug=true;
  }

  //Listen to connections
  socket_listen();

}//end of main
