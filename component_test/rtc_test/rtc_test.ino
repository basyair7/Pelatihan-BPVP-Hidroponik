#include <RTClib.h>

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println(F("RTC lost power, let's set the time!"));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // rtc.adjust(DateTime(2025, 12, 18, 19, 29, 0));
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long lastTime = 0;

  if (millis() - lastTime > 3000U) {
    lastTime = millis();

    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(F(" ("));
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.println(F(") "));
    
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(F("Temperature : "));
    Serial.print(rtc.getTemperature());
    Serial.println(F(" C"));
  }

}
