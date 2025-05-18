#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define MOISTURE_PIN A0
#define PUMP_PIN 7

unsigned long lastPumpTime = 0;
bool recentlyWatered = false;
const unsigned long waitDuration = 3600000UL;  // 1 hour

void setup() {
  Serial.begin(9600);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  Serial.println("System initialized.");
}

void loop() {
  unsigned long currentTime = millis();
  int moistureValue = analogRead(MOISTURE_PIN);
  int moisturePercent = map(moistureValue, 1023, 0, 0, 100);

  Serial.print("Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  if (recentlyWatered) {
    unsigned long elapsed = currentTime - lastPumpTime;
    if (elapsed < waitDuration) {
      int minutesLeft = (waitDuration - elapsed) / 60000;
      displayMoisture(moisturePercent, "Check again in:", String(minutesLeft) + " min");
      Serial.print("Waiting: ");
      Serial.print(minutesLeft);
      Serial.println(" min until next check.");
      delay(1000);
      return;
    } else {
      recentlyWatered = false;
      Serial.println("Wait period over. Checking moisture.");
    }
  }

  if (moisturePercent < 20) {
    displayMoisture(moisturePercent, "Watering...", "");
    Serial.println("Soil too dry! Turning pump ON.");
    for (int i = 0; i < 10; i++){
      digitalWrite(PUMP_PIN, HIGH);
      delay(10);
      digitalWrite(PUMP_PIN, LOW);
      delay(100);
    }

    

    lastPumpTime = millis();
    recentlyWatered = true;
  } else {
    displayMoisture(moisturePercent, "Soil OK", "");
    Serial.println("Soil moisture acceptable. No watering needed.");
  }

  delay(1000);
}

void displayMoisture(int percent, String line1, String line2) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Moisture:");
  display.setCursor(0, 20);
  display.print(percent);
  display.print("%");

  display.setTextSize(1);
  display.setCursor(0, 45);
  display.println(line1);
  display.setCursor(0, 55);
  display.println(line2);
  display.display();
}
