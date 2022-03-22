// Basic Config
#include "globals.h"
#include "sensor.h"

// Local logging tag
static const char TAG[] = __FILE__;

// Pluviometer variables
float bucketAmount = 0.25;                // ml equivalent of the trip tipping-bucket
float dailyRain = 0.0;                    // rain accumulated for the day
float hourlyRain = 0.0;                   // rain accumulated for one hour
float dailyRain_till_LastHour = 0.0;      // rain accumulated for the day till the last hour

// Wind Direction Indicator variables
uint16_t wdiRead = 0;
uint16_t wdiValue = 0;

// Wind speed variables
float RPM;
float windSpeed;
volatile byte counter;

#define SENSORBUFFER                                                           \
  10 // max. size of user sensor data buffer in bytes [default=20]

void IRAM_ATTR readPluviometer(){
  dailyRain+=bucketAmount;                // Update the pluviometer read
}

void IRAM_ATTR readAnemometer(){
  counter++;                              // Update the anemometer read
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
  uint8_t length = 3;

  switch (sensor) {

  case 1:    //Pluviometer

  //   if(now.minute() == 0){
  //     hourlyRain = dailyRain - dailyRain_till_LastHour;    // calculate the last hour's rain
  //     dailyRain_till_LastHour = dailyRain;                 // update the rain till last hour for next calculation
  //   }

  //   if(now.hour()== 0) {
  //     dailyRain = 0.0;                                     // clear daily-rain at midnight
  //     dailyRain_till_LastHour = 0.0;                       // we do not want negative rain at 01:00
  // }  
      
    detachInterrupt(PLUV_PIN);
    ESP_LOGI(TAG, "Hourly Rain %.2f mm/h | Daily Rain %.2f mm/d", hourlyRain, dailyRain);
    attachInterrupt(digitalPinToInterrupt(PLUV_PIN), PluvIRQ, RISING);   

    buf[0] = length;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
    break;

  case 2:    //Wind Direction Indicator (WDI)
    
    wdiRead = analogRead(WDI_PIN);

    if (wdiRead == 0){
      wdiValue = 315;                                       // NORTHWEST
    }
    else if ((wdiRead >= 5) && (wdiRead <= 100)){
      wdiValue = 270;                                       // WEST
    }
    else if ((wdiRead >= 101) && (wdiRead <= 300)){
      wdiValue = 225;                                       // SOUTHWEST
    }
    else if ((wdiRead >= 301) && (wdiRead <= 500)){
      wdiValue = 180;                                       // SOUTH
    }
    else if ((wdiRead >= 501) && (wdiRead <= 700)){
      wdiValue = 135;                                       // SOUTHEAST
    }
    else if ((wdiRead >= 701) && (wdiRead <= 900)){
      wdiValue = 90;                                        // EAST
    }
    else if ((wdiRead >= 901) && (wdiRead <= 1000)){
      wdiValue = 45;                                        // NORTHEAST
    }
    else{
      wdiValue = 0;                                         // NORTH
    }

    ESP_LOGI(TAG, "WDI Read %d | Wind Direction %d°", wdiRead, wdiValue);

    buf[0] = length;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
    break;

  case 3:      //Anemometer
                  
    RPM = counter;                              // 1min cycle interval (counter=revolutions per minute)
    windSpeed = ((30.35 * RPM) / 1000) * 3.6;   // Calculate wind speed on km/h
    detachInterrupt(ANEM_PIN);
    ESP_LOGI(TAG, "Wind Speed %.2f km/h | Pulses %.d", windSpeed, counter);
    windSpeed = 0;
    counter = 0;   
    attachInterrupt(digitalPinToInterrupt(ANEM_PIN), AnemIRQ, RISING); 
    
    buf[0] = length;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
    break;
  }

  return buf;
}