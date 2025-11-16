#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKLOG 10  // how many pending connections queue will hold

static void sigchld_handler(int s) {
  (void)s;

  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0) {};
  errno = saved_errno;
}

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
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
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
      perror("socket");
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

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  printf("server: waiting for connections...\n");

  while (true) {
    int new_fd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    char s[INET6_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr),
              s, sizeof s);
    printf("server: got connection from %s\n", s);
    pid_t child_pid = fork();
    if (child_pid == 0) {  // child
      close(sockfd);
      if (send(new_fd, "Hello World!", 12, 0) == -1) {
        perror("send");
      }
      close(new_fd);
      exit(EXIT_SUCCESS);
    } else if (child_pid < 0) {
      perror("fork");
    }
    close(new_fd);
  }

  return EXIT_SUCCESS;
}
