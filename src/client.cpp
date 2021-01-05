/*OB
  client.cpp -- Client communicates with other clients, centralip and the User interface

  Jeremy Costanzo 2021
*/

#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>
#include <fstream>
#include <vector>
#include <signal.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_ntoa
#include <unistd.h> // close
#include <cassert>
#include <sstream>
#include <string.h> // strlen
#include "../include/sock.h" //listen_socket, connect_socket
#include <signal.h>
#define CLIENT 0
#define USER_INTERFACE 1

using namespace std;

int ux_fd{-1};
int client_fd{-1};
int nfd{2};
struct pollfd *pfds;

void on_close(int signal)
{
  if (nfd > 0){
    if (ux_fd >= 0){
      int status = close(ux_fd);
      if (status == -1){
	cout << "error closing socket" << endl;
	exit(1);
      }
    }
    if (client_fd >= 0){
      int status = close(client_fd);
      if (status == -1){
	cout << "error closing socket" << endl;
	exit(1);
      }
    }
    delete[] pfds;   
  }
  cout << endl << "Closed successfully" << endl;
  exit(0);
}

int main(int argc, char** argv)
{
  //input processing
  if (argc != 6){
    cout << "Usage : " << argv[0] << " routeur_ip" << " routeur_port " << "UX_ip " << "UX_port " << "Client_port" <<endl;
    exit(1);
  }
  const char* routeur_ip = argv[1];
  int routeur_port = atoi(argv[2]);
  const char* UX_ip = argv[3];
  int UX_port = atoi(argv[4]);
  int Client_port = atoi(argv[5]);

  // Variables declaration
  int timeout = -1;
  int buffersize{255};
  char buffer[buffersize];
  int nread{0};
  stringstream to_deliver_ux{string()};
  
  // Registering to the routing service
  cout << "registering...\n";
  int routeur_fd = connect_socket(routeur_port, routeur_ip);
  string name;
  cout << "enter your name : ";
  cin >> name; // I used cin to make the projet easier to test, but it could have also been passed by the UX
  const char* msg = ("r " + name + " " + to_string(Client_port)).c_str();
  send(routeur_fd, msg, strlen(msg), 0);
  nread = read(routeur_fd, buffer, buffersize);
  string buff = (string) buffer;
  if (buff.at(nread-1) == '\n')
    buff = buff.substr(0,nread-1);
  else
    buff = buff.substr(0,nread);
  cout << "from routing server : " << buff << endl;
  close(routeur_fd);
  
  // Creating the file descriptors for the other client and the user interface
  ux_fd = connect_socket(UX_port,UX_ip); // To connect to the UX, not listen for security reasons
  client_fd = listen_socket(Client_port); // Listens for incoming client connections
  pfds = (struct pollfd*) malloc(nfd * sizeof(*pfds)); // I will need two : one to listen for incoming connections from other clients, and one to listen for the UX (when the user wants to send a message)
  if (pfds == NULL){
    cout << "malloc" << endl;
    exit(1);
  }
  pfds[USER_INTERFACE].fd = ux_fd;
  pfds[CLIENT].fd = client_fd;
  
  signal(SIGINT, on_close);
  
  // We are ready to begin
  for(;;){
    int status{0};
    pfds[USER_INTERFACE].events = POLLIN | POLLOUT;
    pfds[USER_INTERFACE].revents = 0;
    pfds[CLIENT].events = POLLIN;
    pfds[CLIENT].revents = 0;
    //cout << "polling" << endl;
    status = poll(pfds, nfd, timeout);
    if (status <= 0){
      cout << "negative status" << endl;
      continue;}
    assert(status <= nfd);
    // Peer Client processing
    if (pfds[CLIENT].fd == -1){
      cout << "client socket closed unexpectedly\n";
      exit(1);
    }
    if (pfds[CLIENT].revents){
      const int clientrevents = pfds[CLIENT].revents;
      pfds[CLIENT].revents = 0;

      // The client wants to send a message
      if (clientrevents & POLLIN){
	cout << "client pollin\n";
	sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int cli_sock = accept(pfds[CLIENT].fd,(sockaddr *) &cli_addr, &clilen);
	if (cli_sock < 0){
	  cout << "Accept error" << endl;
	  continue;
	}
	cout << "peer ip : " << inet_ntoa(cli_addr.sin_addr) << endl;
	cout << "peer port : " << htons (cli_addr.sin_port) << endl;
	stringstream message{string()};
	nread = 0;
	do{
	  nread = read(cli_sock, buffer, buffersize);
	  for(int i = 0; i < nread;++i){
	    message << buffer[i];
	  }
	}
	while(buffer[nread - 1] != '\n' or buffer[nread-2] != '\n');
	close(cli_sock);
	if (message.str().at(message.str().size()-1) != '\n' or message.str().at(message.str().size()-1) != '\n'){
	  cout << "incorrect message ending : " << message.str().at(message.str().size()-1) << endl;
	}
	string delivering = message.str().substr(0, message.str().size() -1) + (string) "\n";	  
	to_deliver_ux << delivering;
      }

      // The client wants to do something else
      else
	cout << "incorrect revent from peer\n";
    }
    
    // UX processing
    if (pfds[USER_INTERFACE].fd == -1){
      cout << "user interface socket closed unexpectedly\n";
      exit(1);
    }
    if (pfds[USER_INTERFACE].revents) {
      const int uxrevents = pfds[USER_INTERFACE].revents;
      pfds[USER_INTERFACE].revents = 0;

      // The user wants to send a message
      if (uxrevents & POLLIN){
	cout << "user pollin\n";
	int nread = 0;
	stringstream message{string()};
	do{
	  nread = read(pfds[USER_INTERFACE].fd, buffer, buffersize);
	  for(int i = 0; i < nread;++i){
	    message << buffer[i];
	  }
	}
	while(buffer[nread - 2] != '\n' or buffer[nread -1] != '\n'); // '\n\n' indicates the end of the message
	string recipient;
	message >> recipient;
	cout << "User wants to send message to " << recipient << endl;
	string content;
	getline(message,content);
	routeur_fd = connect_socket(routeur_port, routeur_ip); // we need to ask the routing service how to relay the message we recieved;
	stringstream msgss {""};
	msgss << "w " << name << " " << recipient;
	string msgs {msgss.str()};
	cout << msgs.c_str() << endl;
	const char* msg = msgs.c_str();
	cout << msg << endl;
	cout << "asking routeur " << msg << endl;
	send(routeur_fd, msg, strlen(msg), 0);
	nread = read(routeur_fd, buffer, buffersize);
	close(routeur_fd);
	string buff = (string) buffer;
	buff = buff.substr(0,nread);
	cout << "from routing server : " << buff << endl;
	if (buff == (string)"Unknown"){
	  const char* msg = "Unknown recipient";
	  send(pfds[USER_INTERFACE].fd, msg, strlen(msg), 0);
	}
	else{
	  stringstream received_address{buff};
	  string recipient_ips;
	  string recipient_ports;
	  received_address >> recipient_ips >> recipient_ports;
	  const char* recipient_ip = recipient_ips.c_str();
	  int recipient_port = stoi(recipient_ports);
	  cout << "recipient_ip " << recipient_ip << " recipient_port " << recipient_port << endl;
	  int recipient_fd = connect_socket(recipient_port, recipient_ip);
	  int nbsent{0};
	  stringstream message_contentss {""};
	  message_contentss << name << " :\n" << content << "\n\n";
	  string message_contents = message_contentss.str();
	  const char* message_content = message_contents.c_str();
	  cout << message_content << endl;
	  while(nbsent < strlen(message_content)){
	    nbsent += send(recipient_fd, message_content + nbsent, strlen(message_content) - nbsent, 0);
	  }
	  close(recipient_fd);
	}
      }

      //The user wants to consult their messages
      if (uxrevents & POLLOUT){
	string tosendstr = to_deliver_ux.str();
	if (tosendstr.size() >0){
	  cout << "user pollout and data is ready to be sent\n";
	  const char* to_send {tosendstr.c_str()};
	  send(pfds[USER_INTERFACE].fd, to_send, strlen(to_send), 0); // sends ux stringstream content
	  cout << "sent : " << to_send << endl;
	  to_deliver_ux.str(string()); // empties the ux stringstream
	}
      }
    }    
  }
  return 0;
}
