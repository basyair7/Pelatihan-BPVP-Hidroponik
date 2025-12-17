#include <Arduino.h>

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID     ""
#define BLYNK_TEMPLATE_NAME   "Quickstart Template"
#define BLYNK_AUTH_TOKEN      ""

#define BLYNK_PRINT Serial

#define SSID_STA             "elektronika_AP"
#define PASS_STA             "elektronika@2025"

/****** DHT SENSOR *******/
#define DHTPIN      2
#define DHTTYPE     DHT11

/******* DS18B20 *******/
#define ONE_WIRE_BUS  4 // pininput DS18B20

/******* TDS SENSOR *******/
#define TdsSensorPin    34
#define VREF            3.3   // analog reference voltage of the ADC
#define ADC_RESOLUTION  4096  // for ESP 4096, for arduino 1023
#define SCOUNT          10    // sum of sample point

/******* RELAY & PUSH BUTTON *******/
#define RELAY1        25  // Relay pompa A atau pompa 1
#define RELAY2        26  // Relay pompa B atau pompa 2
#define startButton   32  // Tombol mulai sistem
#define resetButton   33  // Tombol reset sistem
#define relayA        RELAY1
#define relayB        RELAY2
