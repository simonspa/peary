#ifndef CARIBOU_HAL_H
#define CARIBOU_HAL_H

#include <cstdint>
#include <fcntl.h>
#include <map>
#include <string>
#include <sys/mman.h>
#include <tuple>
#include <vector>

#include "interface.hpp"
#include "interface_manager.hpp"

#include "carboard.hpp"
#include "constants.hpp"
#include "exceptions.hpp"
#include "log.hpp"
#include "utils.hpp"

#include "interface.hpp"
#include "interface_manager.hpp"

#include "i2c.hpp"
#include "loopback.hpp"
#include "spi.hpp"
#include "spi_CLICpix2.hpp"

#include "hal.hcc"
#include "hal.tcc"

#endif /* CARIBOU_HAL_H */
