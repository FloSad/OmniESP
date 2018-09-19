//###############################################################################
// ESP8266 - BH1750
//###############################################################################
/*
GPIO WeMos ESPpin Function  BH1750
 2   D4    17
 0   D3    18
 4   D2    19     SDA       SDA
 5   D1    20     SCL       SCL
 3   RX    21
 1   TX    22
15   D8    16     SS
13   D7     7     MOSI
12   D6     6     MISO
14   D5     5     SCK
16   D0     4     SLEEP!
ADC  A0     2     Analog
3V3                         VCC
GND                         GND
GND                         ADDR    0x23

*/


//###############################################################################
// Sensors
//###############################################################################

#define SDA 4
#define SCL 5

//
// pressure sensor BMP 280
//
#include "Adafruit_BMP280.h"
// 0x76 default, 0x77 secondary
#define ADDRESS_PS (0x76)

//
// light sensor BH1750
//
#include "AS_BH1750.h"
// 0x23 default, 0x5c secondary
#define ADDRESS_LS BH1750_DEFAULT_I2CADDR
#define RESOLUTION RESOLUTION_AUTO_HIGH

//
// UV sensor VEML6070
//
// #include "Adafruit_VEML6070.h" // cannot be included twice

//
// GPIO MCP23017
//
#include "modules/MCP23017.h"
// 0x20 default
#define ADDRESS_MCP 0x20


// default polling interval in milliseconds
#define POLL_IVL_DEF 5000
