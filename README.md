# ESP32-Snake-Game-with-RFID-Blocking
This project is all about making a snake game with RFID blocking

The components/libraries you will need for this project:
ADAFRUIT 1306 Compatible OLED screen that is 0.96 in
RFID-RC522 RFID MODULE
ESP32
Joystick Potentiometer
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

These are the headers

AdaFruit BusIO
Adafruit GFX Library
AdaFruit SSD1306
MFRC522
U8G2

These are the libraries

OLED is connected to respective pins(VCC, GND, SDA, SCL)
Joystick(3.3v) is connected to respective pins(GND, 3.3v, VRx, VRy,(no need for SW))
RFID is also connected to respective pins(SDA, SCK, MOSI, MISO, (IRO is not needed for this project), GND, 3.3v)

After everything is connected, get the source code from above to run the code.

All of this was run on ARDUINO IDE 2.3.10

Aadit Dash

