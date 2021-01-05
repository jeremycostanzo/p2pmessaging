/*
  centralip.cpp -- This file is the implementation of the server which will keep track of the users known by the service and will tell clients how to contact another client whose username is supplied

  To register, the client sends a message :

  r client_name listening_port

  The server detects it's ip automatically and adds it to its known users map.
  
  To send a message to a peer, the client needs to know it's IP and port, so it asks the server:
  w {client name} {peer name}
  
  client name is here to prevent unregistered users from using the service, although it is currently not safe at all.
  
  Jeremy Costanzo 2021
*/

#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> // for ip addresses
#include <stdlib.h>
#include <map>
#include <unistd.h> // read
#include <sstream>
#include <string.h>
#include <signal.h>
#include "../include/sock.h"

using namespace std;

int serv_sock{-1}; // I initialised the socket at -1 to know that they are not set up at the beggining
int cli_sock{-1};
int nread{0};

void on_close(int signal)
{
  if (serv_sock >= 0){
    int status = close(serv_sock);
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
  if (argc != 2){
    cout << "Usage : " << argv[0] << " port" << endl;
    exit(1);
  }
  map<string, pair<string, int>> registered_users; // maps user name to the ip and port where they listen for incoming messages
  int port{atoi(argv[1])};
  socklen_t clilen;
  int buffersize{255};
  char buffer[buffersize];
  sockaddr_in serv_addr, cli_addr;
  serv_sock = listen_socket(port); // creates a listening socket
  if (serv_sock < 0){
    cout << "error opening socket" << endl;
    exit(1);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY; // IP address of host machine
  serv_addr.sin_port = htons(port);  
  cout << "server ip : " << inet_ntoa(serv_addr.sin_addr) << endl;
  cout << "server port : " << htons (serv_addr.sin_port) << endl;

  clilen = sizeof(cli_addr);
  signal(SIGINT, on_close); // To properly exit when closing the server
  for(;;){
    cout << "Waiting for new client\n";
    cli_sock = accept(serv_sock,(sockaddr *) &cli_addr, &clilen); // accepts the client
    if (cli_sock < 0){
      cout << "Accept error" << endl;
      exit(1);
    }
    cout << "Client connected\n"; // I used some output to debug and make the program more understandable when executed.
    cout << "ip : " << inet_ntoa(cli_addr.sin_addr) << endl;
    cout << "port : " << htons (cli_addr.sin_port) << endl;
    nread = read(cli_sock, buffer, buffersize); // I assume that I can read all at once, which may not necessarily be the case.
    
    if (nread < 0){
      cout << "Read error from client socket" << endl;
      exit(1);
    }
    if (nread == 0){
      cout << "Empty message from client" << endl;
      continue;
    }
    
    switch (buffer[0]){
    case 'r': // the client wants to register
      {
	string buff = (string) buffer;
	if (buff[nread-1] == '\n')
	  buff = buff.substr(0,nread-1);
	else
	  buff = buff.substr(0,nread);	
	vector<string> arguments; // Parsing client message
	stringstream ss(buff);
	string word;
	while (ss >> word){
	  arguments.push_back(word);
	}
	if (arguments.size() != 3){
	  const char* msg = "usage : r name listening_port\n";
	  send(cli_sock, msg, strlen(msg), 0);
	}
	else{
	  string name = arguments[1];
	  string listening_port = arguments[2];
	  map<string,pair<string,int>>::iterator it;
	  if (registered_users.find(name) != registered_users.end()){
	    const char* msg = "This name already exists\n";
	    send(cli_sock, msg, strlen(msg), 0);
	  }
	  else
	    {
	      registered_users[name] = make_pair(((string)inet_ntoa(cli_addr.sin_addr)), stoi(listening_port));
	      const char * msg = "You are now registered\n";
	      cout << "registered as " << name << endl; 
	      send(cli_sock, msg, strlen(msg), 0);
	    }
	}
      }
      break;
      
    case 'w': // The client wants to know how to contact a user
      {
	string buff = (string) buffer;
	if (buff[nread-1] == '\n')
	  buff = buff.substr(0,nread-1);
	else
	  buff = buff.substr(0,nread);
	
	vector<string> arguments;
	stringstream ss(buff);
	string word;
	while (ss >> word){
	  arguments.push_back(word);
	}
	if (arguments.size() != 3){
	  const char* msg = "usage : w your_id target_id\n";
	  send(cli_sock, msg, strlen(msg), 0);
	}
	else{
	  string asking_user = arguments[1];
	  string target_user = arguments[2];
	  cout << "request : " << asking_user << " " << target_user << endl;
	  if (registered_users.find(asking_user) == registered_users.end()){
	    const char* msg = "You cannot perform such request without be known by the service, use r first\n";
	    send(cli_sock, msg, strlen(msg), 0);
	  }
	  else{
	    cout << "reqname : " << target_user << endl;
	    map<string,pair<string,int>>::iterator it = registered_users.find(target_user);
	    if (it != registered_users.end()){
	      stringstream msgss{string()};
	      msgss << it->second.first << " " << it->second.second;
	      string msgs{msgss.str()};
	      cout << "sending : " << msgs << endl;
	      const char * msg = msgs.c_str(); //ip port
	      send(cli_sock, msg, strlen(msg), 0);
	    }
	    else{
	      const char * msg = "Unknown";
	      send(cli_sock, msg, strlen(msg), 0);
	    }
	  }
	}
      }
      break;
    default:
      const char * msg = "Wrong use of the service, those requests are accepted :\nr : to register\nw your_name target_name : to ask how to contact the target";
      send(cli_sock, msg, strlen(msg), 0);      
      break;
    }
    if (close(cli_sock) == -1){
      cout << "error closing socket" << endl;
      exit(1);
    }
  }
  int status = close(serv_sock);
  if (status == -1){
    cout << "error closing socket" << endl;
  }
  return 0;
}
