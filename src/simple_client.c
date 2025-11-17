#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_DATASIZE 100

static void* get_in_addr(struct sockaddr* sa) {
  if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
  else return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: simple_client HOSTNAME PORT\n");
    return EXIT_FAILURE;
  }
  const char* hostname = argv[1];
  const char* port = argv[2];

  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int status;
  struct addrinfo* server_info;
  if ((status = getaddrinfo(hostname, port, &hints, &server_info)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return EXIT_FAILURE;
  }

  struct addrinfo* p;
  int sockfd;
  char s[INET6_ADDRSTRLEN];
  for (p = server_info; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s,
              sizeof s);
    printf("client: attempting connection to %s\n", s);

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("client: connect");
      close(sockfd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return EXIT_FAILURE;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s,
            sizeof s);
  printf("client: connected to %s\n", s);

  freeaddrinfo(server_info);

  int numbytes;
  char buf[MAX_DATASIZE];
  if ((numbytes = recv(sockfd, buf, (sizeof buf) - 1, 0)) == -1) {
    perror("recv");
    close(sockfd);
    return EXIT_FAILURE;
  };

  buf[numbytes] = '\0';
  printf("client: received '%s'\n", buf);

  close(sockfd);
  return EXIT_SUCCESS;
}
