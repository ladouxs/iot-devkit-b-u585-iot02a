# Setup WIFI for the 

This repo hosts the firmware for a project using WI-FI (mxchip) designed to be flashed on a B-U585I-IOT02A development board (STM32). .

## Requirements

### Hardware

- 2 [B-U585I-IOT02A](https://www.st.com/en/evaluation-tools/b-u585i-iot02a.html) board

### Software

- `gcc-arm-none-eabi` toolchain  
- `cmake` (≥3.10)  
- `ninja-build`  

If you want to compile and flash with VSCode one option si to install VSCode with STM32Cube Core extension.

## Getting started

Clone the repository :

```bash
git clone --recursive git@github.com:ladouxs/iot-devkit-b-u585-iot02a.git
```

If you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
```


## Hardware Setup

Make sure the jumpers are all in the right place for the board alimentation (see user manual UM2839).

## Build & Flash


### Configure

Create a CMakeUserPreset.json with your credentials, e.g.:

```json
{
    "version": 2,
    "configurePresets": [
        {
            "name": "debug-user",
            "inherits": "Debug",
            "cacheVariables": {
                "WIFI_SSID": "YOUR-WIFI-SSID",
                "WIFI_PASSWORD": "YOUR-WIFI-PASSWORD"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "debug-user",
            "configurePreset": "debug-user"
        }
    ]
}
```

```bash
cmake --preset debug-user
```

## Build

```bash
cmake --build --preset debug-user
```
