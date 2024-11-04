#include <TinyGPSPlus.h>
#include <Wire.h>
#include <ICM_20948.h>
#include <math.h> // Include math for the atan2 function
#include <Servo.h> // Include Servo library for ESC control

// Create a TinyGPS++ object
TinyGPSPlus gps;

// Create an ICM-20948 object
ICM_20948_I2C imu; // No parameters needed

// Define sensor pins
const int turbidityPin = A0; // A0 for analog reading of turbidity
const int led1Pin = 4; // PD4
const int led2Pin = 7; // PD7
const int buttonPin = 2; // PD2
const int escPin = 6; // Pin for ESC control

// Timing variables for LEDs
unsigned long previousMillisLed1 = 0;
unsigned long previousMillisLed2 = 0;
const long intervalLed1 = 500;
const long intervalLed2 = 250;

// Button and sensor state variables
int buttonState = 0; 
int lastButtonState = 0; 
unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0; 

// Create a Servo object for ESC
Servo esc;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (imu.begin() != 0) {
    Serial.println("IMU initialization failed!");
    while (1);
  }
  Serial.println("IMU initialized!");

  // Initialize pins
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // Attach the ESC to the servo object
  esc.attach(escPin);
  
  // Calibrate the ESC by sending a minimum signal (1000 microseconds)
  esc.writeMicroseconds(1000);
  delay(2000); // Let ESC initialize
  Serial.println("ESC calibrated and ready");
}

void loop() {
  // Button debouncing
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay && reading != buttonState) {
    buttonState = reading;
    if (buttonState == LOW) Serial.println("Button pressed!");
  }
  lastButtonState = reading;

  // Turbidity sensor reading
  int turbidityValue = analogRead(turbidityPin);
  float voltage = turbidityValue * (5.0 / 1023.0); // Convert to voltage (0-5V)
  Serial.print("Turbidity analog value: ");
  Serial.print(turbidityValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.println(" V");

  // GPS data
  while (Serial.available() > 0) {
    gps.encode(Serial.read());
    if (gps.location.isUpdated()) {
      Serial.print("Latitude: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude: ");
      Serial.println(gps.location.lng(), 6);
    }
  }

  // IMU data
  if (imu.dataReady()) {
    imu.getAGMT();
    float heading = atan2(imu.magY(), imu.magX()) * (180.0 / PI);
    if (heading < 0) heading += 360;

    Serial.print("Heading: ");
    Serial.print(heading);
    Serial.println(" degrees");
  }

  // ESC control
  int escSpeed = 1500; // Set speed (1000 = full reverse, 1500 = stop, 2000 = full forward)
  esc.writeMicroseconds(escSpeed);

  // LED blinking logic
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisLed1 >= intervalLed1) {
    previousMillisLed1 = currentMillis;
    digitalWrite(led1Pin, !digitalRead(led1Pin));
  }
  if (currentMillis - previousMillisLed2 >= intervalLed2) {
    previousMillisLed2 = currentMillis;
    digitalWrite(led2Pin, !digitalRead(led2Pin));
  }

  delay(100);
}
