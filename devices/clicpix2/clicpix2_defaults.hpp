#ifndef DEVICE_CLICPIX2_DEFAULTS_H
#define DEVICE_CLICPIX2_DEFAULTS_H

#include "carboard.hpp"
#include "dictionary.hpp"

namespace caribou {

  /** Default device path for this device: SPI interface
   */
#define DEFAULT_DEVICEPATH "/dev/spi-0"

#define CLICpix2_VDDD     1.2
#define CLICpix2_VDDD_CURRENT  3
#define CLICpix2_VDDA     1.2
#define CLICpix2_VDDA_CURRENT 3
#define CLICpix2_VDDCML   1.2
#define CLICpix2_VDDCML_CURRENT 3
#define CLICpix2_CMLBUFFERS_VDD 2.5
#define CLICpix2_CMLBUFFERS_VDD_CURRENT 3
#define CLICpix2_CMLBUFFERS_VCCO 1.2
#define CLICpix2_CMLBUFFERS_VCCO_CURRENT 3
  
#define CLICPIX2_PERIPHERY							\
  {									\
  }
  
} //namespace caribou

#endif /* DEVICE_CLICPIX2_DEFAULTS_H */
