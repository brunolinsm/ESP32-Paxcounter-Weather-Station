// Basic Config
#include "sensor.h"
#include "time.h"
#include "timekeeper.h"

// Local logging tag
static const char TAG[] = __FILE__;

Ticker windcycler;

// Pluviometer variables
float bucketAmount = 0.25;                // ml equivalent of the trip tipping-bucket
float hourlyRain = 0.0;                   // rain accumulated for one hour
volatile byte pluvCounter;                // pluviometer counter

// Wind Direction Indicator variables
uint16_t wdiMeas = 0;
uint16_t wdiValue = 0;
// char* wdiText;
uint16_t wdiResult = 0;
const int8_t size = 12;
uint16_t wdiReads[size] = {0,0,0,0,0,0,0,0,0,0,0,0};
float wdiReadsSum = 0;
uint16_t wdiReads3sAvg = 0;
uint32_t wdi10mSum = 0;
uint16_t wdi10mAvg = 0;

// Wind speed variables
float RPM;
float windSpeed3sAvg = 0;
float lastWindSpeed3sAvg = 0;
float windGust;
float windSpeed10mSum = 0;
float windSpeed10mAvg = 0;
// volatile byte anemCounter;
uint8_t anemCounter;
bool anemState;
bool lastAnemState;
uint8_t anemReads[size] = {0,0,0,0,0,0,0,0,0,0,0,0};
float anemReadsSum = 0;
float anemReadsAvg = 0;
uint16_t avgCounter = 0;

void setWindIRQ() { xTaskNotify(irqHandlerTask, WINDCYCLE_IRQ, eSetBits); }

#define SENSORBUFFER                                                           \
  10 // max. size of user sensor data buffer in bytes [default=20]

void IRAM_ATTR readPluviometer(){
  pluvCounter++;                             
}

// void IRAM_ATTR readAnemometer(){
//   anemCounter++;
// }

void sensor_init(void) {

  // this function is called during device startup
  // put your user sensor initialization routines here
  pinMode(PLUV_RST_PIN,OUTPUT);
  pinMode(PLUV_PIN,INPUT);
  pinMode(WDI_PIN,INPUT);
  pinMode(ANEM_PIN,INPUT);
  digitalWrite(PLUV_RST_PIN,HIGH);            // Enable optocoupler and SCR
  attachInterrupt(digitalPinToInterrupt(PLUV_PIN), PluvIRQ, RISING);
  // attachInterrupt(digitalPinToInterrupt(ANEM_PIN), anemometerCounter, RISING);
  // detachInterrupt(ANEM_PIN);
  windcycler.attach(WINDCYCLE, setWindIRQ);
  
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
    
    // wdi10mAvg = wdi10mSum / 600;
    // ESP_LOGI(TAG, "Wind Direction Indicator Average %d°", wdi10mAvg); 
    // buf[0] = length;
    // buf[1] = lowByte(wdi10mAvg);
    // buf[2] = highByte(wdi10mAvg);

    // wdi10mSum = 0;  // reset sum
    break;


  case 3:      //Anemometer
    
    ESP_LOGI(TAG, "Number of wind samples: %d", avgCounter); 
    windSpeed10mAvg = windSpeed10mSum / avgCounter;
    wdi10mAvg = wdi10mSum / avgCounter;
    ESP_LOGI(TAG, "Wind Speed Average: %.2f m/s | Wind Direction Indicator Average: %d°", windSpeed10mAvg, wdi10mAvg); 
    windSpeed10mAvg = windSpeed10mAvg * 100;
    windGust = windGust * 100;                         // to transport this as byte via Lora 
    buf[0] = 6;
    buf[1] = lowByte((uint16_t)windSpeed10mAvg);
    buf[2] = highByte((uint16_t)windSpeed10mAvg);
    buf[3] = lowByte((uint16_t)windGust);
    buf[4] = highByte((uint16_t)windGust);
    buf[5] = lowByte(wdi10mAvg);
    buf[6] = highByte(wdi10mAvg);
    
    windSpeed10mSum = 0;    // reset sum
    wdi10mSum = 0;          // reset sum
    lastWindSpeed3sAvg = 0; // reset last wind gust 
    windGust = 0;           // reset wind gust
    avgCounter = 0;         // reset counter
    break;
  }

  return buf;
}


uint16_t wdiCoordinates(){
  wdiMeas = analogRead(WDI_PIN);
  // wdiText = "";
    
  if (wdiMeas == 0){
    wdiValue = 315;                                       // NORTHWEST
    // wdiText = "NORTHWEST";
  }
  else if ((wdiMeas >= 5) && (wdiMeas <= 100)){
    wdiValue = 270;                                       // WEST
    // wdiText = "WEST";
  }
  else if ((wdiMeas >= 101) && (wdiMeas <= 300)){
    wdiValue = 225;                                       // SOUTHWEST
    // wdiText = "SOUTHWEST";
  }
  else if ((wdiMeas >= 301) && (wdiMeas <= 500)){
    wdiValue = 180;                                       // SOUTH
    // wdiText = "SOUTH";
  }
  else if ((wdiMeas >= 501) && (wdiMeas <= 700)){
    wdiValue = 135;                                       // SOUTHEAST
    // wdiText = "SOUTHEAST";
  }
  else if ((wdiMeas >= 701) && (wdiMeas <= 900)){
    wdiValue = 90;                                        // EAST
    // wdiText = "EAST";
  }
  else if ((wdiMeas >= 901) && (wdiMeas <= 1000)){
    wdiValue = 45;                                        // NORTHEAST
    // wdiText = "NORTHEAST";
  }
  else{
    wdiValue = 0;                                         // NORTH
    // wdiText = "NORTH";
  }

  return wdiValue;
}


// void wdiRead(){
//   for (int8_t a = size-1; a > 3; a--){	
// 		wdiReads[a] = wdiReads[a-4];		
//   }  // shift array to the right 4 positions

//   for(int8_t b = 0; b < 4; b++){
//     for(uint32_t timeRead = millis(); (millis()-timeRead) < 250;){
//       wdiResult = wdiCoordinates();
//     }
//     wdiReads[b] = wdiResult;
//   } // update 4 initial positions of array

//   wdiReadsSum = 0;
//   for (int8_t c = 0; c < size; c++) {
//       wdiReadsSum += wdiReads[c];
//   }  
//   wdiReads3sAvg = (uint16_t)(wdiReadsSum/size);  // moveable average of coordinates (3 second window)

//   wdi10mSum += wdiReads3sAvg;  // sum of all measurements in 10 minutes
//   ESP_LOGI(TAG, "Wind Direction Indicator 3s %d", wdiReads3sAvg); 
// }


void anemometerWdiRead(){
  for (int8_t w = size-1; w > 3; w--){	
		anemReads[w] = anemReads[w-4];
    wdiReads[w] = wdiReads[w-4];		
  }  // shift array to the right 4 positions

  for(int8_t i = 0; i < 4; i++){
    for(uint32_t tRead = millis(); (millis()-tRead) < 250;){
      anemState = digitalRead(ANEM_PIN);
      if (anemState != lastAnemState){
        anemCounter++;
        lastAnemState = anemState;
      } // counter
      wdiResult = wdiCoordinates();
    }
    anemReads[i] = anemCounter;
    anemCounter = 0;
    wdiReads[i] = wdiResult;
  } // update 4 initial positions of array

  anemReadsSum = 0;
  wdiReadsSum = 0;
  for (int8_t z = 0; z < size; z++) {
      anemReadsSum += anemReads[z];
      wdiReadsSum += wdiReads[z];
      // ESP_LOGI(TAG, "List %d", readsAnem[z]);
  }  
  anemReadsAvg = anemReadsSum/size;              // moveable average of pulses (3 second window)
  wdiReads3sAvg = (uint16_t)(wdiReadsSum/size);  // moveable average of coordinates (3 second window)

  RPM=(anemReadsAvg*60)/3;
  windSpeed3sAvg = ((30.35 * RPM) / 1000);
  
  if (windSpeed3sAvg > lastWindSpeed3sAvg){
    windGust = windSpeed3sAvg;
    lastWindSpeed3sAvg = windSpeed3sAvg;
  } // read wind gust

  windSpeed10mSum += windSpeed3sAvg;  // sum of all measurements in 10 minutes
  wdi10mSum += wdiReads3sAvg;         // sum of all measurements in 10 minutes
  avgCounter += 1;                    // counter to calculate average values
  ESP_LOGI(TAG, "Number of wind samples: %d | Wind speed 3s: %.2f m/s | Wind Gust: %.2f m/s | Wind Direction Indicator: %d", avgCounter, windSpeed3sAvg, windGust, wdiReads3sAvg); 
}
