
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490" //Port users will connect to
#define BACKLOG 15 //How many pending connections queue will store

void sigchldHandler(int s){
  //save current errno
  int saved_errno = errno;

  while(waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
}

void *getInAddr(struct sockaddr *sa){
  if (sa->sa_family == AF_INET)
    return &( ( (struct sockaddr_in*)sa)->sin_addr);
  else
    return &( ( (struct sockaddr_in6*)sa)->sin6_addr);
}

int main(){
  int sockfd , new_fd , rv, yes =1; //Listen on sockfd, new connect on new_fd
  struct addrinfo hints , *servinfo , *p;
  struct sockaddr_storage their_addr; //Information of connector's
  socklen_t sin_size;
  struct sigaction sa;
  char s[INET6_ADDRSTRLEN];

  memset(&hints , 0 , sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ( (rv = getaddrinfo(NULL , PORT , &hints , &servinfo) ) != 0){
    printf("getaddrinfo: %s \n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
        perror("server: socket");
        continue;
      }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
      sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
      }
      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("server: bind");
        continue;
      }
      break;
  }
  printf("Bound to addr: %u\n", ((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr);
  freeaddrinfo(servinfo);
  if (p == NULL ){  //Reached end of list and no connection available
    printf("server: failed to bind! \n");
    exit(1);
  }
  if ( listen(sockfd, BACKLOG )  == -1) {
    perror("listen");
  }

  sa.sa_handler = sigchldHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa , NULL) == -1){
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections... \n");

  while(1){ //Main accept loop, where it waits for connections
    sin_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if ( new_fd == -1){
      perror("accept");
      continue;
    } 
    inet_ntop(their_addr.ss_family , getInAddr( (struct sockaddr *)&their_addr), s , sizeof(s));
    printf("server: got connection from %s \n", s);

    if (!fork()){ //child proccess
      close(sockfd); //child does not need listener
      if (send(new_fd , "Hello Client!" , 13 , 0) == -1) //send msg size 13
        perror("send");
      close(new_fd);
      exit(0);
    }
  }

  return 0;
}
