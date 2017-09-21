#include <arpa/inet.h>
#include <fstream>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <vector>

#include "configuration.hpp"
#include "devicemgr.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;

caribou::caribouDeviceMgr* manager;
int my_socket;
std::ofstream myfile;
unsigned int framecounter;
std::string configfile;
caribou::Configuration config;

// Global functions
bool configure(int value, unsigned int configureAttempts);
bool start_run(std::string prefix, int run_nr, std::string description);
bool stop_run(std::string prefix);
bool getFrame();
std::vector<std::string> split(std::string str, char delimiter);
void copyFile(std::string src, std::string dst);

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

  int bufsize = 1024;
  char* buffer = (char*)malloc(bufsize);
  std::string rundir = ".";
  std::string ipaddress;

  std::vector<std::string> devices;
  configfile = "";

  // Quick and hacky cli arguments reading:
  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-c configfile  configuration file to be used" << std::endl;
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

  // Create all Caribou devices instance:
  try {

    // Open configuration file and create object:
    std::ifstream file(configfile.c_str());
    if(!file.is_open()) {
      LOG(logERROR) << "No configuration file provided.";
      throw caribou::ConfigInvalid("No configuration file provided.");
    } else
      config = caribou::Configuration(file);

    // Spawn all devices found in the configuration file
    for(auto d : config.GetSections()) {
      if(!config.SetSection(d)) {
        throw caribou::ConfigInvalid("Could not set configuration section for device.");
      }
      size_t device_id = manager->addDevice(d, config);
      LOG(logINFO) << "Manager returned device ID " << device_id << ", fetching device...";

      // Get the device from the manager:
      caribouDevice* dev = manager->getDevice(device_id);
      // Switch on its power:
      dev->powerOn();
    }

    // Configure Socket and address
    int portnumber = 8890;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(portnumber);
    inet_aton(ipaddress.c_str(), &(address.sin_addr));
    int my_socket = socket(AF_INET, SOCK_STREAM, 0);

    std::stringstream ss;
    ss << inet_ntoa(address.sin_addr);

    // Connect to Runcontrol
    int retval = ::connect(my_socket, (struct sockaddr*)&address, sizeof(address));

    if(retval == 0) {
      std::cout << "Connection to server at " << ss.str() << " established" << std::endl;
    } else {
      std::cout << "Connection to server at " << ss.str() << " failed, errno " << errno << std::endl;
    }

    //--------------- Run control ---------------//
    bool cmd_recognised = false;
    int cmd_length = 0;
    char cmd[32];
    run_nr = -1;

    // Simple state machine
    bool configured = false;
    bool running = false;

    std::vector<std::string> commands;
    // Loop listening for commands from the run control
    do {

      // Wait for new command
      // cmd_length = recv(my_socket, buffer, bufsize, 0);
      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 100;

      fd_set set;
      FD_ZERO(&set);           /* clear the set */
      FD_SET(my_socket, &set); /* add our file descriptor to the set */

      int rv = select(my_socket + 1, &set, NULL, NULL, &timeout);
      // LOG(logDEBUG) <<rv;
      /*if (rv == SOCKET_ERROR)
	  {
	      // select error...
      }
	  else*/ if(rv == 0) {
        // timeout, socket does not have anything to read
        cmd_length = 0;
      } else {
        cmd_length = recv(my_socket, buffer, bufsize, 0);
      }
      // socket has something to read
      cmd_recognised = false;
      // LOG(logDEBUG) << "cmd_length: " << cmd_length;
      // Display the command and load it into the command string
      // if(cmd_length > 0) {
      if(commands.size() > 0 || cmd_length > 0) {
        buffer[cmd_length] = '\0';
        LOG(logINFO) << "Message received: " << buffer;
        std::vector<std::string> spl;
        spl = split(std::string(buffer), '\n');
        for(unsigned int k = 0; k < spl.size(); k++) {
          commands.push_back(spl[k]);
          LOG(logINFO) << "commands[" << k << "]: " << commands[k];
        }
        sscanf(commands[0].c_str(), "%s", cmd);
        sprintf(buffer, "%s", commands[0].c_str());
        LOG(logINFO) << buffer;
        commands.erase(commands.begin());
      } else
        sprintf(cmd, "no_cmd");

      if(strcmp(cmd, "configure") == 0) {
        cmd_recognised = true;

        std::istringstream runInfo(buffer);
        std::string dummy;
        int value;
        runInfo >> dummy >> value;

        // Already running!
        if(running) {
          sprintf(buffer, "FAILED configuring - already running");
          LOG(logERROR) << buffer;
        } else if(configure(value, 5)) {
          configured = true;
          sprintf(buffer, "OK configured");
          LOG(logINFO) << buffer;
        } else {
          configured = false;
          sprintf(buffer, "FAILED configuring");
          LOG(logERROR) << buffer;
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
          LOG(logINFO) << "Buffer: " << buffer;
          std::istringstream runInfo(buffer);
          std::string description, dummy;
          runInfo >> dummy >> run_nr >> description;
          LOG(logINFO) << "Starting run " << run_nr;

          // Define the run directory
          std::string dir = rundir + "/Run" + to_string(run_nr);
          framecounter = 0;
          // Reply to the run control
          if(start_run(dir, run_nr, description)) {
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
            framecounter = 0;
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

      if(running)
        getFrame();

      // Don't finish until /q received
    } while(strcmp(buffer, "/q"));

    // When finished, close the sockets
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

bool configure(int value, unsigned int configureAttempts) {

  // Fetch all active devices:
  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      LOG(logINFO) << "Configuring device ID " << i << ": " << d->getName();
      // try to configure the chip ~configureAttempts~ times
      for(unsigned int i = 0; i < configureAttempts; ++i) {
        try {
          d->configure();
          break;
        } catch(const CommunicationError& e) {
          LOG(logERROR) << e.what();
          if(i == configureAttempts - 1)
            return false;
        }
      }
      d->powerStatusLog();
      if(d->getName() == "CLICpix2") {
        d->setRegister("threshold", value);
        LOG(logINFO) << "Setting threshold to " << value << ": " << d->getRegister("threshold_msb") << "-"
                     << d->getRegister("threshold_lsb");
      }
      i++;
    }
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return false;
  }
  return true;
}

bool start_run(std::string rundir, int run_nr, std::string) {

  // Fetch all active devices:
  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto dev : devs) {
      LOG(logINFO) << "Starting run for device ID " << i << ": " << dev->getName();
      // Start the DAQ
      // dev->daqStart();
      if(dev->getName() == "CLICpix2") {
        mkdir(rundir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        std::string filename = rundir + "/run" + to_string(run_nr) + ".raw";
        LOG(logINFO) << "Writing data to " << rundir;
        myfile.open(filename);
        // myfile << "# pearycli > acquire\n";
        myfile << "# Software version: " << dev->getVersion() << "\n";
        myfile << "# Firmware version: " << dev->getFirmwareVersion() << "\n";
        myfile << "# Register state: " << listVector(dev->getRegisters()) << "\n";
        myfile << "# Timestamp: " << LOGTIME << "\n";
      }
      i++;
    }

    // copy config file to run folder
    LOG(logINFO) << "Copy config file: " << configfile;
    copyFile(configfile, rundir + "/" + configfile);

    // Copy matrix and pattern generator
    LOG(logINFO) << "Copy matrix file: " << config.Get("matrix", "");
    copyFile(config.Get("matrix", ""), rundir + "/" + config.Get("matrix", ""));

    LOG(logINFO) << "Copy patterngenerator: " << config.Get("patterngenerator", "");
    copyFile(config.Get("patterngenerator", ""), rundir + "/" + config.Get("patterngenerator", ""));

  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return false;
  }
  return true;
}

bool stop_run(std::string) {

  // Fetch all active devices:
  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      LOG(logINFO) << "Stopping run for device ID " << i << ": " << d->getName();
      // Stop the DAQ
      d->daqStop();
      myfile.close();
      i++;
    }
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return false;
  }
  return true;
}

bool getFrame() {
  LOG(logDEBUG) << "getFrame()";
  std::vector<caribouDevice*> devs = manager->getDevices();
  for(auto dev : devs) {
    if(dev->getName() == "CLICpix2") {
      try {
        // pearydata data;
        std::vector<uint32_t> data;
        try {
          dev->triggerPatternGenerator(true);
          // Read the data:
          data = dev->getRawData();
        } catch(caribou::DataException& e) {
          // Retrieval failed, retry once more before aborting:
          LOG(logWARNING) << e.what() << ", skipping frame.";
          // mDelay(10);
          // data = dev->getData();
          dev->timestampsPatternGenerator(); // in case of readout error, clear timestamp fifo before going to next event
          continue;
        }
        std::vector<uint64_t> timestamps = dev->timestampsPatternGenerator();
        myfile << "===== " << framecounter << " =====\n";
        for(const auto& timestamp : timestamps) {
          myfile << (timestamp >> 48) << ":" << (timestamp & 0xffffffffffff) << "\n";
        }
        for(const auto& px : data) {
          // myfile << px.first.first << "," << px.first.second << "," << (*px.second) << "\n";
          myfile << px << "\n";
        }
        LOG(logINFO) << framecounter << " | " << data.size() << " pixel responses";
        framecounter++;
      } catch(caribou::DataException& e) {
        dev->timestampsPatternGenerator(); // in case of readout error, clear timestamp fifo before going to next event
        continue;
      } catch(caribou::caribouException& e) {
        LOG(logERROR) << e.what();
        return false;
      }
    }
  }

  return true;
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

void copyFile(std::string src, std::string dst) {
  std::ifstream srcfile(src, std::ios::binary);
  std::ofstream dstfile(dst, std::ios::binary);
  dstfile << srcfile.rdbuf();
}
