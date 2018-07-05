PEARY_ENABLE_DEFAULT(OFF)

# Define device and return the generated name as DEVICE_NAME
PEARY_DEVICE(DEVICE_NAME)

# Add source files to library
PEARY_DEVICE_SOURCES(${DEVICE_NAME}
    ATLASPixDevice.cpp
    ATLASPixMatrix.cpp
    ATLASPix_Config.cpp
)

# Provide standard install target
PEARY_DEVICE_INSTALL(${DEVICE_NAME})
