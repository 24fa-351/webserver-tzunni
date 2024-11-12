#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "function.h"

#define DEFAULT_PORT 80
#define LISTEN_BACKLOG 5

int main(int argc, char *argv[])
{
  int port = DEFAULT_PORT;
  if (argc == 3 && strcmp(argv[1], "-p") == 0)
  {
    port = atoi(argv[2]);
  }

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in socket_address;
  memset(&socket_address, 0, sizeof(socket_address));
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(port);

  bind(socket_fd, (struct sockaddr *)&socket_address, sizeof(socket_address));
  listen(socket_fd, LISTEN_BACKLOG);

  while (1)
  {
    int client_fd = accept(socket_fd, NULL, NULL);
    int *client_fd_ptr = malloc(sizeof(int));
    *client_fd_ptr = client_fd;
    pthread_t thread;
    pthread_create(&thread, NULL, handle_connection, client_fd_ptr);
    pthread_detach(thread);
  }

  close(socket_fd);
  return 0;
}
