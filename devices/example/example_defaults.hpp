
#include "dictionary.hpp"

namespace caribou {

#define EXAMPLE_DAC_TEST 5
#define EXAMPLE_DAC_VEC  1,3,5,7,9

#define DEFAULT_DEVICEPATH "/dev/i2c-0"

  class exampleDict : public dictionary<exampleDict> {};

  template<>
  std::map<std::string,registerConfig> dictionary<exampleDict>::_registers = {
      {"vthreshold", registerConfig(5,255)},
      {"vkrum", registerConfig(6,255)},
      {"vadc", registerConfig(6,255)}
    };

}
