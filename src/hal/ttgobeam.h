// clang-format off
// upload_speed 921600
// board ttgo-t-beam

#ifndef _TTGOBEAM_H
#define _TTGOBEAM_H

#include <stdint.h>

// Hardware related definitions for TTGO T-Beam board
// (only) for older T-Beam version T22_V05 eternal wiring LORA_IO1 to GPIO33 is needed!
//
// pinouts taken from http://tinymicros.com/wiki/TTGO_T-Beam

#define HAS_LED GPIO_NUM_14 // on board green LED, only new version TTGO-BEAM V07
//#define HAS_LED GPIO_NUM_21 // on board green LED, only old verison TTGO-BEAM V05

#define HAS_LORA 1       // comment out if device shall not send data via LoRa
#define CFG_sx1276_radio 1 // HPD13A LoRa SoC
#define HAS_BUTTON GPIO_NUM_39 // on board button (next to reset)
// #define BAT_MEASURE_ADC ADC1_GPIO35_CHANNEL // battery probe GPIO pin -> ADC1_CHANNEL_7
// #define BAT_VOLTAGE_DIVIDER 2 // voltage divider 100k/100k on board

// Pins for LORA chip SPI interface, reset line and interrupt lines//<<<<<
//#define LORA_SCK (5)
//#define LORA_CS (18)
//#define LORA_MISO (19)
//#define LORA_MOSI (27)
//#define LORA_RST (23)
//#define LORA_IO0 (26)
//#define LORA_IO1 (33)
//#define LORA_IO2 (32)

//#define HAS_SPI 1  // comment out if device shall not send data via SPI
// pin definitions for SPI slave interface
//#define SPI_MOSI GPIO_NUM_27
//#define SPI_MISO GPIO_NUM_19
//#define SPI_SCLK GPIO_NUM_5
//#define SPI_CS   GPIO_NUM_18

// GPS settings
#define HAS_GPS 0 // use on board GPS
//#define GPS_SERIAL 9600, SERIAL_8N1, GPIO_NUM_12, GPIO_NUM_15 // UBlox NEO 6M
//#define GPS_INT GPIO_NUM_34 // 30ns accurary timepulse, to be external wired on pcb: NEO 6M Pin#3 -> GPIO34

// enable only if device has these sensors, otherwise comment these lines
// BME680 sensor on I2C bus
//#define HAS_BME 1 // Enable BME sensors in general
//#define HAS_BME680 GPIO_NUM_21, GPIO_NUM_22 // SDA, SCL
//#define BME680_ADDR BME680_I2C_ADDR_PRIMARY // connect SDIO of BME680 to GND

// BME280 sensor on I2C bus
#define HAS_BME 1 // Enable BME sensors in general
#define HAS_BME280 GPIO_NUM_21, GPIO_NUM_22 // SDA, SCL
#define BME280_ADDR 0x76 // change to 0x77 depending on your wiring

// user defined sensors
#define HAS_SENSORS 1 // comment out if device has user defined sensors
#define HAS_SENSOR_1 1 // comment out if device has user defined sensor #1
#define HAS_SENSOR_2 1 // comment out if device has user defined sensor #2
#define HAS_SENSOR_3 1 // comment out if device has user defined sensor #3

// used pins pluviometer (SENSOR 1)
#define PLUV_PIN GPIO_NUM_4
#define PLUV_RST_PIN GPIO_NUM_0  // enable optocoupler and SCR

// used pin wind direction indicator (SENSOR 2)
#define WDI_PIN GPIO_NUM_34

// used pin anemometer (SENSOR 3)
#define ANEM_PIN GPIO_NUM_35

// SDS011 dust sensor settings
//#define HAS_SDS011 1 // use SDS011
// used pins on the ESP-side:
//#define SDS_TX 19     // connect to RX on the SDS011
//#define SDS_RX 23     // connect to TX on the SDS011

// display (if connected)
//#define HAS_DISPLAY 1
//#define MY_DISPLAY_SDA SDA//<<<<<
//#define MY_DISPLAY_SCL SCL//<<<<<
//#define MY_DISPLAY_RST NOT_A_PIN//<<<<<
//#define MY_DISPLAY_FLIP  1 // use if display is rotated

//#define DISABLE_BROWNOUT 1 // comment out if you want to keep brownout feature

#endif
