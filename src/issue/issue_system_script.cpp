#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


bool debug=false;




int socket_connect(){
  int sock=0;
  struct sockaddr_in serv_addr;
  sock=socket(AF_INET,SOCK_STREAM,0);
  serv_addr.sin_family=AF_INET;
  serv_addr.sin_port=htons(8000);
  inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr);
  connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
  return sock;
}

int main(int args,char *argv[])
{

  char recv_buf[100];
  int *syscallid,*syscall,*size,*fd,*flag,*mode;
  long long int *dirfd;
  char *msg,buf[100],*makemsg;
  int ret,sock;

  struct stat send_stat;

  for(int i=1;i<args;i++){
    if(strcmp(argv[i],"-d")==0)
      debug=true;
  }

  //talk to local Dameon and get port to listen
  int sockfd;
  char buffer[1024];
  int n, len;
  char *init_msg = "Hello from client";
  struct sockaddr_in     servaddr;

  // Creating socket file descriptor
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(8000);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");


  sendto(sockfd, (const char *)init_msg, strlen(init_msg),
      MSG_CONFIRM, (const struct sockaddr *) &servaddr,
          sizeof(servaddr));
  n = recvfrom(sockfd, (char *)buffer, 1024,
              MSG_WAITALL, (struct sockaddr *) &servaddr,
              &len);
  close(sockfd);

  //connect with idle node(s)
  sock=socket_connect();
  read(sock,recv_buf,100);
  syscall = (int *)recv_buf;
  int syscall_count=0;
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
   syscall_count+=1;
   read(sock,recv_buf,100);
   syscall = (int *)recv_buf;
 }//end of while
}//end of main
