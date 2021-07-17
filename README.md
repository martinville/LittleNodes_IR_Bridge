# LittleNodes_IR_Bridge
ESP8266 IR bridge compatible with alexa. Control your IR devices such as a TV by acting as an IR bridge via Amazon Alexa voice commands.

**How it works.**
The INO sketch make use of the ESP8266IRReceiver and Wemo Emulator libraries.
The web front end is making use of twitter bootstrap on a SPIFFS filesystem.

**What it does**
The LittleNodes IR bridge can learn infrared remote codes and play them back to a device by acting on Alexa voice commands.

**Important Prerequisite:**
Without the frontend HTML/CSS GUI the device will not work. Make sure you upload the GUI files using a SPIFFS upload plugin from the Arduino IDE.

**SPIFFS Upload Plugin Installation Instructions:**
https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/
After installing the plugin extract the data.zip file and place the contents in a folder called data inside the arduino sketch folder.
I.E "LittleNodes_IR_Bridge-master\SensorWebClient\data"

Plugin Download: https://github.com/esp8266/arduino-esp8266fs-plugin/releases

![alt text](https://raw.githubusercontent.com/mailmartinviljoen/LittleNodes_IR_Bridge/master/ArduinoIDEPlugin.png)


**Firmware Install Instructions**
Flash the firmware (SensorWebClient.ino.bin) using the ESP8266 Download tool with the following settings.

Memory Address 0x0000
SPI Speed: 26.7Mhz, 
SPI Mode: DIO, 
Flash Size: 8Mbit, 
SPI Autoset: Unchecked, 
DoNotChgBin: Checked

![alt text](https://raw.githubusercontent.com/mailmartinviljoen/LittleNodes_IR_Bridge/master/ESPtoolSettings.png)










