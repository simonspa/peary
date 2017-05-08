#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>

#include "configuration.hpp"
#include "devicemgr.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;

int my_socket, new_socket;

// Global functions
bool configure();
bool start_run(std::string prefix, int run_nr, std::string description);
bool stop_run(std::string prefix);
bool timestamp();
void *timestamp_per_sec(void*);  // to run in a separate thread

// Main thread
int main(int argc, char *argv[]){

  int run_nr;
  int prev_run_nr = -1;
  
  // TCP/IP server variables
  int portnumber = 4000;
  struct sockaddr_in address;
  socklen_t addrlen;
  pthread_t ts_thread = 0;
  int bufsize = 1024;
  char *buffer = (char *)malloc(bufsize);
  char cmd[32];
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
  caribou::caribouDeviceMgr* manager = new caribouDeviceMgr();

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
    if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0) LOG(logINFO) << "Socket created";
    
    // Set up which port to listen to
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(portnumber);
    
    // Bind the socket
    if (bind(my_socket,(struct sockaddr *)&address,sizeof(address)) == 0) LOG(logINFO) << "Binding Socket";
    else {
      LOG(logCRITICAL) << "Socket binding failed";
      throw CommunicationError("Socket binding failed");
    }
    
    // Wait for communication from the run control
    listen(my_socket,3);
    addrlen = sizeof(struct sockaddr_in);
    
    // Wait for client to connect (will block until client connects)
    new_socket = accept(my_socket,(struct sockaddr *)&address,&addrlen);
    if (new_socket > 0) LOG(logINFO) << "Client " << inet_ntoa(address.sin_addr) << " is connected";

    //--------------- Run control ---------------//
    bool cmd_recognised = false;
    int cmd_length = 0;
    char dummy[32];
    run_nr = -1;
    prev_run_nr = -1;
    bool run_started = false;

    // Loop listening for commands from the run control
    do {
      
      // Wait for new command
      cmd_length = recv(new_socket, buffer, bufsize, 0);
      cmd_recognised = false;
      
      // Display the command and load it into the command string
      if(cmd_length > 0){
        buffer[cmd_length] = '\0';
        LOG(logDEBUG) << "Message received: " << buffer;
        sscanf(buffer, "%s", cmd);
      } else sprintf( cmd, "no_cmd" );

      //======== Configure command
      if(strcmp(cmd,"configure") == 0){
        cmd_recognised = true;
        LOG(logINFO) << "Configuring ...";

        // Configure the system
      }
      
      //======== Start run command
      if(strcmp(cmd,"start_run") == 0){
        cmd_recognised = true;
        LOG(logINFO) << "Starting run";

        // Get the run number and comment (placed in output file header)
	std::istringstream runInfo(buffer);
	std::string description, dummy;
        runInfo >> dummy >> run_nr >> description;
        LOG(logINFO) << "Starting run " << run_nr;

	
        // Define the run directory
        rundir = "Run" + to_string(run_nr);

	bool status = true; // FIXME START RUN

        // Reply to the run control
        if (status == true) {
          run_started = true;
          // Write the reply string to the run control
          sprintf(buffer,"OK run %d started", run_nr);
	  // Also log locally
          LOG(logINFO) << buffer;
        }else {
          // Stop the DAQ threads
          status = false; // FIXME STOP RUNS?
	    // Write the reply string to the run control
	    run_started = false;
          sprintf(buffer,"FAILED start run %d status %d", run_nr, status);
	  // Also log locally
          LOG(logERROR) << buffer;
        }
      }
      
      //======== Stop run command
      if (strcmp(cmd,"stop_run")==0) {
        cmd_recognised = true;
        LOG(logINFO) << "Stopping run";
        // Close the time-stamping thread
        if (ts_thread) pthread_cancel( ts_thread );
        // Tell the DAQ to stop
	//        if ( !spidrctrl->closeShutter() )
	//          tcout_spidr_err( "###closeShutter" );
        bool status = stop_run(rundir);
        
        if (status == true) {
          run_started = false;
          sprintf(buffer,"OK run %d stopped", run_nr);
          LOG(logINFO) << buffer;
        }
        else {
          sprintf(buffer,"FAILED stop run %d status %d", run_nr, status);
          LOG(logERROR) << buffer;
        }
      }
      // If we don't recognise the command
      if (!cmd_recognised && (cmd_length > 0)){
        sprintf(buffer,"FAILED unknown command");
        LOG(logERROR) << "Unknown command: " << buffer;
      }
    
      // Finally, send a reply to the client
      if(cmd_length > 0){
        send( new_socket, buffer, strlen(buffer), 0);
        LOG(logINFO) << "Sending reply to client: " << buffer;
      }
      
      // Don't finish until /q received
    } while( strcmp(buffer,"/q") );
    
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
  return true;
}

bool start_run(std::string prefix, int runNo, std::string description) {

  // Start the DAQ
  return true;
}
        

bool stop_run(std::string runDirectory) {
  
  // Stop the DAQ
  return true;
}
