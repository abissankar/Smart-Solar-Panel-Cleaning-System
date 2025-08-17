#define BLYNK_TEMPLATE_ID "TMPL3L-RfqA3i"
#define BLYNK_TEMPLATE_NAME "Smart Solar Panel Cleaning"
#define BLYNK_AUTH_TOKEN "5Z0jo9Un1qEZTPhAa08HSPK6rprIGMwg"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <DHT.h>

// Pin definitions for Proteus simulation
#define DHTPIN 12      // DHT11 data pin
#define LDR_PIN 13     // LDR input pin
#define MOTOR1 14      // First relay
#define MOTOR2 15      // Second relay
#define MOTOR3 16      // Third relay
#define WIFI_LED 2     // WiFi status LED

// Constants
#define VOLTAGE_THRESHOLD 10
#define MOTOR_DELAY 1000
#define CLEANING_DELAY 10000

// Object initialization
DHT dht(DHTPIN, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

// Global variables
float humidity = 0, temperature = 0;
unsigned long previousMillis = 0;
const long interval = 2000;
bool isConnected = false;

// WiFi credentials
char auth[] = "5Z0jo9Un1qEZTPhAa08HSPK6rprIGMwg";
char ssid[] = "iotproject1234";
char pass[] = "iotproject1234";

void setup() {
  Serial.begin(9600);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize DHT sensor
  dht.begin();
  
  // Configure pins
  pinMode(LDR_PIN, INPUT);
  pinMode(MOTOR1, OUTPUT);
  pinMode(MOTOR2, OUTPUT);
  pinMode(MOTOR3, OUTPUT);
  pinMode(WIFI_LED, OUTPUT);
  
  // Initial state
  digitalWrite(MOTOR1, LOW);
  digitalWrite(MOTOR2, LOW);
  digitalWrite(MOTOR3, LOW);
  digitalWrite(WIFI_LED, HIGH);
  
  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus);
}

void checkBlynkStatus() {
  isConnected = Blynk.connected();
  digitalWrite(WIFI_LED, !isConnected);
  
  if (isConnected) {
    updateSensors();
  }
}

void updateSensors() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  Blynk.virtualWrite(V0, (int)humidity);
  Blynk.virtualWrite(V1, (int)temperature);
  
  // Update day/night status
  if (digitalRead(LDR_PIN) == 0) {
    Blynk.virtualWrite(V3, "DAY TIME!");
  } else {
    Blynk.virtualWrite(V3, "NIGHT TIME");
  }
}

void runCleaningCycle() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cleaning");
  lcd.setCursor(0, 1);
  lcd.print("running** ");
  
  Blynk.virtualWrite(V4, "Cleaning Process Run");
  
  // Motor sequence
  digitalWrite(MOTOR3, HIGH);
  delay(MOTOR_DELAY);
  
  digitalWrite(MOTOR1, HIGH);
  delay(CLEANING_DELAY);
  digitalWrite(MOTOR1, LOW);
  
  delay(MOTOR_DELAY);
  digitalWrite(MOTOR2, HIGH);
  delay(CLEANING_DELAY);
  digitalWrite(MOTOR2, LOW);
  
  delay(MOTOR_DELAY);
  digitalWrite(MOTOR3, LOW);
}

void loop() {
  // Read solar panel voltage
  int voltage = analogRead(A0);
  
  // Update LCD with voltage
  lcd.setCursor(12, 0);
  lcd.print("V:");
  lcd.print(voltage);
  
  // Regular sensor updates
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Blynk.virtualWrite(V2, voltage);
  }
  
  // Check cleaning conditions
  if (digitalRead(LDR_PIN) == 0 && voltage <= VOLTAGE_THRESHOLD) {
    runCleaningCycle();
  } else {
    Blynk.virtualWrite(V4, "Normal");
  }
  
  Blynk.run();
  timer.run();
}