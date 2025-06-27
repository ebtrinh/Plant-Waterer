#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define MOISTURE_PIN A0
#define PUMP_PIN 7

const uint8_t  POT_PIN  = A2;    // Wiper connected to A1
const uint16_t RAW_MAX  = 1023;  // 10-bit ADC gives 0-1023

int   potRaw  = 0;     // Latest raw reading
float potFrac = 0.0f;  // Scaled 0.0-1.0 value

unsigned long lastPumpTime = 0;
bool recentlyWatered = false;
const unsigned long waitDuration = 20000UL;  // 1 hour
int prevMoisture = 0;
int curMoisture = 0;
unsigned long pumpPower = 0;
int targetMoisture = 50;
int lowWaterPower = 0;
int lastdisplaytime = 0;
int initializeTime = 10000;

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
  int moistureValue = analogRead(MOISTURE_PIN);
  int moisturePercent = map(moistureValue, 1023, 0, 0, 100);
  
  curMoisture = moisturePercent;
}

void loop(){
  
  // int curTime = millis();
  // Serial.println(curTime);
  // Serial.println(curTime-lastdisplaytime);
  //   delay(0);
    
  //   delay(0);
  //   //lastdisplaytime += 3000;
  //   Serial.println("HITT");
    
  unsigned long currentTime = millis();
  int moistureValue = analogRead(MOISTURE_PIN);
  int moisturePercent = map(moistureValue, 1023, 0, 0, 100);
  potRaw  = analogRead(POT_PIN);            // 0-1023
    potFrac = potRaw / float(RAW_MAX);        // 0.0-1.0
    targetMoisture = 100-round(potFrac*100);
  if (currentTime < initializeTime){
    displayMoisture(targetMoisture, "set target", "");
    delay(100);
    return;
  }
  
  // Serial.print("Moisture: ");
  // Serial.print(moisturePercent);
  // Serial.println("%");
  
  if (recentlyWatered) {
    unsigned long elapsed = currentTime - lastPumpTime;
    if (elapsed < waitDuration) {
      int minutesLeft = (waitDuration - elapsed) / 1000;
      displayMoisture(moisturePercent, String(minutesLeft) + " sec", "Target: " + String(targetMoisture));
      //Serial.print("Waiting: ");
      //Serial.print(minutesLeft);
      //Serial.println(" sec until next check.");
      return;
    } else {
      prevMoisture = curMoisture;
      curMoisture = moisturePercent;
      recentlyWatered = false;
      //Serial.println("Wait period over. Checking moisture.");
    }
  }
  
  Serial.println("pumpif");
  if (moisturePercent < targetMoisture) {
    int MoiTillTarget = targetMoisture-curMoisture;
    int moistureIncrease = curMoisture-prevMoisture;
    if (moistureIncrease > 0 && lowWaterPower == 0){
      lowWaterPower = pumpPower;\
       Serial.print("this2");
    }
    else {
      if (moistureIncrease < 1){
        lowWaterPower++;
        pumpPower = lowWaterPower;
        Serial.print("this");
      }
      else {
        pumpPower = lowWaterPower + MoiTillTarget/2;
         Serial.print("this3");
      }
      
    }
    displayMoisture(pumpPower, "Watering...", "");  
    Serial.println("Soil too dry! Turning pump ON.");
    Serial.println(lowWaterPower);
    Serial.println(moistureIncrease);
    Serial.println(pumpPower);
    Serial.println(pumpPower);
    delay(500);
    for (int i = 0; i < 10; i++){
      digitalWrite(PUMP_PIN, HIGH);
      delay((pumpPower)*1.5);
      digitalWrite(PUMP_PIN, LOW);
      delay(pumpPower*0.9);
      
    }
    delay(500);

    lastPumpTime = millis();
    recentlyWatered = true;
  } else {
    displayMoisture(moisturePercent, "Soil OK",  "Target: " + String(targetMoisture));
    Serial.println("Soil moisture acceptable. No watering needed.");
  }
  Serial.println("end");
  
}

void displayMoisture(int percent, String line1, String line2) {
  delay(500);
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
  delay(500);
}
