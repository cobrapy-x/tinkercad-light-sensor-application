/*
  Light Sensor Application

  A0: Phototransistor emitter and 1 kΩ resistor junction
  A4: I2C LCD SDA data connection
  A5: I2C LCD SCL clock connection
  D9: Red LED through a 300 Ω resistor
  5V: Phototransistor collector and LCD VCC
  GND: Sensor resistor, LED cathode, and LCD GND
*/

#include <Adafruit_LiquidCrystal.h>

// ----- Pins -----
const int LIGHT_SENSOR_PIN = A0;
const int LED_PIN = 9;

// ----- Circuit values -----
const int SENSOR_RESISTOR_OHMS = 1000;
const int LED_RESISTOR_OHMS = 300;
const float ADC_REFERENCE_VOLTS = 5.0;
const int ADC_MAXIMUM = 1023;

// ----- Settings -----
const int SENSOR_MINIMUM = 0;
const int SENSOR_MAXIMUM = 471;
const int DARK_LIMIT_PERCENT = 25;
const int BRIGHT_LIMIT_PERCENT = 65;
const int LED_FULL_PWM = 255;
const int LED_DIM_PWM = 100;
const int LED_OFF_PWM = 0;
const unsigned long SAMPLE_DELAY_MS = 200;

// Address selector 0 corresponds to I2C address 0x20.
Adafruit_LiquidCrystal lcd(0);

// Reads the sensor and keeps its value inside the measured range.
int readCalibratedSensor() {
  int reading = analogRead(LIGHT_SENSOR_PIN);
  return constrain(reading, SENSOR_MINIMUM, SENSOR_MAXIMUM);
}

// Converts the measured sensor range into a relative percentage.
int calculateLightPercent(int sensorReading) {
  return map(
    sensorReading,
    SENSOR_MINIMUM,
    SENSOR_MAXIMUM,
    0,
    100
  );
}

// Converts an ADC reading into an estimated sensor voltage.
float calculateSensorVoltage(int sensorReading) {
  return sensorReading * ADC_REFERENCE_VOLTS / ADC_MAXIMUM;
}

// Selects the LED output for dark, medium, or bright conditions.
int selectLedBrightness(int lightPercent) {
  if (lightPercent < DARK_LIMIT_PERCENT) {
    return LED_FULL_PWM;
  }

  if (lightPercent < BRIGHT_LIMIT_PERCENT) {
    return LED_DIM_PWM;
  }

  return LED_OFF_PWM;
}

// Returns a short name for the current light condition.
const char* getLightCondition(int lightPercent) {
  if (lightPercent < DARK_LIMIT_PERCENT) {
    return "DARK";
  }

  if (lightPercent < BRIGHT_LIMIT_PERCENT) {
    return "MEDIUM";
  }

  return "BRIGHT";
}

void setup() {
  // Configure the LED pin before controlling its brightness.
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, LED_OFF_PWM);

  // Start the 16-column, 2-row I2C LCD.
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("LIGHT SENSOR");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(1000);
  lcd.clear();

  // Start communication with the Serial Monitor.
  Serial.begin(9600);
}

void loop() {
  // ----- 1) Measure -----
  int sensorReading = readCalibratedSensor();

  // ----- 2) Calculate -----
  int lightPercent = calculateLightPercent(sensorReading);
  float sensorVoltage = calculateSensorVoltage(sensorReading);
  int ledBrightness = selectLedBrightness(lightPercent);
  const char* condition = getLightCondition(lightPercent);

  // Apply the selected PWM brightness to the LED.
  analogWrite(LED_PIN, ledBrightness);

  // ----- 3) Display -----
  lcd.setCursor(0, 0);
  lcd.print("LIGHT: ");
  lcd.print(lightPercent);
  lcd.print("%    ");  // Extra spaces erase digits left from an older value.

  lcd.setCursor(0, 1);

  // Print a fixed-width message so old characters are removed.
  if (lightPercent < DARK_LIMIT_PERCENT) {
    lcd.print("DARK: LAMP ON  ");
  } else if (lightPercent < BRIGHT_LIMIT_PERCENT) {
    lcd.print("MEDIUM: DIM    ");
  } else {
    lcd.print("BRIGHT: OFF    ");
  }

  // ----- 4) Serial Monitor -----
  Serial.print("Sensor: ");
  Serial.print(sensorReading);

  Serial.print(" | Voltage: ");
  Serial.print(sensorVoltage, 2);
  Serial.print(" V");

  Serial.print(" | Light: ");
  Serial.print(lightPercent);
  Serial.print("%");

  Serial.print(" | Condition: ");
  Serial.print(condition);

  Serial.print(" | LED PWM: ");
  Serial.println(ledBrightness);

  delay(SAMPLE_DELAY_MS);
}
