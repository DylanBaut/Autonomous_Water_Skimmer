#include <Servo.h>

Servo myESC;  // Create a Servo object for controlling the ESC

int minThrottle = 0;  // Minimum throttle mapped to 0 degrees
int maxThrottle = 180;  // Maximum throttle mapped to 180 degrees
int neutralThrottle = 90;  // Neutral throttle mapped to 90 degrees

int throttleValue = 50;  // Throttle value between 0 (min) and 100 (max)

void setup() {
  Serial.begin(9600);  // Start serial communication for debugging

  myESC.attach(5);  // Attach the ESC signal wire to Pin 5 on the Arduino
  //Initialize at zero pos
  Serial.println("Setting ZERO");
  myESC.write(minThrottle);  
  delay(30000);  


  // Initialize the ESC at neutral throttle (avoid sudden motor movement)
  Serial.println("Setting neutral throttle");
  myESC.write(neutralThrottle);  // Send neutral throttle (90 degrees)
  delay(5000);  // Wait for 5 seconds to let the ESC settle

  // Set the motor to full power (maximum throttle)
  throttleValue = 100;  // Full throttle
  int angleValue = map(throttleValue, 0, 100, minThrottle, maxThrottle);
  Serial.print("Setting full throttle: ");
  Serial.println(angleValue);
  myESC.write(angleValue);  // Send maximum throttle (180 degrees)
  delay(5000);  // Wait for 5 seconds to let the motor run

  // Set the motor to minimum power (minimum throttle)
  throttleValue = 0;  // Minimum throttle
  angleValue = map(throttleValue, 0, 100, minThrottle, maxThrottle);
  Serial.print("Setting minimum throttle: ");
  Serial.println(angleValue);
  myESC.write(angleValue);  // Send minimum throttle (0 degrees)
  delay(5000);  // Wait for 5 seconds
}

void loop() {
  // Loop through different throttle values (this could be replaced with other control logic)
  for (throttleValue = 50; throttleValue <= 100; throttleValue++) {
    int angleValue = map(throttleValue, 0, 100, minThrottle, maxThrottle);
    Serial.print("Throttle: ");
    Serial.print(throttleValue);
    Serial.print(" -> Angle: ");
    Serial.println(angleValue);
    myESC.write(angleValue);
    delay(500);  // Wait 2 seconds before changing throttle
  }
}