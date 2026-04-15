#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// 0.9" ST7735 160x80 Display pins
#define TFT_CS    5
#define TFT_RST   16
#define TFT_DC    17
#define TFT_MOSI  23
#define TFT_SCLK  18

// Initialize for 0.9" 160x80 display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Pin definitions
#define BUZZER_PIN 25
#define BUTTON_PIN 26

// RTC object
RTC_DS3231 rtc;

// Bell schedule structure
struct BellEvent {
  uint8_t hour;
  uint8_t minute;
  const char* name;
  bool longRing;
};

// School schedule (CUSTOMIZE THIS)
BellEvent schedule[] = {
  {22, 27,  "Warning", false},
  {22, 30,  "Start",    false},
  {22, 32, "Recess",   true},
  {22, 34,"End Rc",   false},
  {22, 36, "Lunch",    true},
  {22, 38, "End Ln",   false},
  {22, 40, "Closing",  true}
};

const int scheduleSize = sizeof(schedule) / sizeof(schedule[0]);

// System state
enum SystemMode {
  MODE_IDLE,
  MODE_RINGING,
  MODE_MANUAL_OVERRIDE
};

SystemMode currentMode = MODE_IDLE;
int lastRungIndex = -1;
unsigned long ringStartTime = 0;
unsigned long ringDuration = 0;
unsigned long lastDisplayUpdate = 0;
bool buttonPressed = false;
unsigned long lastButtonDebounce = 0;
const unsigned long debounceDelay = 50;

int currentHour = 0, currentMinute = 0, currentSecond = 0;

// Function prototypes
void updateDisplay();
void ringBell(bool longRing);
void stopBell();
void checkSchedule();
int findNextBellIndex(int hour, int minute);
void readRTC();
void checkButton();

void setup() {
  Serial.begin(115200);
  Serial.println("Automated Bell System Starting...");
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize ST7735 0.9" 160x80 display
  Serial.println("Initializing ST7735 0.9\"...");
  tft.initR(INITR_MINI160x80);  // Important! This is for 0.9" 160x80 display
  
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);  // Adjust as needed (0-3)
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  
  tft.setCursor(0, 0);
  tft.println("Bell System");
  tft.println("Starting...");
  
  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("RTC ERROR!");
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.println("RTC ERROR!");
    tft.println("Check wiring");
    for(;;);
  }
  
  if (rtc.lostPower()) {
    Serial.println("Setting RTC time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    tft.setCursor(0, 40);
    tft.println("Time Set");
    delay(2000);
  }
  
  // Test buzzer
  Serial.println("Testing buzzer...");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  
  tft.fillScreen(ST77XX_BLACK);
  Serial.println("System Ready!");
}

void loop() {
  readRTC();
  checkButton();
  checkSchedule();
  updateDisplay();
  
  if (currentMode == MODE_RINGING) {
    if (millis() - ringStartTime >= ringDuration) {
      stopBell();
    }
  }
  
  delay(100);
}

void readRTC() {
  DateTime now = rtc.now();
  currentHour = now.hour();
  currentMinute = now.minute();
  currentSecond = now.second();
}

void checkSchedule() {
  if (currentMode != MODE_IDLE) return;
  
  for (int i = 0; i < scheduleSize; i++) {
    if (currentHour == schedule[i].hour && 
        currentMinute == schedule[i].minute && 
        currentSecond < 3) {
      
      if (lastRungIndex != i) {
        Serial.print("Trigger: ");
        Serial.println(schedule[i].name);
        ringBell(schedule[i].longRing);
        lastRungIndex = i;
        break;
      }
    }
  }
  
  static int lastMinute = -1;
  if (currentMinute != lastMinute) {
    lastMinute = currentMinute;
    if (currentSecond > 10) {
      lastRungIndex = -1;
    }
  }
}

void ringBell(bool longRing) {
  currentMode = MODE_RINGING;
  ringStartTime = millis();
  ringDuration = longRing ? 12000 : 2000;
  digitalWrite(BUZZER_PIN, HIGH);
  Serial.println("🔔 BELL RINGING!");
}

void stopBell() {
  digitalWrite(BUZZER_PIN, LOW);
  currentMode = MODE_IDLE;
  Serial.println("Bell stopped");
}

void checkButton() {
  if ((millis() - lastButtonDebounce) > debounceDelay) {
    bool buttonState = digitalRead(BUTTON_PIN);
    
    if (buttonState == LOW && !buttonPressed) {
      buttonPressed = true;
      lastButtonDebounce = millis();
      Serial.println("Button pressed!");
      
      if (currentMode == MODE_IDLE) {
        currentMode = MODE_MANUAL_OVERRIDE;
        ringBell(false);
      } else if (currentMode == MODE_RINGING) {
        stopBell();
      }
    } 
    else if (buttonState == HIGH && buttonPressed) {
      buttonPressed = false;
      lastButtonDebounce = millis();
      if (currentMode == MODE_MANUAL_OVERRIDE) {
        stopBell();
      }
    }
  }
}

int findNextBellIndex(int hour, int minute) {
  int currentTotal = hour * 60 + minute;
  int nextIndex = -1;
  int smallestDiff = 24 * 60;
  
  for (int i = 0; i < scheduleSize; i++) {
    int eventTotal = schedule[i].hour * 60 + schedule[i].minute;
    int diff = eventTotal - currentTotal;
    
    if (diff > 0 && diff < smallestDiff) {
      smallestDiff = diff;
      nextIndex = i;
    }
  }
  
  if (nextIndex == -1 && scheduleSize > 0) {
    nextIndex = 0;
  }
  
  return nextIndex;
}

void updateDisplay() {
  if (millis() - lastDisplayUpdate < 500) return;
  lastDisplayUpdate = millis();
  
  tft.fillScreen(ST77XX_BLACK);
  
  // === ROW 1: Mode (Y = 0) ===
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.print("Mode:");
  
  switch(currentMode) {
    case MODE_IDLE:
      tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
      tft.print(" IDLE");
      break;
    case MODE_RINGING:
      tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
      tft.print(" RING!");
      break;
    case MODE_MANUAL_OVERRIDE:
      tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
      tft.print(" MANUAL");
      break;
  }
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  
  // === ROW 2: Time (Y = 12) - Large font ===
  tft.setTextSize(2);
  tft.setCursor(0, 12);
  if (currentHour < 10) tft.print("0");
  tft.print(currentHour);
  tft.print(":");
  if (currentMinute < 10) tft.print("0");
  tft.print(currentMinute);
  tft.print(":");
  if (currentSecond < 10) tft.print("0");
  tft.print(currentSecond);
  
  // === ROW 3: Next Bell Label (Y = 32) ===
  tft.setTextSize(1);
  tft.setCursor(0, 32);
  tft.print("Next:");
  
  // === ROW 4: Next Bell Time & Event (Y = 44) ===
  int nextIndex = findNextBellIndex(currentHour, currentMinute);
  if (nextIndex != -1) {
    tft.setCursor(0, 44);
    if (schedule[nextIndex].hour < 10) tft.print("0");
    tft.print(schedule[nextIndex].hour);
    tft.print(":");
    if (schedule[nextIndex].minute < 10) tft.print("0");
    tft.print(schedule[nextIndex].minute);
    tft.print(" ");
    tft.println(schedule[nextIndex].name);
  } else {
    tft.setCursor(0, 44);
    tft.print("None");
  }
  
  // === ROW 5: Current Event if ringing (Y = 60) ===
  if (currentMode == MODE_RINGING) {
    tft.setCursor(0, 60);
    tft.print("Event:");
    for (int i = 0; i < scheduleSize; i++) {
      if (currentHour == schedule[i].hour && currentMinute == schedule[i].minute) {
        tft.print(schedule[i].name);
        break;
      }
    }
  }
}