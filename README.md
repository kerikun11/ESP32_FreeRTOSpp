# ESP32_FreeRTOSpp
C++ Wrapper for ESP32 FreeRTOS Library

## Install

This is a component of ESP-IDF.

```sh
cd YOUR_PROJECT_DIR
mkdir components # make local components directory if it does not exist
git clone https://github.com/kerikun11/ESP32_FreeRTOSpp components/freertospp
```

## Example

```cpp
#include "FreeRTOSpp.h"

extern "C" void app_main() {
	FreeRTOSpp::Mutex mutex;
	mutex.take();
	mutex.give();
}
```
