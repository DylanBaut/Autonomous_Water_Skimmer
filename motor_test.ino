#include <Servo.h>

Servo myESC;  // Create a Servo object for controlling the ESC

int minThrottle = 1000;  // Minimum throttle (1000 microseconds = 1ms)
int maxThrottle = 2000;  // Maximum throttle (2000 microseconds = 2ms)
int neutralThrottle = 1500;  // Neutral throttle (1500 microseconds = 1.5ms)

int throttleValue = 50;  // Throttle value between 0 (min) and 100 (max)

void setup() {
  Serial.begin(9600);  // Start serial communication for debugging

  myESC.attach(5);  // Attach the ESC signal wire to Pin 5 on the Arduino

  // Initialize the ESC at neutral throttle (avoid sudden motor movement)
  Serial.println("Setting neutral throttle");
  myESC.writeMicroseconds(neutralThrottle);  // Send neutral throttle (1500us)
  delay(5000);  // Wait for 5 seconds to let the ESC settle

  // Set the motor to full power (maximum throttle)
  throttleValue = 100;  // Full throttle
  int pwmValue = map(throttleValue, 0, 100, minThrottle, maxThrottle);
  Serial.print("Setting full throttle: ");
  Serial.println(pwmValue);
  myESC.writeMicroseconds(pwmValue);  // Send maximum throttle (2000us)
  delay(5000);  // Wait for 5 seconds to let the motor run

  // Set the motor to minimum power (minimum throttle)
  throttleValue = 0;  // Minimum throttle
  pwmValue = map(throttleValue, 0, 100, minThrottle, maxThrottle);
  Serial.print("Setting minimum throttle: ");
  Serial.println(pwmValue);
  myESC.writeMicroseconds(pwmValue);  // Send minimum throttle (1000us)
  delay(5000);  // Wait for 5 seconds
}

void loop() {
  // Loop through different throttle values (this could be replaced with other control logic)
  for (throttleValue = 0; throttleValue <= 100; throttleValue++) {
    int pwmValue = map(throttleValue, 0, 100, minThrottle, maxThrottle);
    Serial.print("Throttle: ");
    Serial.print(throttleValue);
    Serial.print(" -> PWM: ");
    Serial.println(pwmValue);
    myESC.writeMicroseconds(pwmValue);
    delay(2000);  // Wait 2 seconds before changing throttle
  }
}
