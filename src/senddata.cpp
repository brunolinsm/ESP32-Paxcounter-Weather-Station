// Basic Config
#include "senddata.h"

// Local logging tag
static const char TAG[] = __FILE__;

time_t timestmp;                        // timestamp
struct tm utc_time = {0};               // UTC time

uint8_t wind_send_counter = 0;          // counter for sending wind data

uint8_t rain_send_counter = 0;          // counter for sending rainfall data

bool once = true;                       // activate ticker windcycle

void setSendIRQ(TimerHandle_t xTimer) {
  xTaskNotify(irqHandlerTask, SENDCYCLE_IRQ, eSetBits);
}

void setSendIRQ(void) { setSendIRQ(NULL); }

void initSendDataTimer(uint8_t sendcycle) {
  static TimerHandle_t SendDataTimer = NULL;

  if (SendDataTimer == NULL) {
    SendDataTimer =
        xTimerCreate("SendDataTimer", pdMS_TO_TICKS(sendcycle * 1000), pdTRUE,
                     (void *)0, setSendIRQ);
    xTimerStart(SendDataTimer, 0);
  } else {
    xTimerChangePeriod(SendDataTimer, pdMS_TO_TICKS(sendcycle * 1000), 0);
  }
}

// put data to send in RTos Queues used for transmit over channels Lora and SPI
void SendPayload(uint8_t port) {
  ESP_LOGD(TAG, "sending Payload for Port %d", port);

  MessageBuffer_t SendBuffer; // contains MessageSize, MessagePort, Message[]

  SendBuffer.MessageSize = payload.getSize();

  switch (PAYLOAD_ENCODER) {
  case 1: // plain -> no mapping
  case 2: // packed -> no mapping
    SendBuffer.MessagePort = port;
    break;
  case 3: // Cayenne LPP dynamic -> all payload goes out on same port
    SendBuffer.MessagePort = CAYENNE_LPP1;
    break;
  case 4: // Cayenne LPP packed -> we need to map some paxcounter ports
    SendBuffer.MessagePort = CAYENNE_LPP2;
    switch (SendBuffer.MessagePort) {
    case COUNTERPORT:
      SendBuffer.MessagePort = CAYENNE_LPP2;
      break;
    case RCMDPORT:
      SendBuffer.MessagePort = CAYENNE_ACTUATOR;
      break;
    case TIMEPORT:
      SendBuffer.MessagePort = CAYENNE_DEVICECONFIG;
      break;
    }
    break;
  default:
    SendBuffer.MessagePort = port;
  }
  memcpy(SendBuffer.Message, payload.getBuffer(), SendBuffer.MessageSize);

// enqueue message in device's send queues
#if (HAS_LORA)
  lora_enqueuedata(&SendBuffer);
#endif
#ifdef HAS_SPI
  spi_enqueuedata(&SendBuffer);
#endif
#ifdef HAS_MQTT
  mqtt_enqueuedata(&SendBuffer);
#endif
} // SendPayload

// timer triggered function to prepare payload to send
void sendData() {
  uint8_t bitmask = cfg.payloadmask;
  uint8_t mask = 1;

  // Time control
  timestmp = time(NULL);
  localtime_r(&timestmp,&utc_time);

  if (once){
    windcycler.attach(WINDCYCLE, setWindIRQ);
    once = false;
  }

#if (HAS_GPS)
  gpsStatus_t gps_status;
#endif
#if (HAS_SDS011)
  sdsStatus_t sds_status;
#endif
#if ((WIFICOUNTER) || (BLECOUNTER))
  struct count_payload_t count =
      count_from_libpax; // copy values from global libpax var
  ESP_LOGD(TAG, "Sending count results: pax=%d / wifi=%d / ble=%d", count.pax,
           count.wifi_count, count.ble_count);
#endif

  while (bitmask) {
    switch (bitmask & mask) {
#if ((WIFICOUNTER) || (BLECOUNTER))
    case COUNT_DATA:
      payload.reset();

#if !(PAYLOAD_OPENSENSEBOX)
      payload.addCount(count.wifi_count, MAC_SNIFF_WIFI);
      if (cfg.blescan) {
        payload.addCount(count.ble_count, MAC_SNIFF_BLE);
      }
#endif

#if (HAS_GPS)
      if (GPSPORT == COUNTERPORT) {
        // send GPS position only if we have a fix
        if (gps_hasfix()) {
          gps_storelocation(&gps_status);
          payload.addGPS(gps_status);
        } else
          ESP_LOGD(TAG, "No valid GPS position");
      }
#endif

#if (PAYLOAD_OPENSENSEBOX)
      payload.addCount(count.wifi_count, MAC_SNIFF_WIFI);
      if (cfg.blescan) {
        payload.addCount(count.ble_count, MAC_SNIFF_BLE);
#endif

#if (HAS_SDS011)
        sds011_store(&sds_status);
        payload.addSDS(sds_status);
#endif

#ifdef HAS_DISPLAY
        dp_plotCurve(count.pax, true);
#endif

#if (HAS_SDCARD)
        sdcardWriteData(count.wifi_count, count.ble_count
#if (defined BAT_MEASURE_ADC || defined HAS_PMU)
                        ,
                        read_voltage()
#endif
        );
#endif // HAS_SDCARD

        SendPayload(COUNTERPORT);
        break; // case COUNTDATA

#endif // ((WIFICOUNTER) || (BLECOUNTER))

#if (HAS_BME)
      case MEMS_DATA:
        // wind_send_counter += 1;
        // rain_send_counter += 1;
        // if (wind_send_counter < 10 && rain_send_counter < 59){
        if(((utc_time.tm_min % 10) != 0) && ((utc_time.tm_min % 59) != 0)){   // send every 10 minutes
          // Average of values
          bme_status.temperature = bme_status.temperature/bme_status.count_to_avg;
          bme_status.pressure = bme_status.pressure/bme_status.count_to_avg;
          bme_status.humidity = bme_status.humidity/bme_status.count_to_avg;
          payload.reset();
          payload.addBME(bme_status);
          SendPayload(BMEPORT);
          // ESP_LOGI(TAG, "Counter for sending wind data: %d | Number of BME samples: %d", wind_send_counter, bme_status.count_to_avg);
          ESP_LOGI(TAG, "Number of BME samples: %d", bme_status.count_to_avg);
          ESP_LOGI(TAG, "Current Time %d:%d:%d", utc_time.tm_hour, utc_time.tm_min, utc_time.tm_sec);
          
          // Reset
          bme_status.temperature = 0;
          bme_status.pressure = 0;
          bme_status.humidity = 0;
          bme_status.count_to_avg = 0;
          wind_send_counter = 0;
          rain_send_counter = 0;
          break;
        }
#endif

#if (HAS_GPS)
      case GPS_DATA:
        if (GPSPORT != COUNTERPORT) {
          // send GPS position only if we have a fix
          if (gps_hasfix()) {
            gps_storelocation(&gps_status);
            payload.reset();
            payload.addGPS(gps_status);
            SendPayload(GPSPORT);
          } else
            ESP_LOGD(TAG, "No valid GPS position");
        }
        break;
#endif

#if (HAS_SENSORS)
#if (HAS_SENSOR_1)
      case SENSOR1_DATA:
        // if(rain_send_counter == 59){   // send every 59 minutes
        if(((utc_time.tm_min % 59) == 0) && (rain_send_counter == 0)){   // send every 59 minutes
          payload.reset();
          payload.addSensor(sensor_read(1));
          SendPayload(SENSOR1PORT);
          rain_send_counter = 1;
          break;
        }
#endif
// #if (HAS_SENSOR_2)
//       case SENSOR2_DATA:
//         if(((uint8_t)(utc_time.tm_min) % 10) == 0){   // send every 10 minutes
//           payload.reset();
//           payload.addSensor(sensor_read(2));
//           SendPayload(SENSOR2PORT);
//           delay(2000);
//           break;
//         }
// #endif
#if (HAS_SENSOR_3)
      case SENSOR3_DATA:
        // if(wind_send_counter == 10){   // send every 10 minutes
        if(((utc_time.tm_min % 10) == 0) && (wind_send_counter == 0)){   // send every 10 minutes
          payload.reset();
          payload.addSensor(sensor_read(3));
          SendPayload(SENSOR3PORT);
          wind_send_counter = 1;
          break;
        }
#endif
#endif

#if (defined BAT_MEASURE_ADC || defined HAS_PMU)
      case BATT_DATA:
        payload.reset();
        payload.addVoltage(read_voltage());
        SendPayload(BATTPORT);
        break;
#endif
      } // switch
      bitmask &= ~mask;
      mask <<= 1;
    } // while (bitmask)
  }   // sendData()

  void flushQueues(void) {
    rcmd_queuereset();
#if (HAS_LORA)
    lora_queuereset();
#endif
#ifdef HAS_SPI
    spi_queuereset();
#endif
#ifdef HAS_MQTT
    mqtt_queuereset();
#endif
  }

  bool allQueuesEmtpy(void) {
    uint32_t rc = rcmd_queuewaiting();
#if (HAS_LORA)
    rc += lora_queuewaiting();
#endif
#ifdef HAS_SPI
    rc += spi_queuewaiting();
#endif
#ifdef HAS_MQTT
    rc += mqtt_queuewaiting();
#endif
    return (rc == 0) ? true : false;
  }