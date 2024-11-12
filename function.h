#include <pthread.h>

void send_response(int client_fd, const char* status, const char* content_type, const char* body, int body_length);
void handle_static(int client_fd, const char* path);
void handle_stats(int client_fd);
void handle_calc(int client_fd, const char* query);
void* handle_connection(void* client_fd_ptr);
