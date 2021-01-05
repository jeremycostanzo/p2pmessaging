#ifndef SOCK_H
#define SOCK_H

int listen_socket(int port);
int connect_socket(int port, const char* ip);

#endif
