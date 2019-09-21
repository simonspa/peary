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

#include "device/DeviceManager.hpp"
#include "utils/configuration.hpp"
#include "utils/exceptions.hpp"
#include "utils/log.hpp"

using namespace caribou;

caribou::DeviceManager* manager;
Device* devM1;
Device* devM1ISO;
Device* devM2;

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
  LOG(INFO) << "Caught user signal \"" << s << "\", ending processes.";
  delete manager;
  close(my_socket);
  fclose(lfile);

  exit(1);
}

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

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = termination_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);

  int bufSize = 256;
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
    } else if(!strcmp(argv[i], "-c")) {
      configfile = std::string(argv[++i]);
      continue;
    } else if(!strcmp(argv[i], "-i")) {
      ipaddress = argv[++i];
      LOG(INFO) << "Connecting to runcontrol at " << ipaddress;
      continue;
    } else if(!strcmp(argv[i], "-d")) {
      rundir = std::string(argv[++i]);
      continue;
    } else {
      std::cout << "Unrecognized option: " << argv[i] << std::endl;
    }
  }

  // Add an extra file to log too if possible
  // NOTE: this stream should be available for the duration of the logging
  std::ofstream log_file;
  log_file.open(rundir + "/log.txt", std::ios_base::out | std::ios_base::trunc);
  if(!log_file.good()) {
    LOG(FATAL) << "Cannot write to provided log file! Check if permissions are sufficient.";
    clean();
  }
  Log::addStream(log_file);

  // Create new Peary device manager
  manager = new DeviceManager();

  caribou::Configuration config = caribou::Configuration();
  size_t device_idM1 = manager->addDevice("ATLASPix", config);
  // size_t device_idM1ISO = manager->addDevice("ATLASPix", config);
  // size_t device_idM2 = manager->addDevice("ATLASPix", config);

  // Get the device from the manager:
  devM1 = manager->getDevice(device_idM1);
  // devM1ISO = manager->getDevice(device_idM1ISO);
  // devM2 = manager->getDevice(device_idM2);

  devM1->command("SetMatrix", "M1");
  // devM1ISO->command("SetMatrix", "M1ISO");
  // devM2->command("SetMatrix", "M2");

  // Switch on its power:
  devM1->powerOn();
  // devM1ISO->powerOn();
  // devM2->powerOn();

  devM1->configure();
  // devM2->configure();
  // devM1ISO->configure();

  devM1->command("lock");
  // devM2->command("lock");
  // devM1ISO->command("lock");

  devM1->command("unlock");

  devM1->command("setThreshold", "1.2");
  devM1->setRegister("VNFBPix", 32);
  devM1->setRegister("VNPix", 20);
  devM1->setRegister("VNDACPix", 4);
  devM1->command("setAllTDAC", "4");

  devM1->command("powerStatusLog");

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
      } else if(cmd.find("daqStart") != std::string::npos) {
        devM1->daqStart();
        std::cout << "Starting Data acquisition" << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Starting Data acquisition\n");

      }

      else if(cmd.find("unlock") != std::string::npos) {
        devM1->command("unlock");
        std::cout << "unlock" << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] unlock\n");

      } else if(cmd.find("lock") != std::string::npos) {
        devM1->command("lock");
        std::cout << "lock" << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] lock\n");

      }

      else if(cmd.find("isLocked") != std::string::npos) {
        devM1->command("isLocked");
        std::cout << "isLocked?" << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] isLocked\n");

      } else if(cmd.find("daqStop") != std::string::npos) {
        devM1->daqStop();
        std::cout << "Stoping Data acquisition" << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Stopping Data acquisition\n");

      }

      else if(cmd.find("setThreshold") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        double value = std::stof(words[1]);
        devM1->command("setThreshold", std::to_string(value));
        std::cout << "Setting Threshold to " << value << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting threshold\n");

      }

      else if(cmd.find("setBias") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        double value = std::stof(words[2]);
        devM1->setVoltage(words[1], value);
        std::cout << "Setting Bias " << words[1] << " to " << value << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting a bias\n");

      } else if(cmd.find("setVMinus") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        devM1->command("setVMinus", words[1]);
        std::cout << "Setting VMinus to " << words[1] << std::endl;
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
        devM1->command("setOutputDirectory", words[1]);
        std::cout << "Setting output directory to " << words[1] << std::endl;
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting output directory\n");
      }

      else if(cmd.find("LoadConfig") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Loading Config with basename " << words[1] << std::endl;
        devM1->command("LoadConfig", words[1]);
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Loading Config\n");

      }

      else if(cmd.find("WriteConfig") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Writing Config with basename " << words[1] << std::endl;
        devM1->command("WriteConfig", words[1]);
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Writing Config\n");

      } else if(cmd.find("resetFIFO") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "reset " << std::endl;
        devM1->command("resetFIFO");
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] reset FIFO \n");

      } else if(cmd.find("reset") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "reset " << std::endl;
        devM1->reset();
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] reset \n");

      } else if(cmd.find("MaskPixel") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Masking Pixel  " << words[1] << " " << words[2] << std::endl;
        devM1->command("MaskPixel", std::vector<std::string>({words[1], words[2]}));
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Masking a pixel \n");
      }

      else if(cmd.find("SetPixelInjection") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Setting injection in Pixel  " << words[1] << " " << words[2] << std::endl;
        devM1->command("SetPixelInjection", {words[1], words[2], words[3], words[4], words[5]});
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting Injection \n");
      }

      else if(cmd.find("setAllTDAC") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Setting all TDAC to  " << words[1] << std::endl;
        devM1->command("setAllTDAC", words[1]);
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting TDAC \n");

      }

      else if(cmd.find("NoiseRun") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Taking a Noise Run for  " << words[1] << " s" << std::endl;
        devM1->command("NoiseRun", words[1]);
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Noise run\n");

      }

      else if(cmd.find("SetInjectionOff") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Setting injection off row by row  " << words[1] << std::endl;
        devM1->command("SetInjectionOff");
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting injection off \n");

      } else if(cmd.find("ReapplyMask") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        std::cout << "Reapplying mask to pixel in the matrix  " << words[1] << std::endl;
        devM1->command("ReapplyMask");
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Reapplying Mask \n");

      } else if(cmd.find("powerStatusLog") != std::string::npos) {
        devM1->command("powerStatusLog");
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Power report\n");

      } else if(cmd.find("setOutput") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        devM1->command("setOutput", words[1]);
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Set output formatt\n");

      } else if(cmd.find("LoadTDAC") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        devM1->command("LoadTDAC", words[1]);
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Loading TDAC file\n");

      } else if(cmd.find("FindHotPixels") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        devM1->command("FindHotPixels", {words[1]});
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Finding hot pixels\n");

      } else if(cmd.find("pulse") != std::string::npos) {
        std::vector<std::string> words = split(cmd, ' ');
        devM1->command("pulse", {words[1], words[2], words[3], words[4]});
        // memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "[ATLASPixServer] Setting pulser\n");

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
    clean();
    exit(1);
  }

  /* ---------------- CLOSE CALL ------------- */
  close(server);
  clean();
  return 0;
}

std::vector<std::string> split(std::string str, char delimiter) {
  std::vector<std::string> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  return internal;
}
