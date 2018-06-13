#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/socket.h>

// Main thread
int main(int argc, char* argv[]) {

  int portnumber = 4000;
  std::string ipaddress;

  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-p portnumber   connect to TCP/IP server on given port" << std::endl;
      std::cout << "-a IP address   connect to TCP/IP server at given address" << std::endl;
      return 0;
    } else if(!strcmp(argv[i], "-p")) {
      portnumber = std::stoi(argv[++i]);
      std::cout << "Conf: control server on port " << portnumber << std::endl;
      continue;
    } else if(!strcmp(argv[i], "-a")) {
      ipaddress = std::string(argv[++i]);
      std::cout << "Conf: control server at address " << ipaddress << std::endl;
      continue;
    } else {
      std::cout << "Unrecognized option: " << argv[i] << std::endl;
    }
  }

  // Configure Socket and address
  struct sockaddr_in address;

  address.sin_family = AF_INET;
  address.sin_port = htons(portnumber);
  inet_aton(ipaddress.c_str(), &(address.sin_addr));
  int mysocket = socket(AF_INET, SOCK_STREAM, 0);

  std::stringstream ss;
  ss << inet_ntoa(address.sin_addr);

  std::cout << "Connecting to " << ss.str() << std::endl;
  int retval = ::connect(mysocket, (struct sockaddr*)&address, sizeof(address));

  if(retval == 0) {
    std::cout << "Connection to server at " << ss.str() << " established" << std::endl;
  } else {
    std::cout << "Connection to server at " << ss.str() << " failed, errno " << errno << std::endl;
  }

  while(1) {
    std::cout << "Command: ";
    std::string command = "";
    getline(std::cin, command);

    if(command == "quit")
      break;
    send(mysocket, command.c_str(), strlen(command.c_str()), 0);

    std::cout << "Awaiting reply from server at " << ss.str() << " to command: " << command << std::endl;

    int buffersize = 1024;
    char buffer[1024];
    int msglen = recv(mysocket, buffer, buffersize, 0);
    buffer[msglen] = '\0';
    std::cout << "Message received from server at " << ss.str() << ": " << std::string(buffer) << std::endl;
  }

  return 0;
}
