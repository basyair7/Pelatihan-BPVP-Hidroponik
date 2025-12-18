#include <Arduino.h>

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID     ""
#define BLYNK_TEMPLATE_NAME   "Quickstart Template"
#define BLYNK_AUTH_TOKEN      ""

#define BLYNK_PRINT Serial

#define SSID_STA             "elektronika_AP"
#define PASS_STA             "elektronika@2025"

#define EEPROM_SIZE 32

/****** DHT SENSOR *******/
#define DHTPIN          2         // Pin Input DHT
#define DHTTYPE         DHT11     // DHT22, DHT21, DHT11

/******* DS18B20 *******/
#define ONE_WIRE_BUS    4       // Pin Input DS18B20

/******* TDS SENSOR *******/
#define TdsSensorPin    34    // Pin Input TDS Sensor
#define VREF            3.3   // analog reference voltage of the ADC
#define ADC_RESOLUTION  4096  // for esp32 : 4096, for arduino/esp8266 : 1023
#define SCOUNT          10    // sum of sample point

/******* RELAY & PUSH BUTTON *******/
#define RELAY1          25      // Pin Output Relay A / Pump 1
#define RELAY2          26      // Pin Output Relay B / Pump 2
#define startButton     18      // Pin Input Start Button Sistem
#define resetButton     19      // Pin Input Reset Button Sistem
#define relayA        RELAY1
#define relayB        RELAY2
