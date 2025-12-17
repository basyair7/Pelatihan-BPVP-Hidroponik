#include "config.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

/******* LCD I2C Config *******/
#define ROWS_LCD 20
#define COLS_LCD 4
LiquidCrystal_I2C lcd(0x27, ROWS_LCD, COLS_LCD);

/******* RTC I2C Config *******/
RTC_DS3231  rtc;
char daysOfTheWeek[7][12] = {
  "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"
};

/******* DHT Object *******/
DHT dht(DHTPIN, DHTTYPE);
float t_dht, h_dht;
unsigned long lastTime_dht      = 0;
const unsigned int interval_dht = 1000;

/******* DS18B20 Object *******/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DSsensor(&oneWire);
float Wtemp;

/******* TDS Variable *******/
int analogBuffer[SCOUNT];   // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0.0, tdsValue = 0.0, temperature = 16.0;
float ppm = 0.0;

/******* RTC Variable *******/
DateTime startDate;   // Variable untuk menyimpan tanggal mulai penyiraman
unsigned long lastSensorRead = 0; // Waktu terakhir sensor dibaca
bool isPlanted = false; // status kondisi penyiraman (apakah sudah disiram)
int hst = 0;
String fase = "-";

// Tambahkan waktu untuk delay 10 detik setelah pembacaan PPM
unsigned long lastPPMUpdate = 0;
bool ppmReady = false;
bool running = false;

// Tambahkan variable millis untuk kontrol module relay
unsigned long relayOnTime   = 0;
unsigned long lastRelayTime = 0;
int relayState = 0; // 0 = IDLE, 1 = RELAY1 ON, 2 =  RELAY2 ON

// inisialisasikan fungsi program
void btn_control_start(DateTime now);
void btn_control_reset();

void screen_state_program(int screen_state, DateTime now);
int getMedianNum(int bArray[], int iFilterLen);
void run_tds_program();
void run_ds_program();
void run_dht_program();
void kontrolPompa(float ppm, int hst);

/***** Configurasi Blynk *****/
// set password to "" for open networks
char ssid[] = SSID_STA;
char pass[] = PASS_STA;

BlynkTimer timer;

BLYNK_WRITE(V0) {
  if (param.asInt() == 1) {
    btn_control_reset();
  }
}

BLYNK_WRITE(V1) {
  if (param.asInt() == 1) {
    DateTime now = rtc.now();
    btn_control_start(now);
  }
}

void sendData_T_DHT() {
  static unsigned long lastTime_SendData = 0;
  if (millis() - lastTime_SendData > 5000U) {
    lastTime_SendData = millis();
    Blynk.virtualWrite(V2, t_dht);
  }
}

void sendData_H_DHT() {
  static unsigned long lastTime_SendData = 0;
  if (millis() - lastTime_SendData > 5000U) {
    lastTime_SendData = millis();
    Blynk.virtualWrite(V3, h_dht);
  }
}

void sendData_DS() {
  static unsigned long lastTime_SendData = 0;
  if (millis() - lastTime_SendData > 5000U) {
    lastTime_SendData = millis();
    Blynk.virtualWrite(V4, temperature);
  }
}

void sendData_TDS() {
  static unsigned long lastTime_SendData = 0;
  if (millis() - lastTime_SendData > 5000U) {
    lastTime_SendData = millis();
    Blynk.virtualWrite(V5, ppm);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  Wire.begin(22, 21);   // SDA = 22, SCL = 21 (DEFAULT ESP32)
  Wire.setClock(100000); // 100kHz

  /****** Deklarasi RTC ******/
  if (!rtc.begin()) {
    Serial.println();
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while(1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  delay(1000);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi...");
    Serial.println(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connection : ");
    lcd.setCursor(0, 1);
    lcd.print("Successfully");
    delay(1000);
    lcd.clear();
  }

  /****** SplashScreen ******/
  lcd.setCursor(0, 1);
  lcd.print("     HIDROPONIK     ");
  lcd.setCursor(0, 2);
  lcd.print("   BPVP BANDA ACEH   ");
  delay(2000);
  lcd.clear();

  /****** Deklarasi DHT & sensor DS ******/
  dht.begin();
  DSsensor.begin();

  /****** Deklarasi TDS ******/
  pinMode(TdsSensorPin, INPUT);

  /****** Tombol Relay ******/
  EEPROM.begin(32); // Inisialisasikan EEPROM dengan 32 byte

  pinMode(startButton, INPUT_PULLUP); // Tombol start menggunakan internal pull-up
  pinMode(resetButton, INPUT_PULLUP); // Tombol reset menggunakan internal pull-up
  pinMode(RELAY1, OUTPUT); // Pin Relay A sebagai OUTPUT
  pinMode(RELAY2, OUTPUT); // Pin Relay B sebagai OUTPUT
  digitalWrite(RELAY1, HIGH); // Awal relay 1 mati
  digitalWrite(RELAY2, HIGH); // Awal relay 2 mati

  // Jika flag EEPROM bernilai 1 berarti ada data tanam
  if (EEPROM.read(0) == 1) {
    int year  = EEPROM.read(1) * 256 + EEPROM.read(2); // Gabungkan 2 byte untuk tahun
    int month = EEPROM.read(3); // Baca bulan
    int day   = EEPROM.read(4); // Baca tanggal
    startDate = DateTime(year, month, day, 0, 0, 0); // Simpan tanggal mulai
    isPlanted = true; // Tandai bahwa sudah ditanam
  }

  timer.setInterval(5000L, sendData_T_DHT);
  timer.setInterval(5000L, sendData_H_DHT);
  timer.setInterval(5000L, sendData_DS);
  timer.setInterval(5000L, sendData_TDS);

}

void loop() {
  // put your main code here, to run repeatedly:
  /*****Program RTC Tahun dan Tanggal*****/
  static unsigned long lastTime_main;
  static int screen_state = 0;
  DateTime now = rtc.now();

  if (millis() - lastTime_main > 1000U) {
    lastTime_main = millis();
    lcd.clear();
    if (screen_state >= 6) screen_state = 0;
    screen_state++;
  }
  
  screen_state_program(screen_state, now);
  run_dht_program();
  run_ds_program();
  run_tds_program();

  if (ppmReady && millis() - lastPPMUpdate >- 10000 && millis() - lastRelayTime >= 10000) {
    kontrolPompa(ppm, hst);
    ppmReady = false; // reset supaya tidak berulang
    lastRelayTime = millis(); // catat waktu relay terakhir
  }

  if (digitalRead(startButton) == LOW && !isPlanted) {
    btn_control_start(now);
  }

  if (digitalRead(resetButton) == LOW) {
    btn_control_reset();
  }

  if (isPlanted) {
    TimeSpan duration = now -startDate;
    hst = duration.days() + 1;
    if (hst >= 1 && hst <= 7)         fase = "1";
    else if (hst >= 8 && hst <= 14)   fase = "2";
    else if (hst >= 15 && hst <= 21)  fase = "3";
    else if (hst >= 22 && hst <= 35)  fase = "4";
    else if (hst > 35)                fase = "5";
  }
  else {
    hst = 0; // reset HST ketika belum tanam
  }

  kontrolPompa(ppm, hst);
}

void screen_state_program(int screen_state, DateTime now) {
  if (screen_state == 1) {
    lcd.setCursor(0, 0);
    if (now.day() < 10) lcd.print("0");
    lcd.print(now.day(), DEC);
    lcd.print('/');
    if (now.month() < 10) lcd.print("0");
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.year(), DEC);

    lcd.setCursor(11, 0);
    if (now.hour() < 10) lcd.print("0");
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    if (now.minute() < 10) lcd.print("0");
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    if (now.second() < 10) lcd.print("0");
    lcd.print(now.second(), DEC);
  }
  else if (screen_state == 2) {
    lcd.setCursor(0, 1);
    lcd.print("   DHT Data   ");
    lcd.setCursor(0, 1);
    lcd.print("H : ");
    lcd.print(h_dht);
    lcd.print("%");
    lcd.setCursor(0, 2);
    lcd.print("T : ");
    lcd.print(t_dht, 0);
    lcd.print((char)223);
    lcd.print("C");
  }
  else if (screen_state == 3) {
    lcd.setCursor(0, 0);
    lcd.print("   DS Data   ");
    lcd.setCursor(0, 1);
    lcd.print("WT : ");
    lcd.print(temperature, 0);
    lcd.print((char)223);
    lcd.print("C");
  }
  else if (screen_state == 4) {
    lcd.setCursor(0, 0);
    lcd.print("   TDS Data   ");
    lcd.setCursor(0, 1);
    lcd.print("TDS Value : ");
    lcd.print(tdsValue);
    lcd.print("ppm");
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print("Fase : "); 
    lcd.print(fase);
    lcd.setCursor(0, 2);
    lcd.print("HST : ");
    lcd.print(hst);
  }
}

void btn_control_start(DateTime now) {
  startDate = now;
  EEPROM.write(0, 1);
  EEPROM.write(1, startDate.year() / 256);
  EEPROM.write(2, startDate.year() % 256);
  EEPROM.write(3, startDate.month());
  EEPROM.write(4, startDate.day());
  EEPROM.commit();
  isPlanted = true;
  delay(1000);
}

void btn_control_reset() {
  EEPROM.write(0, 0);
  EEPROM.commit();
  isPlanted = false;
  digitalWrite(relayA, HIGH);
  digitalWrite(relayB, HIGH);
  delay(1000);
  lcd.clear();
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void run_tds_program() {
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++; // nilai analogBufferIndex ditambah 1 per 40 miliseconds
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)(VREF / ADC_RESOLUTION);
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage = averageVoltage/compensationCoefficient;
      
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      // Serial.print("TDS Value:");
      // Serial.print(tdsValue,0);
      // Serial.println("ppm");

      ppm = tdsValue;
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      Serial.print("ppm");
      Serial.print(" | PPM: "); Serial.print(ppm);
      Serial.print(" | Sensor: "); Serial.print(ppmReady);
      Serial.print(" | HST: "); Serial.print(hst);
      Serial.print(" | Nyala? "); Serial.println(running); 
      lcd.setCursor(0,2);
      lcd.print("PPM: ");
      lcd.print(ppm,0);
      lcd.print(" ");

      ppmReady = true;
      lastPPMUpdate = millis();
    }
  }
}

void run_ds_program() {
  static unsigned long lastTime_sample_point = millis();
  if (millis() - lastTime_sample_point > 1000U) {
    lastTime_sample_point = millis();
    DSsensor.setResolution(12);
    DSsensor.requestTemperatures();
    temperature = DSsensor.getTempCByIndex(0);
  }
}

void run_dht_program() {
  if (millis() - lastTime_dht > interval_dht) {
    lastTime_dht = millis();
    t_dht = dht.readTemperature();
    h_dht = dht.readHumidity();
  }
}

void kontrolPompa(float ppm, int hst) {
  // Logika fase & PPM
  running = false;
  if (hst >= 1 && hst <= 7 && ppm < 400) running = true;
  else if (hst >= 8 && hst <= 14 && ppm <= 600) running = true; 
  else if (hst >= 15 && hst <= 21 && ppm <= 800) running = true;
  else if (hst >= 22 && hst <= 35 && ppm <= 1100) running = true;
  else if (hst > 35 && ppm < 1200) running = true;

  // siklus relay dengan millis (non-blocking)
  if (running && relayState == 0) {
    digitalWrite(relayA, LOW);
    relayOnTime = millis();
    relayState = 1;
    Serial.println("Relay A ON");
  }

  if (relayState == 1 && millis() - relayOnTime >= 500) {
    digitalWrite(relayA, HIGH);
    digitalWrite(relayB, LOW);
    relayOnTime = millis();
    relayState = 2;
    Serial.println("Relay A OFF, Relay B ON");
  }

  if (relayState == 2 && millis() - relayOnTime >= 500) {
    digitalWrite(relayB, HIGH);
    relayState = 0;
    Serial.println("Relay B OFF, Selesai siklus");
  }

  if (!running && relayState == 0) {
    digitalWrite(relayA, HIGH);
    digitalWrite(relayB, HIGH);
  }
}