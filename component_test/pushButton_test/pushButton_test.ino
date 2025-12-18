#define startButton 32
#define resetButton 33

bool isPlanted = false;

void handleButtons() {
  static bool startState = HIGH, lastStartState = HIGH;
  static bool resetState = HIGH, lastResetState = HIGH;

  static unsigned long lastStartDebounce = 0;
  static unsigned long lastResetDebounce = 0;
  const unsigned long debounceDelay = 50;

  bool startRead = digitalRead(startButton);
  bool resetRead = digitalRead(resetButton);

  // ===== START BUTTON =====
  if (startRead != lastStartState) {
    lastStartDebounce = millis();
  }

  if ((millis() - lastStartDebounce) > debounceDelay) {
    if (startRead != startState) {
      startState = startRead;

      if (startState == LOW && !isPlanted) {
        Serial.println();
        Serial.println(F("System Start!"));
      }
    }
  }

  lastStartState = startRead;

  // ===== RESET BUTTON =====
  if (resetRead != lastResetState) {
    lastResetDebounce = millis();
  }

  if ((millis() - lastResetDebounce) > debounceDelay) {
    if (resetRead != resetState) {
      resetState = resetRead;

      if (resetState == LOW) {
        Serial.println();
        Serial.println(F("System Reset"));
        isPlanted = false;
      }
    }
  }

  lastResetState = resetRead;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(startButton, INPUT_PULLUP);
  pinMode(resetButton, INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:
  handleButtons();
}
