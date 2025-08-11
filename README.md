# Project

**Installations:**
Install ESP-IDF Extension using from this link https://github.com/espressif/vscode-esp-idf-extension/tree/master
Select version v5.5.

 

![project](https://github.com/user-attachments/assets/d4db2ae6-789a-4844-acb7-9e20d7cd96dc)

**Build and config of ESP #1**
ESP-IDF Targe is ESP32s2.
Board is board/esp32-wrover-kit-3.3v.cfg.
Before the first build type : idf.py set-target esp32s2
For building : idf.py build.
To flash the program and run it : idf.py -p PORT_NUMBER(path to device on Linux) flash monitor.


**Build and config of ESP #2**
ESP-IDF Targe is ESP32s2.
Board is board/esp32-wrover-kit-3.3v.cfg.
Before the first build type : idf.py set-target esp32s2
Config gptimer_ctl : type idf.py and enable CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM.
For building : idf.py build.
To flash the program and run it : idf.py -p PORT_NUMBER(path to device on Linux) flash monitor.

**System Algorithm**
