PEARY_ENABLE_DEFAULT(ON)

# Define device and return the generated name as DEVICE_NAME
PEARY_DEVICE(DEVICE_NAME)

# Add source files to library
PEARY_DEVICE_SOURCES(${DEVICE_NAME}
	example.cpp
)

# Provide standard install target
PEARY_DEVICE_INSTALL(${DEVICE_NAME})
