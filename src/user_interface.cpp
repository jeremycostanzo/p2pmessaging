/*
  user_interface.cpp -- This is a simple user interface, it only communicates with the client.
  Only two commands are available :
  fetch
  to fetch messages, although if fetch is used and no message is available the program will block in its current implementation
  
  received messages include the sender's name
  
  send {name} {message}
  to send 'message' to 'name'

  exit or quit to stop the program
  
  Jeremy Costanzo 2021
*/

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include "../include/sock.h"
#include <sstream>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

int ui_sock{-1};
int cli_sock{-1};
int nread{0};

void on_close(int signal) // To cleanup when closing
{
  if (ui_sock >= 0){
    int status = close(ui_sock);
    if (status == -1){
      cout << "error closing socket" << endl;
      exit(1);
    }
  }
  if (cli_sock >= 0){
    int status = close(cli_sock);
    if (status == -1){
      cout << "error closing socket" << endl;
      exit(1);
    }
  }
  cout << endl << "Closed successfully" << endl;
  exit(0);
}

int main(int argc, char** argv)
{
  // input processing
  if (argc != 2){
    cout << "Usage : " << argv[0] << " port" << endl;
    exit(1);
  }
  int port{atoi(argv[1])};
  int ui_sock, cli_sock, nread;
  socklen_t clilen;
  int buffersize{255};
  char buffer[buffersize];
  sockaddr_in ui_addr, cli_addr;
  ui_sock = listen_socket(port); // creates a listening socket
  if (ui_sock < 0){
    cout << "error opening socket" << endl;
    exit(1);
  }

  ui_addr.sin_family = AF_INET;
  ui_addr.sin_addr.s_addr = INADDR_ANY; // IP address of host machine
  ui_addr.sin_port = htons(port);  
  clilen = sizeof(cli_addr);
  cout << "waiting for the client to connect" << endl;
  signal(SIGINT, on_close);
  cli_sock = accept(ui_sock,(sockaddr *) &cli_addr, &clilen); // accepts the client
  cout << "accepted client\n";
  string command;
  for(;;){
    cout << "Enter command" << endl;
    cin >> command;
    if (command == (string) "fetch"){
      nread = 0;
      stringstream message{string()};
      cout << "waiting for client to answer\n";
      do{
  	nread = read(cli_sock, buffer, buffersize);
  	for(int i = 0; i < nread;++i){
  	  message << buffer[i];
  	}
      }
      while(buffer[nread - 1] != '\n' or buffer[nread - 2] != '\n'); // '\n\n' indicates the end of the message
      if (message.str().at(message.str().size()-1) != '\n' or message.str().at(message.str().size()-1) != '\n'){
  	cout << "incorrect message ending : " << message.str().at(message.str().size()-1) << endl;
      }
      string received = message.str().substr(0, message.str().size() -1) + (string) "\n";
      cout << received;
    }
    else if (command == (string)"send"){
      string recipient, content;
      cin >> recipient;
      getline(cin,content);
      string tosend{recipient + (string)" " + content + (string)"\n\n"};
      const char* message {tosend.c_str()};
      send(cli_sock, message, strlen(message), 0);
    }
    else if (command == (string)"exit" or command == (string)"quit"){
      on_close(SIGTERM);
    }
    else{
      cout << "possible commands are fetch or send {recipient} {message}\n\n" ;
    }
  }
  if (ui_sock >= 0){
    int status = close(ui_sock);
    if (status == -1){
      cout << "Error closing socket" << endl;
    }
  }
  if (cli_sock >= 0){
    int status = close(cli_sock);
    if (status == -1){
      cout << "Error closing socket" << endl;
    }
  }
  return 0;
}
