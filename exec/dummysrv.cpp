#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

#include "utils/exceptions.hpp"
#include "utils/log.hpp"

using namespace caribou;

int my_socket, new_socket;

/**
 * @brief Clean the environment when closing application
 */
void clean() {
  Log::finish();
}

// Main thread
int main(int argc, char* argv[]) {
  // Add cout as the default logging stream
  Log::addStream(std::cout);

  // TCP/IP server variables
  int portnumber = 4000;
  struct sockaddr_in address;
  socklen_t addrlen;
  int bufsize = 4096;
  char* buffer = (char*)malloc(bufsize);
  std::string rundir;

  // Quick and hacky cli arguments reading:
  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-r portnumber  start TCP/IP server listening on given port" << std::endl;
      clean();
      return 0;
    } else if(!strcmp(argv[i], "-v")) {
      try {
        LogLevel log_level = Log::getLevelFromString(std::string(argv[++i]));
        Log::setReportingLevel(log_level);
      } catch(std::invalid_argument& e) {
        LOG(ERROR) << "Invalid verbosity level \"" << std::string(argv[i]) << "\", ignoring overwrite";
      }
      continue;
    } else if(!strcmp(argv[i], "-r")) {
      portnumber = std::stoi(argv[++i]);
      LOG(INFO) << "Starting dummy control server, listening on port " << portnumber;
      continue;
    } else {
      std::cout << "Unrecognized option: " << argv[i] << std::endl;
    }
  }

  // Create a socket and start listening for commands from run control
  if((my_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0) {
    LOG(INFO) << "Socket created";
  }

  // Set up which port to listen to
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(portnumber);

  // Bind the socket
  if(bind(my_socket, (struct sockaddr*)&address, sizeof(address)) == 0) {
    LOG(INFO) << "Binding Socket";
  } else {
    LOG(FATAL) << "Socket binding failed";
    throw CommunicationError("Socket binding failed");
  }

  // Wait for communication from the run control
  listen(my_socket, 3);
  addrlen = sizeof(struct sockaddr_in);

  // Wait for client to connect (will block until client connects)
  new_socket = accept(my_socket, (struct sockaddr*)&address, &addrlen);
  if(new_socket > 0) {
    LOG(INFO) << "Client " << inet_ntoa(address.sin_addr) << " is connected";
  }

  //--------------- Run control ---------------//
  int cmd_length = 0;
  char cmd[32];

  // Loop listening for commands from the run control
  do {

    // Wait for new command
    while(buffer[cmd_length - 1] != '\n') {
      LOG(DEBUG) << "Receiving buffer from socket...";
      // retrieve response:
      cmd_length = recv(new_socket, buffer, bufsize, 0);
    }

    // Display the command and load it into the command string
    if(cmd_length > 0) {
      buffer[cmd_length] = '\0';
      LOG(INFO) << "Message received: " << buffer;
      sscanf(buffer, "%s", cmd);
    } else
      sprintf(cmd, "no_cmd");

    // Finally, send a reply to the client
    if(cmd_length > 0) {
      send(new_socket, buffer, strlen(buffer), 0);
      LOG(INFO) << "Sending reply to client: " << buffer;
    }

    // Don't finish until /q received
  } while(strcmp(buffer, "/q"));

  // When finished, close the sockets
  close(new_socket);
  close(my_socket);

  clean();
  return 0;
}
