# Kusama's Corner Pumpkin Light
Create RGB Kusama pumpkin lamp powered with an ESP32 with menu toggling function built in. This is proof of concept for subsequent project involving making this smart home comptaible either using espressif or homespan via ESP32. 


## Instructions to run
1) Using the [Arduino IDE]([https://www.arduino.cc/en/software](https://www.arduino.cc/en/software), open the respective ``.ino`` file
2) Install [TTGO T-display driver]([https://github.com/Xinyuan-LilyGO/TTGO-T-Display](https://github.com/Xinyuan-LilyGO/TTGO-T-Display))
3) Assign Arduino IDE port to the one that Lilygo TTGO is connected to
4) Install TFT_eSPI library (display library)
	- Under Arduino IDE/tools/library_manager, search ``tft_espi``
	- Navigate to the library, e.g. Documents/Arduino/libraries/tft_espi
	- Open up the file Arduino/libraries/TFT_eSPI/User_Setup_Select.h
	- comment out the line ``#include <User_setup.h>``
	- uncomment out the line include ``<User_Setups/Setup25_TTGO_T_Display.h>>``
	- note - commenting out means adding ``//`` in front of the line, to uncomment, remove ``//`` but don’t remove the ``#`` sign
5) Install [ESP32 Board defintion]([https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)) under ``Installing using Arduino IDE``
6) Compile and upload the code from the ``.ino`` to your Lilygo TTGO
7) Money 💸💸💸