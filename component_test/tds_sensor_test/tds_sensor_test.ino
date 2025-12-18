#define TdsSensorPin    34
#define VREF            3.3   // analog reference voltage of the ADC
#define ADC_RESOLUTION  4096  // for esp32 : 4096, for arduino/esp8266 : 1023
#define SCOUNT          10    // sum of sample point

/****** TDS Variable ******/
int analogBuffer[SCOUNT];   // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0.0, tdsValue = 0.0, temperature = 16.0;
float ppm = 0.0;

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
  if(millis() - analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
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
      Serial.print(" | PPM: "); Serial.println(ppm);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(TdsSensorPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  run_tds_program();
}
