#include "function.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>

#define STATIC_DIR "./static"

int request_count = 0;
int total_received_bytes = 0;
int total_sent_bytes = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_response(int client_fd, const char *status, const char *content_type, const char *body, int body_length)
{
  char header[1024];
  int header_length = snprintf(header, sizeof(header),
                               "HTTP/1.1 %s\r\n"
                               "Content-Type: %s\r\n"
                               "Content-Length: %d\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               status, content_type, body_length);
  write(client_fd, header, header_length);
  write(client_fd, body, body_length);
  pthread_mutex_lock(&stats_mutex);
  total_sent_bytes += header_length + body_length;
  pthread_mutex_unlock(&stats_mutex);
}

void handle_static(int client_fd, const char *path)
{
  char filepath[1024];
  snprintf(filepath, sizeof(filepath), "%s%s", STATIC_DIR, path + 7); // Skip "/static"
  FILE *file = fopen(filepath, "rb");
  if (file == NULL)
  {
    send_response(client_fd, "404 Not Found", "text/plain", "File not found", 13);
    return;
  }

  fseek(file, 0, SEEK_END);
  int file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *file_content = malloc(file_size);
  fread(file_content, 1, file_size, file);
  fclose(file);

  send_response(client_fd, "200 OK", "application/octet-stream", file_content, file_size);
  free(file_content);
}

void handle_stats(int client_fd)
{
  char body[1024];
  pthread_mutex_lock(&stats_mutex);
  int body_length = snprintf(body, sizeof(body),
                             "<html><body>"
                             "<h1>Server Stats</h1>"
                             "<p>Requests: %d</p>"
                             "<p>Received Bytes: %d</p>"
                             "<p>Sent Bytes: %d</p>"
                             "</body></html>",
                             request_count, total_received_bytes, total_sent_bytes);
  pthread_mutex_unlock(&stats_mutex);
  send_response(client_fd, "200 OK", "text/html", body, body_length);
}

void handle_calc(int client_fd, const char *query)
{
  int a = 0, b = 0;
  sscanf(query, "a=%d&b=%d", &a, &b);
  char body[1024];
  int body_length = snprintf(body, sizeof(body), "<html><body><h1>Result: %d</h1></body></html>", a + b);
  send_response(client_fd, "200 OK", "text/html", body, body_length);
}

void *handle_connection(void *client_fd_ptr)
{
  int client_fd = *(int *)client_fd_ptr;
  free(client_fd_ptr);

  char buffer[4096];
  int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
  if (bytes_read <= 0)
  {
    close(client_fd);
    return NULL;
  }

  buffer[bytes_read] = '\0';
  pthread_mutex_lock(&stats_mutex);
  request_count++;
  total_received_bytes += bytes_read;
  pthread_mutex_unlock(&stats_mutex);

  char method[16], path[1024], protocol[16];
  sscanf(buffer, "%s %s %s", method, path, protocol);

  if (strcmp(method, "GET") != 0)
  {
    send_response(client_fd, "405 Method Not Allowed", "text/plain", "Method not allowed", 18);
  }
  else if (strncmp(path, "/static/", 8) == 0)
  {
    handle_static(client_fd, path);
  }
  else if (strcmp(path, "/stats") == 0)
  {
    handle_stats(client_fd);
  }
  else if (strncmp(path, "/calc?", 6) == 0)
  {
    handle_calc(client_fd, path + 6); // Skip "/calc?"
  }
  else
  {
    send_response(client_fd, "404 Not Found", "text/plain", "Not found", 9);
  }

  close(client_fd);
  return NULL;
}
