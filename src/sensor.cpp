// Basic Config
#include "globals.h"
#include "sensor.h"
#include "time.h"
#include "timekeeper.h"

// Local logging tag
static const char TAG[] = __FILE__;

// Pluviometer variables
float bucketAmount = 0.25;                // ml equivalent of the trip tipping-bucket
float hourlyRain = 0.0;                   // rain accumulated for one hour
volatile byte pluvCounter;                // pluviometer counter
time_t currentSec;                        // timestamp
struct tm now = {0};                      // UTC time

// Wind Direction Indicator variables
uint16_t wdiRead = 0;
uint16_t wdiValue = 0;
char* wdiText;

// Wind speed variables
float RPM;
float windSpeed;
volatile byte anemCounter;

#define SENSORBUFFER                                                           \
  10 // max. size of user sensor data buffer in bytes [default=20]

void IRAM_ATTR readPluviometer(){
  pluvCounter++;                             
}

void IRAM_ATTR readAnemometer(){
  anemCounter++;                              
}

void sensor_init(void) {

  // this function is called during device startup
  // put your user sensor initialization routines here
  pinMode(PLUV_RST_PIN,OUTPUT);
  pinMode(PLUV_PIN,INPUT);
  pinMode(WDI_PIN,INPUT);
  pinMode(ANEM_PIN,INPUT);
  digitalWrite(PLUV_RST_PIN,HIGH);            // Enable optocoupler and SCR
  attachInterrupt(digitalPinToInterrupt(PLUV_PIN), PluvIRQ, RISING);
  attachInterrupt(digitalPinToInterrupt(ANEM_PIN), AnemIRQ, RISING); 
  
}

uint8_t sensor_mask(uint8_t sensor_no) {
  switch (sensor_no) {
  case 0:
    return (uint8_t)COUNT_DATA;
  case 1:
    return (uint8_t)SENSOR1_DATA;
  case 2:
    return (uint8_t)SENSOR2_DATA;
  case 3:
    return (uint8_t)SENSOR3_DATA;
  case 4:
    return (uint8_t)BATT_DATA;
  case 5:
    return (uint8_t)GPS_DATA;
  case 6:
    return (uint8_t)MEMS_DATA;
  case 7:
    return (uint8_t)RESERVED_DATA;
  default:
    return 0;
  }
}

uint8_t *sensor_read(uint8_t sensor) {

  static uint8_t buf[SENSORBUFFER] = {0};
  uint8_t length = 2;

  switch (sensor) {

  case 1:    //Pluviometer
    
    detachInterrupt(PLUV_PIN);
    hourlyRain = pluvCounter * bucketAmount;   // convert pulses in mm/d
    hourlyRain = hourlyRain * 100;             // to transport this as byte via Lora 
    ESP_LOGI(TAG, "Hourly Rain %.2f mm", hourlyRain);
    pluvCounter = 0;                           // pluviometer counter reset                         
    
    buf[0] = length;
    buf[1] = lowByte((uint16_t)hourlyRain);
    buf[2] = highByte((uint16_t)hourlyRain);
    
    attachInterrupt(digitalPinToInterrupt(PLUV_PIN), PluvIRQ, RISING); 
    break;
    
   
  case 2:    //Wind Direction Indicator (WDI)
    
    wdiRead = analogRead(WDI_PIN);
    wdiText = "";
    
    if (wdiRead == 0){
      wdiValue = 315;                                       // NORTHWEST
      wdiText = "NORTHWEST";
    }
    else if ((wdiRead >= 5) && (wdiRead <= 100)){
      wdiValue = 270;                                       // WEST
      wdiText = "WEST";
    }
    else if ((wdiRead >= 101) && (wdiRead <= 300)){
      wdiValue = 225;                                       // SOUTHWEST
      wdiText = "SOUTHWEST";
    }
    else if ((wdiRead >= 301) && (wdiRead <= 500)){
      wdiValue = 180;                                       // SOUTH
      wdiText = "SOUTH";
    }
    else if ((wdiRead >= 501) && (wdiRead <= 700)){
      wdiValue = 135;                                       // SOUTHEAST
      wdiText = "SOUTHEAST";
    }
    else if ((wdiRead >= 701) && (wdiRead <= 900)){
      wdiValue = 90;                                        // EAST
      wdiText = "EAST";
    }
    else if ((wdiRead >= 901) && (wdiRead <= 1000)){
      wdiValue = 45;                                        // NORTHEAST
      wdiText = "NORTHEAST";
    }
    else{
      wdiValue = 0;                                         // NORTH
      wdiText = "NORTH";
    }

    ESP_LOGI(TAG, "WDI Read %d | Wind Direction %d° (%s)", wdiRead, wdiValue, wdiText);

    buf[0] = length;
    buf[1] = lowByte(wdiValue);
    buf[2] = highByte(wdiValue);
    break;

  case 3:      //Anemometer

    detachInterrupt(ANEM_PIN);

    // Time control
    // currentSec = time(NULL);
    // localtime_r(&currentSec,&now);

    // if(now.tm_min == 0){
    // ESP_LOGI(TAG, "Current Time %d:%d:%d", now.tm_hour, now.tm_min, now.tm_sec);
    
    RPM = anemCounter;                          // 1min cycle interval (counter=revolutions per minute)
    windSpeed = ((30.35 * RPM) / 1000) * 3.6;   // Calculate wind speed on km/h
    ESP_LOGI(TAG, "Wind Speed %.2f km/h | Pulses %.d", windSpeed, anemCounter); 
    
    windSpeed = windSpeed * 100;                         // to transport this as byte via Lora 
    buf[0] = length;
    buf[1] = lowByte((uint16_t)windSpeed);
    buf[2] = highByte((uint16_t)windSpeed);

    windSpeed = 0;
    anemCounter = 0;   
    attachInterrupt(digitalPinToInterrupt(ANEM_PIN), AnemIRQ, RISING);
    break;
  }

  return buf;
}