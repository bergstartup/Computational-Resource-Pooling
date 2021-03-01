/*
Daemon script
1. Maintain IDLE/BUSY status of machine
2. UDP port 8000 listen for inter-Dameon communication
3. Membership maitainer, process completion, Job tracker and scheduler
*/


/*Imports*/
#include<vector.h>
#include<pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


/*Decl data structures*/
//data structure to store machine status
struct MachineState{
 int mid;
 string addr;
 bool idle;
}machine;

//data structure for issued process
struct SubmittedProcess{
  int id; //process id
  int port; //port assigned for that process's issue script
  string dir; //parent directory of the process
  int clones_alive; //no of clones alive
  vector<char *> addrs;
  bool is_completed; //boolean for intiamation of process completion
};

//data structure for locally executing process
struct RemoteProcess{
  int id;
  string issue_addr;
  int issue_script_port;
  string exec_path;
  bool is_completed;
};

//Data structure for controller queue and global decl for it
struct ControllerQueue{
  int job_id;
  char *job_msg;
  ControllerQueue *next;
}*Controller_head,*Controller_tail;


/*Fucntions*/
void *UDPListner(){
  int sockfd;
  char buffer[MAXLINE];
  struct sockaddr_in servaddr;
  // Creating socket file descriptor
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family    = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if ( bind(sockfd, (const struct sockaddr *)&servaddr,
          sizeof(servaddr)) < 0 )
  {
      perror("bind failed");
      exit(EXIT_FAILURE);
  }

  int len, n;

  len = sizeof(cliaddr);  //len is value/resuslt

  n = recvfrom(sockfd, (char *)buffer, MAXLINE,
              MSG_WAITALL, ( struct sockaddr *) &cliaddr,
              &len);
  buffer[n] = '\0';
}
//Main function
int main(){
  //A thread to listen udp ports
  pthread_t UDPListner_tid;
  pthread_create(&UDPListner_tid,NULL,UDPListner,NULL);

  return 0;
}
