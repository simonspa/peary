macro(optional_add_subdirectory dir)
  option(BUILD_${dir} "Build device in directory ${dir}?" OFF)
  if(BUILD_${dir} OR BUILD_alldevices)
    add_subdirectory(${dir})
    MESSAGE( STATUS "Building device: " ${dir} )
  endif()
endmacro()

# set to build all devices -- mostly useful for tests
option(BUILD_alldevices "Build all devices?" OFF)

optional_add_subdirectory(clicpix2)
