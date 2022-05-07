#ifndef _SENSOR_H
#define _SENSOR_H

#include "configmanager.h"
#include "irqhandler.h"
#include "timekeeper.h"
#include "globals.h"
#include "bmesensor.h"

#define HAS_SENSORS (HAS_SENSOR_1 || HAS_SENSOR_2 || HAS_SENSOR_3)

uint8_t sensor_mask(uint8_t sensor_no);
uint8_t *sensor_read(uint8_t sensor);
void sensor_init(void);
void IRAM_ATTR readPluviometer();
// void wdiRead();
uint16_t wdiCoordinates();
void anemometerWdiRead();
void setWindIRQ(void);
#endif

extern Ticker windcycler;

