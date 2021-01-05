# Peer to peer messaging app in C++

This repo contains a peer to peer app project.

# Quick start

To compile the project, use make.
You may also need to make the scripts executable : chmod +x *.sh
then you can open 5 shells to try to send messages between them.
In the first one, use ./quickcentralip.sh
In the second one, use ./quickui.sh
In the third one, use ./quickclient.sh and type a name : name1
In the fourth one, use ./quickui1.sh
In the fifth one, use ./quickclient1.sh and type a different name : name2

Then in the second shell (ui), you can type : send name2 {some message} and press enter
In the fourth shell, you can type fetch and press enter.
Then you can type send name1 {some answer} and press enter in the fourth shell (ui1)
And type fetch in the second shell

You won't be able to send and receive messages from other computers using those scripts, as your local ip is supplied.
If you want to do so, you will need to use the .out's and supply your network ip to the client.
You can remove any binary files with make clean

# Executables

Three executables are produced : Centralip.out, User\_interface.out and Client.out
you can simply type ./{one of the executables} and press enter to see what arguments it needs to be supplied, but I will describe them here.

- Centralip.out:
  Usage : ./Centralip.out port
  port is the port that Centralip.out uses to listen for incoming tcp connections.

- User\_interface.out:
  Usage : ./User\_interface.out port
  port is the port that User\_interface.out uses to listen for a connection from a client.
  
- Client.out:
  Usage : ./Client.out routeur\_ip routeur\_port UX\_ip UX\_port Client\_port
  routeur\ip is an ipv4 address where the executable Centralip.out is running
  routeur\_port is the port used by Centralip.out
  UX\_ip is the ipv4 address where User\_interface.out is running
  UX\_port is the port used by User\_interface.out
  Client\_port is the port that the client uses to listen for other client connecting

You can terminate any of the programs using control c.
For further information about what each executable does, you can see their source in src.

# Known issues

The user interface will block if fetch is used and there are no messages.
The client will fail if send {some user} {some message} is used by the user interface and Central\_ip does not know {some user}
The client will fail if {name} is used during register and name is already known by Central\_ip
