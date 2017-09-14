macro(optional_add_subdirectory dir)
  if(${dir} MATCHES "example|c3pd|clicpix2")
    option(BUILD_${dir} "Build device in directory ${dir}?" ON)
  ELSE()
    option(BUILD_${dir} "Build device in directory ${dir}?" OFF)
  ENDIF()
  if(BUILD_${dir} OR BUILD_alldevices)
    add_subdirectory(${dir})
    MESSAGE( STATUS "Building device: " ${dir} )
  endif()
endmacro()

# set to build all devices -- mostly useful for tests
option(BUILD_alldevices "Build all devices?" OFF)

# Example device implementation - check this directory as a starting point
# for your own device implementation
optional_add_subdirectory(example)

optional_add_subdirectory(clicpix2)
optional_add_subdirectory(c3pd)
optional_add_subdirectory(timepix3)
optional_add_subdirectory(ATLASPix)
