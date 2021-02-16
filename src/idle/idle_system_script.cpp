#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/user.h>
#include<sys/ptrace.h>
#include<sys/wait.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>

//global data
bool debug=false;

void get_data(char *str,pid_t child,long long int addr,long long int size){
  /*
  Addr of the data written is in regs.rsi and size of data in regs.rdx
  Since, PEEKUSER returns data of size long, we have to do multiple iterations to retrive the data completely.
  sizeof(long)=8bytes [64bit system]
  */
  int i=0;
  int j=(int) (size/sizeof(long));
  union u{
    long val;
    char chars[sizeof(long)];
  }data;
  while(i<=j){
    data.val=ptrace(PTRACE_PEEKDATA,child,addr+i*sizeof(long),NULL);
    memcpy(str+i*sizeof(long),data.chars,sizeof(long));
    i++;
  }
}


void put_data(char *str,pid_t child,long long int addr,int size){
  int i=0;
  int j=(int) (size/sizeof(long));
  union u{
    long val;
    char chars[sizeof(long)];
  }data;
  while(i<=j){
    memcpy(data.chars,str+i*sizeof(long),sizeof(long));
    data.val=ptrace(PTRACE_PEEKDATA,child,addr+i*sizeof(long),data.val);
    i++;
  }
}


void manipulate(pid_t child,struct user_regs_struct *regs,int *wstatus){
   regs->orig_rax=39;//change to dummy syscall
   ptrace(PTRACE_SETREGS,child,NULL,regs);
   ptrace(PTRACE_SYSCALL,child,NULL,NULL);
   wait(wstatus);
}

int socket_connect(){
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
  address.sin_port=htons(8000);
  //Force the address on socket
  setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt));
  //bind
  bind(serverfd, (struct sockaddr * )&address,sizeof(address));
  //listen
  listen(serverfd,1);
  //accept the new connection
  new_socket=accept(serverfd,(struct sockaddr * )&address,(socklen_t *)&addrlen);
  if (debug)
    printf("Connected\n");
  return new_socket;
}

int main(int args, char *argv[]){
/*
./middleware [options[d]] -e exec_file
*/
char *exec_file;
for(int i=1;i<args;i++){
  if(strcmp(argv[i],"-d")==0){
    debug=true;
  }
  else if(strcmp(argv[i],"-e")==0){
    exec_file=argv[i+1];
    break;
  }
}
if (debug)
  printf("File to execute : %s\n",exec_file);

 //fork a child
 pid_t child=fork();

 //Child
 if (child==0){
  ptrace(PTRACE_TRACEME,child,NULL,NULL);
  execl(exec_file,exec_file+2,NULL);
}//end of child


 //Parent
 else{
  //decl(s)
  //Getting user registers value from ptrace
  struct user_regs_struct regs;
  int status,syscall,wstatus,kill=0,count=0,syscall_id=0;
  //message compose and reading value from buffer
  char *readval,*cmp_msg;
  char ret_buf[500];
  //socket
  int new_socket;


  //create a listenting socket and get a client connection for ecporting I/O
  new_socket=socket_connect();
  wait(&wstatus);//wait returned when child call traceme

  //Check if child completed execution
  while (!WIFEXITED(wstatus)){

   //Set parent to listen to child's syscalls and wait till child calls one
   status=ptrace(PTRACE_SYSCALL,child,NULL,NULL);
   wait(&wstatus);
   count+=1;

    //leaving init system calls that arent associated with the program
    if (count>29){

      //sys call enter capture
      status=ptrace(PTRACE_GETREGS,child,NULL,&regs);
      if (debug)
        printf("[EN:%d] Status:%d, orig_rax:%lld rdi:%lld rsi:%lld rdx:%lld, r10:%lld ,r8:%lld\n",count,status,regs.orig_rax,regs.rdi,regs.rsi,regs.rdx,regs.r10,regs.r8);
      syscall=(int) regs.orig_rax; //extracting the syscall called
      //syscall manipulation, send to another process and get send the return value to the child
      switch(syscall){
          //General structure
          /*
          case syscall:
            1.get buffer data if required
            2.compose message with syscall identifier and required attributes and send to client
            3.exec dummy syscall
            4.recv data from client and save return value to respectives
          */

          //error
          case -1:
            //break away from while loop
            kill=1;
            break;

          //read syscall
          case 0:
            //attributes : int fd,char *buf,size_t size)
            cmp_msg =(char *) malloc(4*sizeof(int)+3);
            memcpy(cmp_msg,&syscall_id,sizeof(int));
            memcpy(cmp_msg+sizeof(int)+1,&syscall,sizeof(int));
            memcpy(cmp_msg+2*sizeof(int)+2,&regs.rdi,sizeof(int));
            memcpy(cmp_msg+3*sizeof(int)+3,&regs.rdx,sizeof(int));
            send(new_socket,cmp_msg,4*sizeof(int)+3,0);
            free(cmp_msg);

            manipulate(child,&regs,&wstatus);

            read(new_socket,ret_buf,100);
            put_data((char *)(ret_buf+sizeof(int)+1),child,(int)regs.rsi,*(int *)ret_buf);
            ptrace(PTRACE_GETREGS,child,NULL,&regs);
            regs.rax=*(int *)ret_buf;
            ptrace(PTRACE_SETREGS,child,NULL,&regs);

            if (debug)
              printf("Syscall: Read | Attributes : [int : fd] %lld , [int : size] %lld\n",regs.rdi,regs.rdx);

            break;

          //write syscall
        	case 1:
             //attributes : int fd,char *buf,int size
             readval = (char *) malloc(regs.rdx);
             get_data(readval,child,regs.rsi,regs.rdx);

             cmp_msg = (char *) malloc(4*sizeof(int)+regs.rdx+4);
             memcpy(cmp_msg,&syscall_id,sizeof(int));
             memcpy((char *)cmp_msg+sizeof(int)+1,&syscall,sizeof(int));
             memcpy((char *)cmp_msg+2*sizeof(int)+2,&regs.rdi,sizeof(int));
             memcpy((char *)(cmp_msg+3*sizeof(int)+3),&regs.rdx,sizeof(int));
             memcpy((char *)(cmp_msg+4*sizeof(int)+4),readval,regs.rdx);
             send(new_socket,cmp_msg,4*sizeof(int)+regs.rdx+4,0);
             free(cmp_msg);


             manipulate(child,&regs,&wstatus);

             read(new_socket,ret_buf,100);
             ptrace(PTRACE_GETREGS,child,NULL,&regs);
             regs.rax=*(int *)ret_buf;//change to dummy syscall
             ptrace(PTRACE_SETREGS,child,NULL,&regs);

             if (debug)
               printf("Syscall: Write | Attributes : [int : fd] %lld , [String : buf] %s , [int : size] %lld\n",regs.rdi,readval,regs.rdx);
             break;



          //close syscall
          case 3:
            //attributes : int fd
            cmp_msg=(char *)malloc(3*sizeof(int)+2);
            memcpy(cmp_msg,&syscall_id,sizeof(int));
            memcpy(cmp_msg+sizeof(int)+1,&syscall,sizeof(int));
            memcpy(cmp_msg+2*sizeof(int)+2,&regs.rdi,sizeof(int));
            send(new_socket,cmp_msg,3*sizeof(int)+2,0);
            free(cmp_msg);

            manipulate(child,&regs,&wstatus);

            read(new_socket,ret_buf,100);
            ptrace(PTRACE_GETREGS,child,NULL,&regs);
            regs.rax=*(int *)ret_buf;
            ptrace(PTRACE_SETREGS,child,NULL,&regs);

            if (debug)
              printf("Syscall: Close | Attributes : [int : fd] %lld \n",regs.rdi);
            break;



         //fstat syscall
          case 5:
            //dont want non program syscalls calls to interfer for now
            if (regs.rdi<3)
              break;

            //attributes : int fd,struct stat *buf
            cmp_msg=(char *)malloc(3*sizeof(int)+2);
            memcpy(cmp_msg,&syscall_id,sizeof(int));
            memcpy(cmp_msg+sizeof(int)+1,&syscall,sizeof(int));
            memcpy(cmp_msg+2*sizeof(int)+2,&regs.rdi,sizeof(int));
            send(new_socket,cmp_msg,3*sizeof(int)+2,0);
            free(cmp_msg);

            manipulate(child,&regs,&wstatus);

            read(new_socket,ret_buf,500);
            put_data((char *)ret_buf,child,(int)regs.rsi,sizeof(struct stat));

            if (debug)
              printf("Syscall: fstat | Attributes : [int : fd] %lld \n",regs.rdi);
            break;



          //openat syscall
          case 257:
            //attributes : int dirfd,const char *pathname,int flags
            readval = (char *) malloc(regs.rdx);
            get_data(readval,child,regs.rsi,255); //255 max file size

            cmp_msg = (char *)malloc(sizeof(long long int)+5*sizeof(int)+255+5);
            memcpy(cmp_msg,&syscall_id,sizeof(int));
            memcpy((char *)cmp_msg+sizeof(int)+1,&syscall,sizeof(int));
            memcpy((char *)cmp_msg+2*sizeof(int)+2,&regs.rdi,sizeof(long long int));
            memcpy((char *)(cmp_msg+2*sizeof(int)+sizeof(long long int)+3),&regs.rdx,sizeof(int));
            memcpy((char *)(cmp_msg+3*sizeof(int)+sizeof(long long int)+4),&regs.r10,sizeof(int));
            memcpy((char *)(cmp_msg+4*sizeof(int)+sizeof(long long int)+5),readval,255);
            send(new_socket,cmp_msg,5*sizeof(int)+sizeof(long long int)+255+5,0);
            free(cmp_msg);

            manipulate(child,&regs,&wstatus);

            read(new_socket,ret_buf,100);
            ptrace(PTRACE_GETREGS,child,NULL,&regs);
            regs.rax=*(int *)ret_buf;
            ptrace(PTRACE_SETREGS,child,NULL,&regs);

            if (debug)
              printf("Syscall: openat | Attributes : [int : fd] %lld , [string : filename] %s , [int : flags] %lld \n ",regs.rdi,readval,regs.r10);
            break;


         default:
            //syscall exit wait
            status=ptrace(PTRACE_SYSCALL,child,NULL,NULL);
            wait(&wstatus);
           break;

     }//end of switch
     syscall_id+=1;
     status=ptrace(PTRACE_GETREGS,child,NULL,&regs);
     printf("[EX] return val[rax] : %lld\n\n",regs.rax);
   }//end of if
   else{
     status=ptrace(PTRACE_SYSCALL,child,NULL,NULL);
     wait(&wstatus);
   }
   //if -1 is the case kill from loop
   if (kill==1)
    break;

  }//end of while
  //close the client
  status=-1;
  send(new_socket,&status,sizeof(int),0);
 }//end of else
}//end of main
