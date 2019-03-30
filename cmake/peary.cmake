# For every device, build a separate library to be loaded by peary core
MACRO(peary_enable_default val)
    # Get the name of the device
    GET_FILENAME_COMPONENT(_peary_device_dir ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    # Build all devices by default if not specified otherwise
    OPTION(BUILD_${_peary_device_dir} "Build device in directory ${_peary_device_dir}?" ${val})
ENDMACRO()

# Common device definitions
MACRO(peary_device name)
    # Get the name of the device
    GET_FILENAME_COMPONENT(_peary_device_dir ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    # Build all devices by default if not specified otherwise
    OPTION(BUILD_${_peary_device_dir} "Build device in directory ${_peary_device_dir}?" ON)

    # Put message
    MESSAGE(STATUS "Building device " ${BUILD_${_peary_device_dir}} "\t- " ${_peary_device_dir})

    # Quit the file if not building this file or all devices
    IF(NOT (BUILD_${_peary_device_dir} OR BUILD_ALL_DEVICES))
        RETURN()
    ENDIF()

    # Prepend with the peary device prefix to create the name of the device
    SET(${name} "PearyDevice${_peary_device_dir}")

    # Save the device library for prelinking in the executable (NOTE: see exec folder)
    SET(PEARY_DEVICE_LIBRARIES ${PEARY_DEVICE_LIBRARIES} ${${name}} CACHE INTERNAL "Device libraries")

    # Set default device class name
    SET(_peary_device_class "${_peary_device_dir}Device")

    # Find if alternative device class name is passed or we can use the default
    SET (extra_macro_args ${ARGN})
    LIST(LENGTH extra_macro_args num_extra_args)
    IF (${num_extra_args} GREATER 0)
        MESSAGE (AUTHOR_WARNING "Provided non-standard device class name! Naming it ${_peary_device_class} is recommended")
        LIST(GET extra_macro_args 0 _peary_device_class)
    ENDIF ()

    # check if main header file is defined
    IF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_peary_device_class}.hpp")
        MESSAGE(FATAL_ERROR "Header file ${_peary_device_class}.hpp does not exist, cannot build device! \
Create the header or provide the alternative class name as first argument")
    ENDIF()

    # Define the library
    ADD_LIBRARY(${${name}} SHARED "")

    # Add the current directory as include directory
    TARGET_INCLUDE_DIRECTORIES(${${name}} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    # Set the special header flags and add the special dynamic implementation file
    TARGET_COMPILE_DEFINITIONS(${${name}} PRIVATE PEARY_DEVICE_CLASS=${_peary_device_class})
    STRING(REPLACE Device "" _peary_device_name ${_peary_device_class})
    TARGET_COMPILE_DEFINITIONS(${${name}} PRIVATE DEVICE_NAME="${_peary_device_name}")
    TARGET_COMPILE_DEFINITIONS(${${name}} PRIVATE PEARY_DEVICE_HEADER="${_peary_device_class}.hpp")

    TARGET_SOURCES(${${name}} PRIVATE "${PROJECT_SOURCE_DIR}/peary/device/dynamic_device_impl.cpp")
    SET_PROPERTY(SOURCE "${PROJECT_SOURCE_DIR}/peary/device/dynamic_device_impl.cpp" APPEND PROPERTY OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_peary_device_class}.hpp")
    SET_PROPERTY(SOURCE "${PROJECT_SOURCE_DIR}/peary/device/device.cpp" APPEND PROPERTY OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_peary_device_class}.hpp")
ENDMACRO()

# Add sources to the device
MACRO(peary_device_sources name)
    # Get the list of sources
    SET(_list_var "${ARGN}")
    LIST(REMOVE_ITEM _list_var ${name})

    # Include directories for dependencies
    INCLUDE_DIRECTORIES(SYSTEM ${PEARY_DEPS_INCLUDE_DIRS})

    # Add the library
    TARGET_SOURCES(${name} PRIVATE ${_list_var})

    # Link the standard peary libraries
    TARGET_LINK_LIBRARIES(${name} ${PEARY_LIBRARIES} ${PEARY_DEPS_LIBRARIES})
ENDMACRO()

# Provide default install target for the device
MACRO(peary_device_install name)
    INSTALL(TARGETS ${name}
        COMPONENT devices
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
ENDMACRO()
