/**
 * Caribou implementation for the ATLASPix chip
 */

#ifndef DEVICE_ATLASPix_H
#define DEVICE_ATLASPix_H

#include <algorithm>
#include <atomic>
#include <bitset>
#include <cstdlib>
#include <map>
#include <string>
#include <thread>

#include "device/CaribouDevice.hpp"
#include "interfaces/I2C/i2c.hpp"

#include "ATLASPixMatrix.hpp"
#include "ATLASPix_defaults.hpp"

namespace caribou {

  struct pixelhit {

    uint32_t col = 0;
    uint32_t row = 0;
    uint32_t ts1 = 0;
    uint32_t ts2 = 0;
    uint64_t fpga_ts = 0;
    uint32_t tot = 0;
    uint32_t SyncedTS = 0;
    uint32_t triggercnt;
    uint32_t ATPbinaryCnt;
    uint32_t ATPGreyCnt;

    bool operator==(const pixelhit& hit) {

      if((col == hit.col) && (row == hit.row)) {
        return true;
      } else {
        return false;
      }
    }
  };

  typedef std::map<std::pair<int, int>, unsigned int> CounterMap;
  typedef std::map<std::pair<int, int>, double> TOTMap;

  /** ATLASPix Device class definition
   */
  class ATLASPixDevice : public CaribouDevice<iface_i2c> {

  public:
    ATLASPixDevice(const caribou::Configuration config);
    ~ATLASPixDevice();

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

    /* power monitoring thread */
    void MonitorPower();
    void StopMonitorPower();

    /** Set output base directory for all files.
     */
    void setOutputDirectory(std::string);

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

    void setOutput(std::string datatype);
    void SetScanningMask(uint32_t mx, uint32_t my);

    // Reset the chip
    // The reset signal is asserted for ~5us
    void reset();
    void resetFIFO();
    void isLocked();

    void configureClock();
    void getTriggerCount();
    uint32_t getTriggerCounter();

    pearydata getDataBin();

    std::vector<uint32_t> getRawData();
    pearydata getData();
    pearydata getDataTO(int /* maskx */, int /* masky */);
    std::vector<pixelhit> getDataTOvector(uint32_t timeout = Tuning_timeout, bool noisescan = 0);
    std::vector<pixelhit> getDataTimer(uint32_t timeout, bool to_nodata = false);
    void NoiseRun(double duration);

    void dataTuning(double vmax, int nstep, uint32_t npulses);
    void VerifyTuning(double vmin, double vmax, int npulses, int npoints);
    void TDACScan(int VNDAC, double vmin, double vmax, uint32_t npulses, uint32_t npoints);

    // void doSCurve(uint32_t col, uint32_t row, double vmin, double vmax, uint32_t npulses, uint32_t npoints);

    void doSCurvePixel(uint32_t col, uint32_t row, double vmin, double vmax, uint32_t npulses, uint32_t npoints);
    void doSCurves(double vmin, double vmax, uint32_t npulses, uint32_t npoints);
    void doSCurvesAndWrite(std::string basefolder, double vmin, double vmax, uint32_t npulses, uint32_t npoints);
    void ComputeSCurves(ATLASPixMatrix& matrix, double vmax, int nstep, int npulses, int tup, int tdown);
    void PulseTune(double /* target */);
    void MeasureTOT(double vmin, double vmax, uint32_t npulses, uint32_t npoints);
    void AverageTOT(std::vector<pixelhit> data, uint32_t maskidx, uint32_t maskidy, TOTMap& tots);

    void ReapplyMask();
    void LoadTDAC(std::string filename);
    void setAllTDAC(uint32_t value);
    void MaskPixel(uint32_t col, uint32_t row);
    void FindHotPixels(uint32_t threshold);
    void MaskColumn(uint32_t col);
    void WriteConfig(std::string name);
    void WriteFWRegistersAndBias(std::string name);
    void LoadConfig(std::string filename);

    // void doNoiseCurve(uint32_t col, uint32_t row);
    void pulse(uint32_t npulse, uint32_t tup, uint32_t tdown, double amplitude);
    void SetPixelInjection(uint32_t col, uint32_t row, bool ana_state, bool hb_state, bool inj_state);
    void SetPixelInjectionState(uint32_t col, uint32_t row, bool ana_state, bool hb_state, bool inj);
    void SetInjectionOff();

  private:
    void ProgramSR(const ATLASPixMatrix& matrix);
    void setSpecialRegister(std::string name, uint32_t value);
    void writeOneTDAC(ATLASPixMatrix& matrix, uint32_t col, uint32_t row, uint32_t value);
    void writeUniformTDAC(ATLASPixMatrix& matrix, uint32_t value);
    void writeAllTDAC(ATLASPixMatrix& matrix);
    void SetInjectionMask(uint32_t maskx, uint32_t masky, uint32_t state);
    void ResetWriteDAC();

    template <typename T> uint32_t getSpecialRegister(std::string name);

    std::vector<pixelhit> CountHits(std::vector<pixelhit> data, uint32_t maskidx, uint32_t maskidy, CounterMap& counts);
    uint32_t CountHits(std::vector<pixelhit> data, uint32_t col, uint32_t row);
    void resetCounters();
    int readCounter(int i);
    int readCounter(ATLASPixMatrix& matrix);

    void resetPulser();
    void setPulse(ATLASPixMatrix& /* matrix */, uint32_t npulse, uint32_t n_up, uint32_t n_down, double voltage);
    void sendPulse();

    // void tune(ATLASPixMatrix& matrix, double vmax, int nstep, int npulses, bool tuning_verification);
    void LoadConfiguration(int matrix);

    void runDaq();
    void runMonitorPower();

    ATLASPixMatrix theMatrix;
    int pulse_width;

    std::atomic_flag _daqContinue;
    std::atomic_flag _monitorPowerContinue;

    std::thread _daqThread;
    std::thread _monitorPowerThread;

    std::string _output_directory;
    std::string data_type;
    std::vector<pixelhit> hplist;

    // SW registers
    bool daqRunning = false;
    bool filter_hp = false;
    bool filter_weird_data{};
    bool gray_decoding_state = false;
    bool HW_masking = false;
    bool ro_enable;
    bool busy_when_armed;
    uint32_t armduration;
    bool edge_sel, trigger_enable, trigger_always_armed, t0_enable, send_fpga_ts, trigger_injection, gray_decode, tlu_clock;
  };

} // namespace caribou

#endif /* DEVICE_ATLASPix_H */
