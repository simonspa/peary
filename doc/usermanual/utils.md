# Peary Caribou Utilities

## Configuration Objects

Configuration is done via simple but flexible text files containing key-value pairs. The value can also be a comma-separated vector of any type.

Configuration files are provided by the user code via the constructor of the device classes. Configuration can then be accessed e.g. via

```
uint64_t myvar = _config.Get("key_of_myvar",1234);
```

where the second parameter is a default value used in case the key is not present in the provided file. In device implementations, all default configuration parameters should be collected in a header file as demonstrated for [the example device](devices/example/example_defaults.hpp). This avoids scattering hard-coded values across the code while still always providing sane defaults whenever the device is operated.

## Data Types

## Dictionaries

## Logging
