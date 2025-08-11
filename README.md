# Project

**Installations:**
Install ESP-IDF Extension using from this link https://github.com/espressif/vscode-esp-idf-extension/tree/master
Select version v5.5.

 **System Algorithm**

	**ESP #1**
	
	Config the sensor to communicate with the device via the SPI peripheral.
	Config the UART communication with ESP #2.
	Record current temperature evrey three seconds and transmit to ESP #2.

	**ESP #2**
	Config LED.
	Config the UART communication with ESP #1.
	Config timer and set ISR for that timer.
			An ISR was choosen to ensure accurate toggle between On and Off mode of the LED.
	On-time for the LED is const and set to 200[ms].
	Off-time is temperature dependent:
	0 - 10 : 1780[ms]
	10 - 30 : Off-time is decreased linearly from 1780[ms] to 100[ms]
	above 30 : Fast continious blinking, Off-time is set to 40[ms]

	

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


