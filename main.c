#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 50001
#define LISTEN_BACKLOG 5

void* handleConnection(void* client_fd_ptr)
{
  int client_fd = *(int*)client_fd_ptr;
  free(client_fd_ptr);

  char buffer[1024];
  int bytesRead;

  while ((bytesRead = read(client_fd, buffer, sizeof(buffer))) > 0)
  {
    buffer[bytesRead] = '\0'; // Null-terminate the buffer
    printf("Received: %s\n", buffer);
    write(client_fd, buffer, bytesRead);
  }

  close(client_fd);
  return NULL;
}

int main(int argc, char* argv[])
{
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in socket_address;
  memset(&socket_address, '\0', sizeof(socket_address));
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(PORT);

  int returnval;

  returnval = bind(
      socket_fd, (struct sockaddr *)&socket_address, sizeof(socket_address));

  returnval = listen(socket_fd, LISTEN_BACKLOG);

  struct sockaddr_in client_address;
  socklen_t client_address_len = sizeof(client_address);

  while (1)
  {
    int* client_fd_ptr = malloc(sizeof(int));
    *client_fd_ptr = accept(socket_fd, (struct sockaddr *)&client_address, &client_address_len);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handleConnection, client_fd_ptr);
    pthread_detach(thread_id);
  }

  close(socket_fd);
  return 0;
}
