# iot-devkit-b-u585-iot2a

Example of project using the ARM build toolchain for STM32 board B-U585I-IOT02A.

## Getting started

* Clone the repository including submodules 

```bash
git clone --recursive https://github.com/ladouxs/iot-devkit-b-u585-iot02a.git
```


* You can compile the project using the following cmake commands :

```bash
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=./cmake/arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Release -B build  
cmake --build build  
```
