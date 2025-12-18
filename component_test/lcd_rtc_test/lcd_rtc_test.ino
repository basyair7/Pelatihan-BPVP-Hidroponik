#include <RTClib.h>
#include <LCD_I2C.h>
#include <Wire.h>

#define LCD_ROWS 20
#define LCD_COLS 4
LCD_I2C lcd(0x27, 20, 4);
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
  Wire.begin();
  lcd.begin();
  lcd.backlight();

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    lcd.setCursor(0, 0);
    lcd.print("Couldn't find RTC");
    while(1) delay(50);
  }

  if (rtc.lostPower()) {
    Serial.println(F("RTC lost power, let's set the time!"));
    lcd.setCursor(0, 0);
    lcd.print("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long lastTime = 0;

  if (millis() - lastTime > 3000U) {
    lastTime = millis();

    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    if (now.month() < 10) lcd.print("0");
    Serial.print(now.month(), DEC);
    Serial.print('/');
    if (now.day() < 10) lcd.print("0");
    Serial.print(now.day(), DEC);
    Serial.print(F(" ("));
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.println(F(") "));

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(now.year(), DEC);
    lcd.print('/');
    if (now.month() < 10) lcd.print("0");
    lcd.print(now.month(), DEC);
    lcd.print('/');
    if (now.day() < 10) lcd.print("0");
    lcd.print(now.day(), DEC);
    lcd.setCursor(0, 1);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    
    if (now.hour() < 10) lcd.print("0");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    if (now.minute() < 10) lcd.print("0");
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    if (now.second() < 10) lcd.print("0");
    Serial.print(now.second(), DEC);
    Serial.println();

    lcd.setCursor(0, 2);
    if (now.hour() < 10) lcd.print("0");
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    if (now.minute() < 10) lcd.print("0");
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    if (now.second() < 10) lcd.print("0");
    lcd.print(now.second(), DEC);

    Serial.print(F("Temperature : "));
    Serial.print(rtc.getTemperature());
    Serial.println(F(" C"));

    lcd.setCursor(0, 3);
    lcd.print("T : ");
    lcd.print(rtc.getTemperature());
    lcd.print(" C");
  }
}
