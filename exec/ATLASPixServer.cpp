/*
 * ATLASPixServer.cpp
 *
 *  Created on: Feb 16, 2018
 *      Author: peary
 */
#include <arpa/inet.h>
#include <fstream>
#include <netinet/in.h>
#include <regex>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "configuration.hpp"
#include "devicemgr.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;

caribou::caribouDeviceMgr* manager;
caribouDevice* devM1;
caribouDevice* devM1ISO;
caribouDevice* devM2;

int my_socket;
std::ofstream myfile;

// Global functions

// bool configure(int value, unsigned int configureAttempts);
// bool start_run(std::string prefix, int run_nr, std::string description);
// bool stop_run(std::string prefix);
// bool setThreshold(double value);
// bool setRegister(std::string name,uint32_t value);

std::vector<std::string> split(std::string str, char delimiter);

FILE* lfile;

void termination_handler(int s) {
  std::cout << "\n";
  LOG(logINFO) << "Caught user signal \"" << s << "\", ending processes.";
  delete manager;
  close(my_socket);
  fclose(lfile);

  exit(1);
}

// Main thread
int main(int argc, char* argv[]) {

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = termination_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);

  int run_nr;

  int bufSize = 1024;
  char* buffer = (char*)malloc(bufSize);
  std::string rundir = ".";
  std::string ipaddress;

  std::vector<std::string> devices;
  std::string configfile = "";

  // Quick and hacky cli arguments reading:
  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-i ip          connect to runcontrol on that ip" << std::endl;
      std::cout << "-d dirname     sets output directy path to given folder, folder has to exist" << std::endl;
      return 0;
    } else if(!strcmp(argv[i], "-v")) {
      Log::ReportingLevel() = Log::FromString(std::string(argv[++i]));
      continue;
    } else if(!strcmp(argv[i], "-c")) {
      configfile = std::string(argv[++i]);
      continue;
    } else if(!strcmp(argv[i], "-i")) {
      ipaddress = argv[++i];
      LOG(logINFO) << "Connecting to runcontrol at " << ipaddress;
      continue;
    } else if(!strcmp(argv[i], "-d")) {
      rundir = std::string(argv[++i]);
      continue;
    } else {
      std::cout << "Unrecognized option: " << argv[i] << std::endl;
    }
  }

  lfile = fopen((rundir + "/log.txt").c_str(), "a");
  SetLogOutput::Stream() = lfile;
  SetLogOutput::Duplicate() = true;

  // Create new Peary device manager
  manager = new caribouDeviceMgr();

  caribou::Configuration config = caribou::Configuration();
  size_t device_idM1 = manager->addDevice("ATLASPix", config);
  size_t device_idM1ISO = manager->addDevice("ATLASPix", config);
  size_t device_idM2 = manager->addDevice("ATLASPix", config);

  // Get the device from the manager:
  devM1 = manager->getDevice(device_idM1);
  devM1ISO = manager->getDevice(device_idM1ISO);
  devM2 = manager->getDevice(device_idM2);

  devM1->SetMatrix("M1");
  devM1ISO->SetMatrix("M1ISO");
  devM2->SetMatrix("M2");

  // Switch on its power:
  devM1->powerOn();
  devM1ISO->powerOn();
  devM2->powerOn();

  devM1->configure();
  devM2->configure();
  devM1ISO->configure();

  devM1->lock();
  devM2->lock();
  devM1ISO->lock();

  devM1->unlock();

  devM1->setThreshold(1.2);
  devM1->setRegister("VNFBPix", 32);
  devM1->setRegister("VNPix", 20);
  devM1->setRegister("VNDACPix", 4);
  devM1->setAllTDAC(4);

  /* -------------- INITIALIZING VARIABLES -------------- */
  int server, client;  // socket file descriptors
  int portNum = 2705;  // port number
  bool isExit = false; // var fo continue infinitly

  /* Structure describing an Internet socket address. */
  struct sockaddr_in server_addr;
  socklen_t size;

  std::cout << "\n- Starting server..." << std::endl;

  /* ---------- ESTABLISHING SOCKET CONNECTION ----------*/

  server = socket(AF_INET, SOCK_STREAM, 0);

  /*
  * The socket() function creates a new socket.
  * It takes 3 arguments:
  * 1) AF_INET: address domain of the socket.
  * 2) SOCK_STREAM: Type of socket. a stream socket in
  * which characters are read in a continuous stream (TCP)
  * 3) Third is a protocol argument: should always be 0.
  * If the socket call fails, it returns -1.
  */

  if(server < 0) {
    std::cout << "Error establishing socket ..." << std::endl;
    exit(-1);
  }

  std::cout << "- Socket server has been created..." << std::endl;

  /*
  * The variable serv_addr is a structure of sockaddr_in.
  * sin_family contains a code for the address family.
  * It should always be set to AF_INET.
  * INADDR_ANY contains the IP address of the host. For
  * server code, this will always be the IP address of
  * the machine on which the server is running.
  * htons() converts the port number from host byte order
  * to a port number in network byte order.
  */

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(portNum);

  /*
  * This function is used to set the socket level for socket.
  * It is used to avoid blind error when reuse the socket.
  * For more info, see the url.
  * http://stackoverflow.com/questions/5592747/bind-error-while-recreating-socket
  */

  int yes = 1;
  if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    perror("setsockopt");
    exit(1);
  }
  /* ---------------- BINDING THE SOCKET --------------- */

  /*
  * The bind() system call binds a socket to an address,
  * in this case the address of the current host and port number
  * on which the server will run. It takes three arguments,
  * the socket file descriptor. The second argument is a pointer
  * to a structure of type sockaddr, this must be cast to
  * the correct type.
  */

  if((bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr))) < 0) {
    std::cout << "- Error binding connection, the socket has already been established..." << std::endl;
    exit(-1);
  }

  /* ------------------ LISTENING CALL ----------------- */

  size = sizeof(server_addr);
  std::cout << "- Looking for clients..." << std::endl;

  /*
  * The listen system call allows the process to listen
  * on the socket for connections.
  * The program will be stay idle here if there are no
  * incomming connections.
  * The first argument is the socket file descriptor,
  * and the second is the size for the number of clients
  * i.e the number of connections that the server can
  * handle while the process is handling a particular
  * connection. The maximum size permitted by most
  * systems is 5.
  */

  listen(server, 1);
  /* ------------------- ACCEPT CALL ------------------ */

  client = accept(server, (struct sockaddr*)&server_addr, &size);

  /*
  * The accept() system call causes the process to block
  * until a client connects to the server. Thus, it wakes
  * up the process when a connection from a client has been
  * successfully established. It returns a new file descriptor,
  * and all communication on this connection should be done
  * using the new file descriptor. The second argument is a
  * reference pointer to the address of the client on the other
  * end of the connection, and the third argument is the size
  * of this structure.
  */

  if(client < 0)
    std::cout << "- Error on accepting..." << std::endl;

  std::string echo;
  while(client > 0) {
    // Welcome message to client
    memset(buffer, ' ', bufSize);

    strcpy(buffer, "\n-> Welcome to ATLASPix server...\n");
    send(client, buffer, bufSize, 0);

    std::cout << "- Connected with the client, waiting for commands..." << std::endl;
    // loop to recive messages from client

    bool isExit = 0;
    do {
      std::cout << "\nClient: ";
      echo = "";
      /*
      * A send operation from client is done for each word
      * has written on it's terminal line. We need a special
      * character to stop transmission and this loop works
      * until this char ('*') arrives.
      */
      // wait the request from client
      recv(client, buffer, bufSize, 0);

      std::string cmd(buffer);

      std::cout << "Command received : " << cmd << std::endl;

      // verify if client does not close the connection
      if(cmd.find("configure") != std::string::npos) {
        devM1->configure();
        std::cout << "configuring device M1" << std::endl;
        strcpy(buffer, "[ATLASPixServer] configuring device M1\n");

        // memset(buffer, 0, sizeof buffer);
      } else if(cmd.find("start_run") != std::string::npos) {
        devM1->daqStart();
        std::cout << "Starting Data acquisition" << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Starting Data acquisition\n");

      }

      else if(cmd.find("stop_run") != std::string::npos) {
        devM1->daqStop();

        std::cout << "Stoping Data acquisition" << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Stopping Data acquisition\n");

      }

      else if(cmd.find("setThreshold") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        double value = std::stof(words[1]);
        devM1->setThreshold(value);
        std::cout << "Setting Threshold to " << value << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting threshold\n");

      }

      else if(cmd.find("setRegister") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        uint32_t value = std::stoi(words[2]);
        std::cout << "Setting register " << words[1] << " to " << words[2] << std::endl;
        devM1->setRegister(words[1], value);
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting register\n");

      } else if(cmd.find("setOutputDirectory") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        devM1->setOutputDirectory(words[1]);
        std::cout << "Setting output directory to " << words[1] << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting output directory\n");
      }

      else if(cmd.find("LoadConfig") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Loading Config with basename " << words[1] << std::endl;
        devM1->LoadConfig(words[1]);
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Loading Config\n");

      }

      else if(cmd.find("WriteConfig") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Writing Config with basename " << words[1] << std::endl;
        devM1->WriteConfig(words[1]);
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Writing Config\n");

      } else if(cmd.find("exit") != std::string::npos) {
        std::cout << "Exiting" << std::endl;
        isExit = true;

        delete manager;
        close(my_socket);
        fclose(lfile);

        // memset(buffer, 0, sizeof buffer);

      }

      else {
        std::cout << "command not understood: " << cmd;
        // memset(buffer, 0, sizeof buffer);
      }

      // send the message to the client
      send(client, buffer, bufSize, 0);
    } while(!isExit);

    /* ---------------- CLOSE CALL ------------- */
    std::cout << "\n\n=> Connection terminated with IP " << inet_ntoa(server_addr.sin_addr);
    close(client);
    std::cout << "\nGoodbye..." << std::endl;
    exit(1);
  }

  /* ---------------- CLOSE CALL ------------- */
  close(server);
  return 0;
};

std::vector<std::string> split(std::string str, char delimiter) {
  std::vector<std::string> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  return internal;
}
