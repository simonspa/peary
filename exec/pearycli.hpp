/**
 * Caribou Device API class header
 */

#ifndef PEARY_CLI_H
#define PEARY_CLI_H

#include "../extern/cpp-readline/src/Console.hpp"
#include "devicemgr.hpp"

#include <vector>

using ret = CppReadline::Console::ReturnCode;

namespace caribou {

  class pearycli {

  public:

    pearycli();
    ~pearycli();

    int readLine() { return c.readLine(); }
    int executeFile(std::string file) { return c.executeFile(file); }

    static int devices(const std::vector<std::string> &);
    static int verbosity(const std::vector<std::string> & input);
    static int powerOn(const std::vector<std::string> & input);

    // Readline console object
    CppReadline::Console c;

    // Create new Peary device manager
    static caribou::caribouDeviceMgr * manager;
  };

} //namespace caribou

#endif /* PEARY_CLI_H */
