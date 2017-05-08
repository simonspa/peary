#include <arpa/inet.h>
#include <fstream>
#include <sys/socket.h>

#include "configuration.hpp"
#include "devicemgr.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;

caribou::caribouDeviceMgr* manager;

// Global functions
bool configure();
bool start_run(std::string prefix, int run_nr, std::string description);
bool stop_run(std::string prefix);

// Main thread
int main(int argc, char* argv[]) {

  int run_nr;
  int prev_run_nr = -1;

  // TCP/IP server variables
  int my_socket, new_socket;
  int portnumber = 4000;
  struct sockaddr_in address;
  socklen_t addrlen;
  pthread_t ts_thread = 0;
  int bufsize = 1024;
  char* buffer = (char*)malloc(bufsize);
  std::string rundir;

  std::vector<std::string> devices;
  std::string configfile = "";

  // Quick and hacky cli arguments reading:
  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-c configfile  configuration file to be used" << std::endl;
      std::cout << "-r portnumber  start TCP/IP server listening on given port" << std::endl;
      std::cout << "-d dirname     sets output directy path to given folder" << std::endl;
      return 0;
    } else if(!strcmp(argv[i], "-v")) {
      Log::ReportingLevel() = Log::FromString(std::string(argv[++i]));
      continue;
    } else if(!strcmp(argv[i], "-c")) {
      configfile = std::string(argv[++i]);
      continue;
    } else if(!strcmp(argv[i], "-r")) {
      portnumber = std::stoi(argv[++i]);
      LOG(logINFO) << "Starting Peary control server, listening on port " << portnumber;
      continue;
    } else if(!strcmp(argv[i], "-d")) {
      continue;
    } else {
      std::cout << "Unrecognized option: " << argv[i] << std::endl;
    }
  }

  // Create new Peary device manager
  manager = new caribouDeviceMgr();

  // Create all Caribou devices instance:
  try {

    // Open configuration file and create object:
    caribou::Configuration config;
    std::ifstream file(configfile.c_str());
    if(!file.is_open()) {
      LOG(logERROR) << "No configuration file provided.";
      throw caribou::ConfigInvalid("No configuration file provided.");
    } else
      config = caribou::Configuration(file);

    // Spawn all devices found in the configuration file
    for(auto d : config.GetSections()) {

      size_t device_id = manager->addDevice(d, config);
      LOG(logINFO) << "Manager returned device ID " << device_id << ", fetching device...";

      // Get the device from the manager:
      caribouDevice* dev = manager->getDevice(device_id);
      // Switch on its power:
      dev->powerOn();
    }

    // Create a socket and start listening for commands from run control
    if((my_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0)
      LOG(logINFO) << "Socket created";

    // Set up which port to listen to
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(portnumber);

    // Bind the socket
    if(bind(my_socket, (struct sockaddr*)&address, sizeof(address)) == 0)
      LOG(logINFO) << "Binding Socket";
    else {
      LOG(logCRITICAL) << "Socket binding failed";
      throw CommunicationError("Socket binding failed");
    }

    // Wait for communication from the run control
    listen(my_socket, 3);
    addrlen = sizeof(struct sockaddr_in);

    // Wait for client to connect (will block until client connects)
    new_socket = accept(my_socket, (struct sockaddr*)&address, &addrlen);
    if(new_socket > 0)
      LOG(logINFO) << "Client " << inet_ntoa(address.sin_addr) << " is connected";

    //--------------- Run control ---------------//
    bool cmd_recognised = false;
    int cmd_length = 0;
    char cmd[32];
    run_nr = -1;
    prev_run_nr = -1;

    // Simple state machine
    bool configured = false;
    bool running = false;

    // Loop listening for commands from the run control
    do {

      // Wait for new command
      cmd_length = recv(new_socket, buffer, bufsize, 0);
      cmd_recognised = false;

      // Display the command and load it into the command string
      if(cmd_length > 0) {
        buffer[cmd_length] = '\0';
        LOG(logDEBUG) << "Message received: " << buffer;
        sscanf(buffer, "%s", cmd);
      } else
        sprintf(cmd, "no_cmd");

      if(strcmp(cmd, "configure") == 0) {
        cmd_recognised = true;

        // Already running!
        else if(running) {
          sprintf(buffer, "FAILED configuring - already running");
          LOG(logERROR) << buffer;
        }
        else if(configure()) {
          configured = true;
          sprintf(buffer, "OK configured");
        }
        else {
          configured = false;
          sprintf(buffer, "FAILED configuring");
        }
      }

      if(strcmp(cmd, "start_run") == 0) {
        cmd_recognised = true;

        // Not configured yet!
        if(!configured) {
          sprintf(buffer, "FAILED start run - not configured");
          LOG(logERROR) << buffer;
        }
        // Already running!
        else if(running) {
          sprintf(buffer, "FAILED start run - already running");
          LOG(logERROR) << buffer;
        } else {
          // Get the run number and comment (placed in output file header)
          std::istringstream runInfo(buffer);
          std::string description, dummy;
          runInfo >> dummy >> run_nr >> description;
          LOG(logINFO) << "Starting run " << run_nr;

          // Define the run directory
          rundir = "Run" + to_string(run_nr);

          // Reply to the run control
          if(start_run(rundir, run_nr, description)) {
            running = true;
            sprintf(buffer, "OK run %d started", run_nr);
            LOG(logINFO) << buffer;
          } else {
            running = false;
            sprintf(buffer, "FAILED start run %d", run_nr);
            LOG(logERROR) << buffer;
          }
        }
      }

      if(strcmp(cmd, "stop_run") == 0) {
        cmd_recognised = true;

        // Not running yet!
        if(!running) {
          sprintf(buffer, "FAILED stop run - not running");
          LOG(logERROR) << buffer;
        } else {
          if(stop_run(rundir)) {
            running = false;
            sprintf(buffer, "OK run %d stopped", run_nr);
            LOG(logINFO) << buffer;
          } else {
            sprintf(buffer, "FAILED stop run %d", run_nr);
            LOG(logERROR) << buffer;
          }
        }
      }

      // If we don't recognise the command
      if(!cmd_recognised && (cmd_length > 0)) {
        sprintf(buffer, "FAILED unknown command");
        LOG(logERROR) << "Unknown command: " << buffer;
      }

      // Finally, send a reply to the client
      if(cmd_length > 0) {
        send(new_socket, buffer, strlen(buffer), 0);
        LOG(logDEBUG) << "Sending reply to client: " << buffer;
      }

      // Don't finish until /q received
    } while(strcmp(buffer, "/q"));

    // When finished, close the sockets
    close(new_socket);
    close(my_socket);

    // And end that whole thing correcly:
    delete manager;
    LOG(logINFO) << "Done. And thanks for all the fish.";
  } catch(caribouException& e) {
    LOG(logCRITICAL) << "This went wrong: " << e.what();
    return -1;
  } catch(...) {
    LOG(logCRITICAL) << "Something went terribly wrong.";
    return -1;
  }

  return 0;
}

bool configure() {

  // Fetch all active devices:
  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      LOG(logINFO) << "Configuring device ID " << i << ": " << d->getName();
      i++;
    }
  } catch(caribou::DeviceException& e) {
    return false;
  }
  return true;
}

bool start_run(std::string prefix, int runNo, std::string description) {

  // Fetch all active devices:
  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      LOG(logINFO) << "Starting run for device ID " << i << ": " << d->getName();
      // Start the DAQ
      d->daqStart();
      i++;
    }
  } catch(caribou::DeviceException& e) {
    return false;
  }
  return true;
}

bool stop_run(std::string runDirectory) {

  // Fetch all active devices:
  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      LOG(logINFO) << "Stopping run for device ID " << i << ": " << d->getName();
      // Stop the DAQ
      d->daqStop();
      i++;
    }
  } catch(caribou::DeviceException& e) {
    return false;
  }
  return true;
}
