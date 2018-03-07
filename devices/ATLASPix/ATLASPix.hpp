/**
 * Caribou implementation for the ATLASPixF chip
 */

#ifndef DEVICE_ATLASPix_H
#define DEVICE_ATLASPix_H

#include <algorithm>
#include <atomic>
#include <bitset>
#include <cstdlib>
#include <string>
#include <thread>

#include "device.hpp"
#include "i2c.hpp"
#include "pearydevice.hpp"

#include "ATLASPixMatrix.hpp"
#include "ATLASPix_defaults.hpp"

namespace caribou {

  /** ATLASPix Device class definition
   */
  class ATLASPix : public pearyDevice<iface_i2c> {

  public:
    ATLASPix(const caribou::Configuration config);
    ~ATLASPix();

    void SetMatrix(std::string matrix);

    std::string getName();

    void printBits(size_t const size, void const* const ptr);

    /** Initializer function for ATLASPix
     */
    void configure();

    void lock();
    void unlock();
    void setThreshold(double threshold);
    void setVMinus(double vminus);

    /** Turn on the power supply for the ATLASPix chip
     */
    void powerUp();

    /** Turn off the ATLASPix power
     */
    void powerDown();

    /** Start the data acquisition
     */
    void daqStart();

    /** Stop the data acquisition
     */
    void daqStop();

    /** Report power status
     */
    void powerStatusLog();

    void exploreInterface(){};

    // Reset the chip
    // The reset signal is asserted for ~5us
    void reset();
    void isLocked();

    void configureClock();
    void getTriggerCount();

    uint32_t getTriggerCounter();
    pearydata getData();

    std::vector<int> getCountingData();
    void dataTuning( double vmax, int nstep, int npulses);
    void ReapplyMask();

    void LoadTDAC(std::string filename);
    void setAllTDAC(uint32_t value);
    void TDACScan(std::string basefolder, int VNDAC, int step, double vmin, double vmax, uint32_t npulses, uint32_t npoints);
    void MaskPixel(uint32_t col, uint32_t row);
    void WriteConfig(std::string name);
    void LoadConfig(std::string filename);

    void doSCurve(uint32_t col, uint32_t row, double vmin, double vmax, uint32_t npulses, uint32_t npoints);
    void doSCurves(double vmin, double vmax, uint32_t npulses, uint32_t npoints);
    void doNoiseCurve(uint32_t col, uint32_t row);
    void pulse(uint32_t npulse, uint32_t tup, uint32_t tdown, double amplitude);

    void SetPixelInjection(uint32_t col, uint32_t row, bool ana_state, bool hb_state);

  private:
    void ProgramSR(const ATLASPixMatrix& matrix);
    void setSpecialRegister(std::string name, uint32_t value);
    void writeOneTDAC(ATLASPixMatrix& matrix, uint32_t col, uint32_t row, uint32_t value);
    void writeUniformTDAC(ATLASPixMatrix& matrix, uint32_t value);
    void writeAllTDAC(ATLASPixMatrix& matrix);
    void SetInjectionMask(uint32_t mask, uint32_t state);
    void doSCurves(std::string basefolder, double vmin, double vmax, uint32_t npulses, uint32_t npoints);
    void resetCounters();
    int readCounter(int i);
    int readCounter(ATLASPixMatrix& matrix);

    void resetPulser();
    void setPulse(ATLASPixMatrix& matrix, uint32_t npulse, uint32_t n_up, uint32_t n_down, double voltage);
    void sendPulse();
    void ComputeSCurves(ATLASPixMatrix& matrix, double vmax, int nstep, int npulses, int tup, int tdown);
    void tune(ATLASPixMatrix& matrix, double vmax, int nstep, int npulses, bool tuning_verification);
    void LoadConfiguration(int matrix);
    void TakeData();
    void runDaq();

    ATLASPixMatrix theMatrix;
    int pulse_width;
    std::atomic_flag _daqContinue;
    std::thread _daqThread;
  };

  /** Device generator
   *
   *  This generator function is used by the device manager to create instances of
   *  this device using the abstract caribouDevice class interface. It has to be implemented
   *  for every class deriving from caribouDevice.
   */
  extern "C" {
  caribouDevice* generator(const caribou::Configuration);
  }

} // namespace caribou

#endif /* DEVICE_ATLASPix_H */
