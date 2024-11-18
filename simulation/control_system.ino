#include <Servo.h>

// Define states
enum RobotState {
    START,
    STRAIGHT_NAVIGATION,
    BOUNDARY_DETECTED,
    STUCK_DETECTED,
    RETURN_TO_BASE
};

const int turbidityPin = A0; // A0 for analog reading of turbidity
const int led1Pin = 4; // PD4
const int led2Pin = 7; // PD7
const int buttonPin = 2; // PD2

// Button and sensor state variables
int buttonState = 0; 
int lastButtonState = 0; 
unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0; 
unsigned long previousMillisLed = 0;
const long intervalLed = 500;


RobotState currentState = START;

// Motor control
Servo leftMotor;
Servo rightMotor;

int zeroThrottle = 90;  // Neutral throttle
int maxThrottle = 140; // Maximum forward speed
int minThrottle = 60;  // Maximum reverse speed

// Timing variables
unsigned long startTime = 0;
unsigned long correctionCount = 0;
const unsigned long maxMissionTime = 10 * 60 * 1000; // 10 minutes in milliseconds

//location
float currentLat = 0.0, currentLong = 0.0; // Robot's simulated position lat [-90, 90], long [-180, 180]
float baseLat = 0.0, baseLong = 0.0;      // Base coordinates 
float aimedBearing = 0.0;             //The intended angle between robots heading and actual North 
float trueBearing = 0.0;               //Robot's angle between its heading and actual North (calculated from IMU info)

//Boundaries(changeable)
const int numVertices = 5;
float polygonLat[numVertices] = {40.0, 40.01, 40.02, 40.01, 40.0}; // Example latitudes
float polygonLong[numVertices] = {-105.0, -105.01, -105.0, -104.99, -105.0}; // Example longitudes
const float threshold = 0.5;


void setup() {
    Serial.begin(9600);
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(led1Pin, OUTPUT);
    pinMode(led2Pin, OUTPUT);
    // Attach motors to pins
    leftMotor.attach(5);  // Adjust pin as per your circuit
    rightMotor.attach(6); // Adjust pin as per your circuit

    // Initialize ESCs
    initializeESCs(leftMotor, rightMotor);

    Serial.println("System Initialized. Waiting for Start Signal...");
}

void loop() {

    
    switch (currentState) {
        case START:
            // Button debouncing Polling
            int reading = digitalRead(buttonPin);
            if (reading != lastButtonState) lastDebounceTime = millis();
            if ((millis() - lastDebounceTime) > debounceDelay && reading != buttonState) {
              buttonState = reading;
              if (buttonState == LOW){ //ONLY TRANSITION IF BUTTON IS PRESSED
                startTime = millis();
                initializeESCs_wait(leftMotor, rightMotor);
                currentState = STRAIGHT_NAVIGATION;
                Serial.println("Mission Started. Transitioning to Straight-line Navigation.");
                }
            }
            lastButtonState = reading;
            break;
            
            
            
            

        case STRAIGHT_NAVIGATION:
            straightLineNavigation();
            if (isPointNearEdgeGeo(currentLat, currentLong, polygonLat, polygonLong, numVertices, threshold)) {
                currentState = BOUNDARY_DETECTED;
                Serial.println("Boundary Detected. Making a course correction.");
            } else if (millis() - startTime > maxMissionTime) {
                currentState = RETURN_TO_BASE;
                Serial.println("Time limit reached. Returning to base.");
            }
            break;

        case BOUNDARY_DETECTED:
            boundaryCorrection();
            break;

        case STUCK_DETECTED:
            stopMotors();
            currentState = START;
            Serial.println("Robot is stuck. Returning to start state.");
            break;

        case RETURN_TO_BASE:
            returnToBase();
            if (isPointNearEdgeGeo(currentLat, currentLong, polygonLat, polygonLong, numVertices, threshold)) {
                currentState = BOUNDARY_DETECTED;
            } else if (atBase()) {
                stopMotors();
                currentState = START;
                Serial.println("Robot returned to base. Mission completed.");
            }
            break;
    }
}

void initializeESCs(Servo &motor, Servo &motor2) {
    motor.write(89); // Initialize ESC
    motor2.write(89); // Initialize ESC
    delay(10000);     // Wait 10 seconds for ESCs to be ready
    Serial.println("ESC Initialized");
}

void initializeESCs_wait(Servo &motor, Servo &motor2) {
    motor.write(89); // Initialize ESC
    motor2.write(89); // Initialize ESC
    delay(30000);     // Wait 30 seconds for ESCs to be ready
    flash_LEDs();
    Serial.println("Wait completed, should be in the water now.");
}

void flash_LEDs(){
//    Serial.println("Flash LEDs");
   unsigned long currentMillis = millis();
    if (currentMillis - previousMillisLed >= intervalLed) {
      previousMillisLed = currentMillis;
      digitalWrite(led1Pin, !digitalRead(led1Pin));
      digitalWrite(led2Pin, !digitalRead(led2Pin));
    }
}

void straightLineNavigation() {
    setMotorSpeed(zeroThrottle + 20, zeroThrottle + 20); // Both motors forward at the same speed
    updateGPS();
}

void boundaryCorrection() {
    if (random(0, 2)) { // Randomly turn left or right
        //turn left by generated degrees off current direction

        aimedBearing = (trueBearing - generateRandomAngle());
        aimedBearing = aimedBearing - 360.0 * static_cast<int>(aimedBearing / 360.0); // Manual modulo
        if (aimedBearing < 0) aimedBearing += 360.0; // Ensure the value is non-negative


        setMotorSpeed(zeroThrottle, zeroThrottle + 30);
        while(trueBearing != aimedBearing){
            Serial.println("Turning Left");
            setTrueBearing();
         }
    } else {
       //turn right by generated degrees off current direction
        
        aimedBearing = (trueBearing - generateRandomAngle());
        aimedBearing = aimedBearing - 360.0 * static_cast<int>(aimedBearing / 360.0); // Manual modulo
        if (aimedBearing < 0) aimedBearing += 360.0; // Ensure the value is non-negative

        setMotorSpeed(zeroThrottle + 30, zeroThrottle);
        while(trueBearing != aimedBearing){
            Serial.println("Turning Right");
            setTrueBearing();
         }
    }
    updateGPS();
    if (isPointNearEdgeGeo(currentLat, currentLong, polygonLat, polygonLong, numVertices, threshold)) {
        correctionCount++;
        if (correctionCount >= 4) {
            currentState = STUCK_DETECTED;
        }
    }
}

void returnToBase() {
    calculatePathToBase();
    setMotorSpeed(zeroThrottle + 20, zeroThrottle + 20); // Straight-line speed to base
    updateGPS();
}

void stopMotors() {
    setMotorSpeed(zeroThrottle, zeroThrottle); // Neutral throttle
}

void setMotorSpeed(int leftSpeed, int rightSpeed) {
    leftMotor.write(leftSpeed);
    rightMotor.write(rightSpeed);
}

void updateGPS() {
  currentLat = getLat();
  currentLong = getLong();
  
}

bool atBase() {
    // Simulate base detection
    return (currentLat == baseLat && currentLong == baseLong);
}

void calculatePathToBase() {
    // Simulate path calculation to the base
    aimedBearing = atan2(baseLong - currentLong, baseLat - currentLat) * 180 / PI;
}

float generateRandomAngle() {
     return random(90, 180);
}
void setTrueBearing(){
    trueBearing = getIMUinfo();
}
//need to be filled in/abstracted from sensors
float getLat(){
    return 0;
}

float getLong(){
    return 0;
}

float getIMUinfo(){
    return 0;
}



/*
*GPS detection near edges
*/

float pointToSegmentDistanceGeo(float lat, float lon, float lat1, float lon1, float lat2, float lon2) {
    // Convert latitudes and longitudes to radians
    lat *= M_PI / 180.0;
    lon *= M_PI / 180.0;
    lat1 *= M_PI / 180.0;
    lon1 *= M_PI / 180.0;
    lat2 *= M_PI / 180.0;
    lon2 *= M_PI / 180.0;

    // Approximation: Treat latitude/longitude as local Cartesian coordinates
    float dx = (lon2 - lon1) * cos((lat1 + lat2) / 2.0); // Longitude difference scaled by average latitude
    float dy = lat2 - lat1; // Latitude difference

    // Degenerate segment case
    if (dx == 0 && dy == 0) {
        float dLat = lat - lat1;
        float dLon = (lon - lon1) * cos(lat1);
        return sqrt(dLat * dLat + dLon * dLon) * 111000.0; // Distance in meters
    }

    // Project point onto line segment (clamped to segment bounds)
    float t = ((lat - lat1) * dy + (lon - lon1) * dx) / (dx * dx + dy * dy);
    t = fmax(0, fmin(1, t)); // Clamp t to the range [0, 1]

    // Closest point on segment
    float closestLat = lat1 + t * dy;
    float closestLon = lon1 + t * dx;

    // Distance from point to closest point on segment
    float dLat = lat - closestLat;
    float dLon = (lon - closestLon) * cos(lat);
    return sqrt(dLat * dLat + dLon * dLon) * 111000.0; // Distance in meters
}


// Function to check if a point is near any edge of the polygon
bool isPointNearEdgeGeo(float lat, float lon, const float* vertLat, const float* vertLon, int numVertices, float thresholdMeters) {
    for (int i = 0, j = numVertices - 1; i < numVertices; j = i++) {
        float dist = pointToSegmentDistanceGeo(lat, lon, vertLat[i], vertLon[i], vertLat[j], vertLon[j]);
        if (dist <= thresholdMeters) {
            return true; // Point is near an edge
        }
    }
    return false; // Point is not near any edge
}


/*
*GPS detection outside of edges (outside of polygon) Should probably only be used every once in a while
*/

bool isPointInPolygon(float x, float y, const float* vertX, const float* vertY, int numVertices) {
    bool inside = false;
    for (int i = 0, j = numVertices - 1; i < numVertices; j = i++) {
        // Get current and previous vertex
        float xi = vertX[i], yi = vertY[i];
        float xj = vertX[j], yj = vertY[j];

        // Check if the point is inside
        bool intersect = ((yi > y) != (yj > y)) &&
                         (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
        if (intersect) inside = !inside;
    }
    return inside;
}



