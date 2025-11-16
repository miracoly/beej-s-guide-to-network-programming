#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "arpa/inet.h"
#include "netinet/in.h"
#include "sys/socket.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: showip hostname\n");
    return 1;
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo* res;
  int status;
  if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 2;
  }

  printf("IP addresses for %s:\n\n", argv[1]);

  struct addrinfo* p;
  for (p = res; p != NULL; p = p->ai_next) {
    void* address;
    char* ip_version;
    struct sockaddr_in* ipv4;
    struct sockaddr_in6* ipv6;

    if (p->ai_family == AF_INET) {  // IPv4
      ipv4 = (struct sockaddr_in*)p->ai_addr;
      address = &(ipv4->sin_addr);
      ip_version = "IPv4";
    } else {  // IPv6
      ipv6 = (struct sockaddr_in6*)p->ai_addr;
      address = &(ipv6->sin6_addr);
      ip_version = "IPv6";
    }

    // convert IP to string
    char ip_str[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, address, ip_str, sizeof(ip_str));
    printf("  %s: %s\n", ip_version, ip_str);
  }

  freeaddrinfo(res);

  return 0;
}
