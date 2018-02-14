/**
 * Caribou Device API class header
 */

#ifndef PEARY_CLI_H
#define PEARY_CLI_H

#include "../extern/cpp-readline/src/Console.hpp"
#include "configuration.hpp"
#include "devicemgr.hpp"

#include <vector>

using ret = CppReadline::ReturnCode;

namespace caribou {

  class pearycli {

  public:
    pearycli();
    ~pearycli();

    int readLine() { return c.readLine(); }
    int executeFile(std::string file) { return c.executeFile(file); }

    static int devices(const std::vector<std::string>&);
    static int addDevice(const std::vector<std::string>& input);
    static int verbosity(const std::vector<std::string>& input);
    static int delay(const std::vector<std::string>& input);

    static int version(const std::vector<std::string>& input);
    static int configure(const std::vector<std::string>& input);
    static int reset(const std::vector<std::string>& input);
    static int powerOn(const std::vector<std::string>& input);
    static int powerOff(const std::vector<std::string>& input);
    static int setVoltage(const std::vector<std::string>& input);
    static int setBias(const std::vector<std::string>& input);
    static int setCurrent(const std::vector<std::string>& input);
    static int getVoltage(const std::vector<std::string>& input);
    static int getCurrent(const std::vector<std::string>& input);
    static int getPower(const std::vector<std::string>& input);
    static int switchOn(const std::vector<std::string>& input);
    static int switchOff(const std::vector<std::string>& input);

    static int setRegister(const std::vector<std::string>& input);
    static int getRegister(const std::vector<std::string>& input);
    static int getRegisters(const std::vector<std::string>& input);
    static int configureMatrix(const std::vector<std::string>& input);
    static int configurePatternGenerator(const std::vector<std::string>& input);
    static int triggerPatternGenerator(const std::vector<std::string>& input);

    static int scanDAC(const std::vector<std::string>& input);
    static int scanDAC2D(const std::vector<std::string>& input);
    static int scanThreshold(const std::vector<std::string>& input);
    static int scanThreshold2D(const std::vector<std::string>& input);

    static int exploreInterface(const std::vector<std::string>& input);
    static int getADC(const std::vector<std::string>& input);
    static int powerStatusLog(const std::vector<std::string>& input);

    static int daqStart(const std::vector<std::string>& input);
    static int daqStop(const std::vector<std::string>& input);

    static int getRawData(const std::vector<std::string>& input);
    static int getData(const std::vector<std::string>& input);
    static int dataTuning(const std::vector<std::string>& input);
    static int acquire(const std::vector<std::string>& input);
    static int flushMatrix(const std::vector<std::string>& input);
    static int lock(const std::vector<std::string>& input);
    static int unlock(const std::vector<std::string>& input);
    static int setThreshold(const std::vector<std::string>& input);
    static int pulse(const std::vector<std::string>& input);
    static int SetPixelInjection(const std::vector<std::string>& input);
    static int doNoiseCurve(const std::vector<std::string>& input);
	static int doSCurve(const std::vector<std::string>& input);
	static int doSCurves(const std::vector<std::string>& input);
	static int setAllTDAC(const std::vector<std::string>& input);
	static int LoadTDAC(const std::vector<std::string>& input);
	static int LoadConfig(const std::vector<std::string>& input);
	static int WriteConfig(const std::vector<std::string>& input);
	static int TDACScan(const std::vector<std::string>& input);
	static int SetMatrix(const std::vector<std::string>& input);
	static int MaskPixel(const std::vector<std::string>& input);
	static int isLocked(const std::vector<std::string>& input);

    // Create new Peary device manager
    static caribou::caribouDeviceMgr* manager;

    // Configuration object
    static caribou::Configuration config;

    static std::string getFileHeader(std::string function, caribouDevice* dev);
    static std::string allDeviceParameters();

  private:
    // Readline console object
    CppReadline::Console c;
  };

} // namespace caribou

#endif /* PEARY_CLI_H */
