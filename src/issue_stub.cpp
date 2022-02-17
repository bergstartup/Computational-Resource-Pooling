/*
About issue_system_script
1.Gets activated by issue_init
2.Get cmd line arguments and parse it[debug,port,noofreplicas]
3.TCP listen@port
4.Get multiple connections, and execute syscall handlers async for each connections
5.Have a common syscall_capture vector
6.Use the FT approch to reply to syscalls
exec:
./issue -p -d //p for port, d for debug
*/

/*
###############################################################
Provide SIGPIPE handling function for threads
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
#include <vector>
#include <mutex>
#include <thread>
#include <pthread.h>
#include <getopt.h>
/*
-------------------------
Global declarations
-------------------------
*/
#define EXPECTED_REPLICA_COUNT 3
//using namespace std;
bool debug=false;
std::vector<void *> syscall_capture;//2d 0 for data and 1 for size
std::vector<int> syscall_capture_size;
int count=-1;
std::mutex count_lock;
/*
-------------------------
Function descriptions
-------------------------
*/

//Syscall handlers
void syscall_handler(int sock){
  //necessary variable declarations
  char recv_buf[100],*traverse;
  int *syscall_id,*syscall,*size,*fd,*flag,*mode;
  long long int *dirfd;
  char *msg,buf[100],*makemsg;
  int ret;
  struct stat send_stat;
  if (debug){
    printf("Syscall listening\n");
  }
  //read msg from socket, contains [syscall,args...]

  read(sock,recv_buf,100);
  traverse=recv_buf;
  //Extract syscall_id and syscall from msg
  syscall_id=(int *)traverse;
  syscall = (int *)(traverse+sizeof(int)+1);
  traverse+=2*sizeof(int)+2;//offset
  //Loop till recv syscall as -1
  while(*syscall!=-1){
  if(debug)
    printf("Syscall %d\n",*syscall);

   //sync with all threads
   count_lock.lock();
   if (*syscall_id>count){
     //change global count
     count=*syscall_id;
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
        fd = (int *)traverse;
        size = (int *)(traverse+2*sizeof(int)+2);
        ret=read(*fd,buf,*size);
        if(debug)
          printf("Syscall: read | Attrib: [int : fd] %d , [int: size] %d | ret: [int] %d\n",*fd,*size,ret);
        makemsg=(char *)malloc(sizeof(int)+ret+1);
        memcpy(makemsg,&ret,sizeof(int));
        memcpy(makemsg+sizeof(int)+1,buf,ret);
        send(sock,makemsg,sizeof(int)+ret+1,0);

        //capturing syscall and its size
        syscall_capture_size.push_back(sizeof(int)+ret+1);
        syscall_capture.push_back(makemsg);
        //memcpy(syscall_capture[count],makemsg,syscall_capture_size[count]);
        break;


      //write
       case 1:
        // int fd, int size, string msg
        fd=(int *)traverse;
        size = (int *)(traverse+sizeof(int)+1);
        msg = traverse+2*sizeof(int)+2;
        ret=write(*fd,msg,*size);
        if(debug)
          printf("Syscall: write | Attrib: [int : fd] %d , [int: size] %d | ret: [int] %d\n",*fd,*size,ret);
        send(sock,&ret,sizeof(int),0);

        //capturing syscall and its size
        syscall_capture_size.push_back(sizeof(int));
        syscall_capture.push_back(makemsg);
        break;

      //close
      case 3:
        //close fd
        fd=(int *)traverse;
        ret=close(*fd);
        if(debug)
          printf("Syscall: close | Attrib: [int : fd] %d | ret: [int] %d\n",*fd,ret);
        send(sock,&ret,sizeof(int),0);
        memcpy(syscall_capture[count],makemsg,sizeof(int));

        //capturing syscall and its size
        syscall_capture_size.push_back(sizeof(int));
        syscall_capture.push_back(makemsg);
        //memcpy(syscall_capture[count],makemsg,syscall_capture_size[count]);
        break;

      //fstat
      case 5:
        //fstat fd
        fd=(int *)traverse;
        fstat(*fd,&send_stat);
        msg=(char *)&send_stat;
        if(debug)
          printf("Syscall: fstat | Attrib : [int : fd] %d\n",*fd);
        send(sock,msg,sizeof(struct stat),0);

        //capturing syscall and its size
        syscall_capture_size.push_back(sizeof(struct stat));
        syscall_capture.push_back(makemsg);
        //memcpy(syscall_capture[count],makemsg,syscall_capture_size[count]);
        break;

      //open at
      case 257:
        //openat dirfd,flags,filename
        dirfd=(long long int *)traverse;
        flag = (int *)(traverse+1*sizeof(int)+sizeof(long long int)+2);
        mode = (int *)(traverse+2*sizeof(int)+sizeof(long long int)+3);
        msg = traverse+sizeof(long long int)+3*sizeof(int)+4;
        //printf("All parameters: %lld , %d , %d ,%s\n",*dirfd,*flag,*mode,msg);
        ret=openat(*dirfd,msg,*flag,*mode);
        if(debug)
          printf("Syscall: openat | Attrib: [dirfd : long long int] %lld , [flag : int] %d , [mode : int ] %d\n",*dirfd,*flag,*mode);
        send(sock,&ret,sizeof(int),0);

        //capturing syscall and its size
        syscall_capture_size.push_back(sizeof(int));
        syscall_capture.push_back(makemsg);
        //memcpy(syscall_capture[count],makemsg,syscall_capture_size[count]);
        break;

      default:
        break;
    }//end of switch
   }//end of if for syscall_id check
   else{
      //memcpy(makemsg,syscall_capture[*syscall_id],syscall_capture_size[*syscall_id]);
      send(sock,syscall_capture[*syscall_id],syscall_capture_size[*syscall_id],0);
   }
   count_lock.unlock();

   read(sock,recv_buf,100);
   traverse=recv_buf;
   syscall_id=(int *)traverse;
   syscall = (int *)(traverse+sizeof(int)+1);
   traverse+=2*sizeof(int)+2;//offset
  }//end of while
  //close socket
  close(sock);
  if (debug)
    printf("Finished execution\n");
}


//TCP socket listner@port <change>
void socket_listen(int port){
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
  listen(serverfd,EXPECTED_REPLICA_COUNT);
  //accept the new connection(s)
  while (true){
    new_socket=accept(serverfd,(struct sockaddr * )&address,(socklen_t *)&addrlen);
    if (debug)
      printf("Connected with \n");//print as well as the address
    //create thread with new_socket as parameter
    std::thread th(syscall_handler,new_socket);
    th.detach();
  }
}



//Main function
int main(int args,char *argv[]){
  //Decls
  int port,opt;
  bool debug;
  //Parsing cmd line arguments
  while((opt = getopt(args,argv,"p:d")) != -1){
    switch(opt){
      case 'p':
        port = atoi(optarg);//parse to int
        break;
      case 'd':
        debug = true;
        break;
    }
    printf("run");
  }

  if (debug)
   printf("Debug enabled\n");

  //Listen to connections
  socket_listen(port);

}//end of main
