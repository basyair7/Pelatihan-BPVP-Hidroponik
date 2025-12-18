#include <LiquidCrystal_I2C.h>
// #include <LCD_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
// LCD_I2C lcd(0x27, 20, 4);

void setup() {
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned int count = 0;
  static unsigned long lastTime = 0;

  if (count > 10) {
    count = 0;
  }

  if (millis() - lastTime > 1000U) {
    lastTime = millis();
    
    
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Hello, World!");
    lcd.setCursor(2, 1);
    lcd.print("Test Program");
    lcd.setCursor(0, 2);
    lcd.print("DOIT ESP32 DEVKIT V1");
    lcd.setCursor(0, 3);
    lcd.print(count);
    count++;
    
  }

}
