#include <Wire.h>

#define MAX_DEVICES 10

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
  Serial.println(F("Scanning...."));
  Serial.println();

  static byte addresses[MAX_DEVICES];
  static byte unknown_addresses[MAX_DEVICES];
  bool unknown_device_found = false;
  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      // Serial.print(F("I2C device found at address : 0x"));
      // if (address < 16) Serial.print(F("0"));
      // Serial.print(address, HEX);
      // Serial.println();
      // nDevices++;

      if (nDevices < MAX_DEVICES) {
        addresses[nDevices] = address;
        nDevices++;
      }
    }
    else if (error == 4) {
      // Serial.print(F("Unknown error at address : 0x"));
      // if (address < 16) Serial.print(F("0"));
      // Serial.println(address, HEX);
      unknown_device_found = true;
      if (nDevices < MAX_DEVICES) {
        unknown_addresses[nDevices] = address;
        nDevices++;
      }
    }
  }

  Serial.println(F("Scan done!"));

  if (nDevices == 0) {
    Serial.println(F("No I2C devices found"));
  }
  else {
    Serial.print(F("Found devices at address : "));
    Serial.println(nDevices);
    Serial.println(F("Device found : "));
    for (int i = 0; i < nDevices; i++) {
      Serial.print(F("0x"));
      if (addresses[i] < 10) Serial.print(F("0"));
      Serial.println(addresses[i], HEX);
    }

    if (unknown_device_found) {
      Serial.println(F("Unknown Device :"));
      for (int i = 0; i < nDevices; i++) {
        Serial.print(F("0x"));
        if (unknown_addresses[i] < 10) Serial.print(F("0"));
        Serial.println(unknown_addresses[i], HEX);
      }
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  // Nothing
}
