#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BUFLEN 100

static void* get_in_addr(struct sockaddr* sa) {
  if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
  else return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: simple_server PORT\n");
    return EXIT_FAILURE;
  }

  const char* port = argv[1];

  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  int status;
  struct addrinfo* server_info;
  if ((status = getaddrinfo(NULL, port, &hints, &server_info)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return EXIT_FAILURE;
  }

  struct addrinfo* p;
  int sockfd;
  for (p = server_info; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) ==
        -1) {
      perror("setsockopt");
      close(sockfd);
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("bind");
      close(sockfd);
      continue;
    }

    break;  // successfully bound
  }

  freeaddrinfo(server_info);

  if (!p) {
    fprintf(stderr, "server: failed to bind socket\n");
    exit(EXIT_FAILURE);
  }

  printf("server: waiting for connections...\n");

  int numbytes;
  struct sockaddr_storage their_addr;
  socklen_t sin_size = sizeof their_addr;
  char buf[MAX_BUFLEN];
  if ((numbytes = recvfrom(sockfd, buf, MAX_BUFLEN - 1, 0,
                           (struct sockaddr*)&their_addr, &sin_size)) == -1) {
    perror("recvfrom");
    return EXIT_FAILURE;
  }

  char s[INET6_ADDRSTRLEN];
  printf("listener: got packet from %s\n",
         inet_ntop(their_addr.ss_family,
                   get_in_addr((struct sockaddr*)&their_addr), s, sizeof s));
  printf("listener: packet is %d bytes long\n", numbytes);
  buf[numbytes] = '\0';
  printf("listener: packet contains \"%s\"\n", buf);

  close(sockfd);

  return EXIT_SUCCESS;
}
