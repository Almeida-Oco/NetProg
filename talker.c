//Datagram socket client demo

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "4950"

void *getInAddr(struct sockaddr *sa){
  if (sa->sa_family == AF_INET)
    return &( ( (struct sockaddr_in*)sa)->sin_addr);
  else
    return &( ( (struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc , char *argv[] ){
  int sockfd , rv , numbytes;
  struct addrinfo hints , *servinfo , *p;

  if (argc != 3){
    printf("Wrong n of arguments! \n");
    printf("Usage: talker hostname message\n");
    return 1;
  }

  memset(&hints , 0 , sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ( (rv = getaddrinfo( argv[1] , SERVERPORT , &hints , &servinfo )) != 0){
    printf("getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  //loop results, bind to first available
  for ( p = servinfo; p != NULL ; p = p->ai_next){
    if ( (sockfd = socket(p->ai_family , p->ai_socktype , p->ai_protocol)) == -1){
      perror("talker: socket");
      return 1;
    }

    break;//bind successful
  }

  if (p == NULL){
    printf("talker: failed to bind\n");
    return 2;
  }

  if ( (numbytes = sendto(sockfd , argv[2] , strlen(argv[2]) ,0,p->ai_addr,p->ai_addrlen)) == -1){
    perror("talker: sendto");
    exit(1);
  }
  freeaddrinfo(servinfo);
  printf("talker: sent %d bytes to %s \n", numbytes ,argv[1]);

  return 0;
}
