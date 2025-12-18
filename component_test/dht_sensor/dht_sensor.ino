#include <DHT.h>

#define DHTPIN  2
#define DHTTYPE DHT11

DHT dhtSensor(DHTPIN, DHTTYPE);
float t_dht, h_dht;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dhtSensor.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 2000U) {
    lastTime = millis();
    t_dht = dhtSensor.readTemperature();
    h_dht = dhtSensor.readHumidity();

    if (isnan(t_dht) || isnan(h_dht)) {
      Serial.println(F("DHT Error"));
      return;
    }

    Serial.print(F("T : "));
    Serial.print(t_dht);
    Serial.println(F(" C"));
    Serial.print(F("H : "));
    Serial.print(h_dht);
    Serial.println(F(" %"));
  }

}
