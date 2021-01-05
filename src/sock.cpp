/*
  sock.cpp -- this file contains the implementation of functions to quickly and clearly set up listening and connecting sockets
  Jeremy Costanzo 2021
*/

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_aton
#include <unistd.h> // close
#include "../include/sock.h"

using namespace std;
int listen_socket(int port){
  int sock;
  sockaddr_in addr;
  sock = socket(AF_INET, SOCK_STREAM, 0); // Creates a tcp socket
  if (sock < 0){
    cout << "error opening socket\n" << endl;
    exit(1);
  }
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY; // IP address of host machine
  addr.sin_port = htons(port);
  if (bind(sock, (sockaddr *) &addr, sizeof(addr)) < 0){
    cout << "Binding error\n" << endl;
    exit(1);
  }
  listen(sock, 5); // It could have been something other than 5, but why not...
  return sock;
}

int connect_socket(int port, const char* ip){
  struct sockaddr_in addr;
  int cfd;
  cfd = socket(AF_INET, SOCK_STREAM, 0); // Creates the TCP socket
  if (cfd == -1) {
    cout << "connection socket creation error\n";
    exit(1);
  }
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  if (!inet_aton(ip, (struct in_addr *) &addr.sin_addr.s_addr)) {
    cout << "bad IP address format" << endl;
    close(cfd);
    exit(1);
  }
  if (connect(cfd, (sockaddr *) &addr, sizeof(addr)) == -1) {
    cout << "connection error\n";
    shutdown(cfd, SHUT_RDWR);
    close(cfd);
    exit(1);
  }
  return cfd;
}
