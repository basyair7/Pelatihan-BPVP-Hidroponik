#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS  4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DSsensor(&oneWire);
float temperature;

void run_ds_program() {
  static unsigned long lastTime_sample_point = millis();
  if (millis() - lastTime_sample_point > 1000U) {
    lastTime_sample_point = millis();
    DSsensor.setResolution(12);
    DSsensor.requestTemperatures();
    temperature = DSsensor.getTempCByIndex(0);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  DSsensor.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long lastTime = millis();
  run_ds_program();
  if (millis() - lastTime > 1000U) {
    lastTime = millis();
    Serial.print(F("DS Temp : "));
    Serial.print(temperature);
    Serial.println(F(" C"));
  }
}
