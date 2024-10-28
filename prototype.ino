#include <TinyGPSPlus.h>
#include <Wire.h>
#include <ICM_20948.h>

// Create a TinyGPS++ object
TinyGPSPlus gps;

// Create an ICM-20948 object
ICM_20948_I2C imu; // No parameters needed

// Define LED pins
const int led1Pin = 4; // PD4
const int led2Pin = 7; // PD7
const int buttonPin = 2; // PD2

// Timing variables for LEDs
unsigned long previousMillisLed1 = 0;
unsigned long previousMillisLed2 = 0;
const long intervalLed1 = 500; // Blink every 500 ms for LED 1
const long intervalLed2 = 250; // Blink every 250 ms for LED 2

// Button state variables
int buttonState = 0; 
int lastButtonState = 0; 
unsigned long debounceDelay = 50; // Debounce delay
unsigned long lastDebounceTime = 0; 

void setup() {
  // Start the serial communication
  Serial.begin(9600);
  
  // Initialize I2C for the IMU
  Wire.begin();

  // Initialize the IMU
  if (imu.begin() != 0) { // Check if begin() returns 0 for success
    Serial.println("IMU initialization failed!");
    while (1); // Stop execution
  }
  Serial.println("IMU initialized!");

  // Initialize the LED pins as outputs
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  
  // Initialize the button pin as input with pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Read button state
  int reading = digitalRead(buttonPin);

  // Check for button state change (debouncing)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Only change the button state if the debounce time has passed
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      // If the button is pressed (active LOW)
      if (buttonState == LOW) {
        // Action on button press (optional)
        Serial.println("Button pressed!");
      }
    }
  }

  lastButtonState = reading;

  // Read GPS data
  while (Serial.available() > 0) {
    gps.encode(Serial.read());

    if (gps.location.isUpdated()) {
      Serial.print("Latitude: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude: ");
      Serial.println(gps.location.lng(), 6);
    }
  }

  // Read IMU data
  if (imu.dataReady()) {
    imu.getAGMT(); // Get accelerometer, gyroscope, magnetometer, and temperature data

    // Print accelerometer data
    Serial.print("Accel X: ");
    Serial.print(imu.accX());
    Serial.print(" Y: ");
    Serial.print(imu.accY());
    Serial.print(" Z: ");
    Serial.println(imu.accZ());

    // Print gyroscope data
    Serial.print("Gyro X: ");
    Serial.print(imu.gyrX());
    Serial.print(" Y: ");
    Serial.print(imu.gyrY());
    Serial.print(" Z: ");
    Serial.println(imu.gyrZ());

    // Print magnetometer data
    Serial.print("Mag X: ");
    Serial.print(imu.magX());
    Serial.print(" Y: ");
    Serial.print(imu.magY());
    Serial.print(" Z: ");
    Serial.println(imu.magZ());

    // Uncomment if temperature data is supported
    // Serial.print("Temperature: ");
    // Serial.println(imu.temperature());
  }

  // Handle LED 1 blinking
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisLed1 >= intervalLed1) {
    previousMillisLed1 = currentMillis;
    digitalWrite(led1Pin, !digitalRead(led1Pin)); // Toggle LED 1
  }

  // Handle LED 2 blinking
  if (currentMillis - previousMillisLed2 >= intervalLed2) {
    previousMillisLed2 = currentMillis;
    digitalWrite(led2Pin, !digitalRead(led2Pin)); // Toggle LED 2
  }

  delay(100); // Add a delay to avoid overwhelming the Serial Monitor
}
