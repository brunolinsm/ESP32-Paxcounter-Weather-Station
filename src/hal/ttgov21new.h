// clang-format off
// upload_speed 921600
// board esp32dev

#ifndef _TTGOV21NEW_H
#define _TTGOV21NEW_H

#include <stdint.h>

/*  Hardware related definitions for TTGO V2.1 Board
// ATTENTION: check your board version!
// This settings are for boards labeled v1.6 on pcb, NOT for v1.5 or older
*/

#define HAS_LORA         1 // comment out if device shall not send data via LoRa
#define CFG_sx1276_radio 1 // HPD13A LoRa SoC

// enable only if you want to store a local paxcount table on the device
//#define HAS_SDCARD  2      // this board has a SDMMC card-reader/writer

// #define HAS_DISPLAY 1
#define HAS_LED (25) // green on board LED
//#define HAS_LED NOT_A_PIN

// Pins for I2C interface of OLED Display
// #define MY_DISPLAY_SDA (21)
// #define MY_DISPLAY_SCL (22)
// #define MY_DISPLAY_RST (16)

//#define BAT_MEASURE_ADC ADC1_GPIO35_CHANNEL // battery probe GPIO pin -> ADC1_CHANNEL_7
//#define BAT_VOLTAGE_DIVIDER 2 // voltage divider 100k/100k on board

// Pins for LORA chip SPI interface, reset line and interrupt lines
#define LORA_SCK  (5) 
#define LORA_CS   (18)
#define LORA_MISO (19)
#define LORA_MOSI (27)
#define LORA_RST  (23)
#define LORA_IRQ  (26)
#define LORA_IO1  (33)
#define LORA_IO2  (32)

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
#define PLUV_PIN GPIO_NUM_13
#define PLUV_RST_PIN GPIO_NUM_12  // enable optocoupler and SCR

// used pin wind direction indicator (SENSOR 2)
#define WDI_PIN GPIO_NUM_34

// used pin anemometer (SENSOR 3)
#define ANEM_PIN GPIO_NUM_35

#endif
