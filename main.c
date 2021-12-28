#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define EXIT_NOT_OK 2
#define EXIT_MAN 1
#define QUERY 20
#define MAX_BUF_SIZE 30000
#define sockaddr_in struct sockaddr_in
#define sockaddr struct sockaddr
#define forever ;;

int create_tcp_socket();
sockaddr_in* get_tcp_addr(int);
int handle_conns(int, sockaddr*, socklen_t*);
void usage();

int main(int argc, char **argv) {
  int soc, in_soc, addr_len, port;
  
  if (argc < 3) {
    usage(); 
    return EXIT_MAN;
  }

  if (strcmp(argv[1], "-p") != 0) {
    usage();
    return EXIT_MAN;
  }

  if ((port = atoi(argv[2])) == 0) {
    usage();
    return EXIT_MAN;
  }

  if ((soc = create_tcp_socket()) < 0) {
    perror("cannot create socket");
    return EXIT_NOT_OK;
  }

  sockaddr_in* addr = get_tcp_addr(port);
  addr_len = sizeof(*addr);

  if (bind(soc, (sockaddr*)addr, addr_len) < 0) {
    perror("cannot bind tcp socket");
    goto free_addr;
  }

  if (listen(soc, QUERY) < 0) {
    perror("failed to listen on socket");
    goto free_addr;
  }

  char* hello = "hello from server";
  
  printf("tcp server started on: [:%d]\n", port);
  if (handle_conns(soc, (sockaddr*)addr, (socklen_t*)&addr_len) < 0) {
    perror("handle server connections");
  }

  free_addr:
    free(addr);

  return 0;
}

int create_tcp_socket() {
  return socket(AF_INET, SOCK_STREAM, 0);
}

sockaddr_in* get_tcp_addr(int port) {
  sockaddr_in* addr = (sockaddr_in*)malloc(sizeof(sockaddr_in));

  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = htonl(INADDR_ANY);
  addr->sin_port = htons(port);
  memset(addr->sin_zero, '\0', sizeof(addr->sin_zero));

  return addr;
}

int handle_conns(int soc, sockaddr* addr, socklen_t* addr_len) {
  int in_soc;
  long count_bytes;
  char* hello = "hello from server";

  for (forever) {
    char buf[MAX_BUF_SIZE] = {0};
    
    if ((in_soc = accept(soc, addr, addr_len)) < 0) {
      perror("accept connection");
      return -1;
    }

    count_bytes = read(in_soc, buf, MAX_BUF_SIZE);
    printf("got message from client: %s\n", buf);
    
    write(in_soc, hello, strlen(hello));
    
    close(in_soc);
  }

  return 0;
};

void usage() {
  printf("usage: ./server -p 8080\n");
}
