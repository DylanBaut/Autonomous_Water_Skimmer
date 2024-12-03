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
bool button_pressed =false;
bool return_flag = false;

RobotState currentState = START;

// Motor control
Servo leftMotor;
Servo rightMotor;

int zeroThrottle = 90;  // Neutral throttle
int maxThrottle = 140; // Maximum forward speed
int minThrottle = 60;  // Maximum reverse speed
int currentLeftSpeed = 90;
int currentRightSpeed = 90;

// Timing variables
unsigned long startTime = 0;
unsigned long correctionCount = 0;
const unsigned long maxMissionTime = 1 * 60 * 1000; // 10 minutes in milliseconds

//location
float currentLat = 0.0, currentLong = 0.0; // Robot's simulated position lat [-90, 90], long [-180, 180]
float baseLat = 600.0, baseLong = 700.0;      // Base coordinates 
float aimedBearing = 0.0;             //The intended angle between robots heading and actual North 
float trueBearing = 0.0;               //Robot's angle between its heading and actual North (calculated from IMU info)

//Boundaries(changeable)
const int numVertices = 5;
float polygonLat[numVertices] = {41.2749, 43.7749, 41.7749, 37.7749, 38.2749};
float polygonLong[numVertices] = {-124.9194, -120.9194, -116.9194, -118.9194, -125.4194};
const float threshold = 30;

//global variables used for boundary/stuck detection
float waitPeriod = 0;
float boundaryShifts =0; //number of boundary shifts occurred in a row

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

//    Serial.println("System Initialized. Waiting for Start Signal...");
}

void loop() {

    Serial.println(millis());

    //recieved GPS and IMU data from server to abtsract these sensors since were not actually moving
   getAbstractedData();

    // Button debouncing Polling
    int reading = digitalRead(buttonPin);
    if (reading != lastButtonState) lastDebounceTime = millis();
    if ((millis() - lastDebounceTime) > debounceDelay && reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW){
        startTime = millis();
        button_pressed = true;
      }
     }
     lastButtonState = reading;
     
    waitPeriod +=1;
    if (millis() > 25000) {   // YOU NEED TO HAVE 60000 NOT maxMissionTime.... different vars?
    if (return_flag == false){
      currentState = RETURN_TO_BASE;
      Serial.println("Time limit reached. Returning to base.");
    }
      //while(1);
  }

    
    switch (currentState) {
        case START:
            Serial.println("STATE:START");
            //ONLY TRANSITION IF BUTTON IS PRESSED
            if (button_pressed){
              currentState = STRAIGHT_NAVIGATION;
              waitPeriod =0;
              initializeESCs_wait(leftMotor, rightMotor);
            }
 
             break;
        case STRAIGHT_NAVIGATION:
            Serial.println("STATE:STRAIGHT_NAVIGATION");
            straightLineNavigation();
            if (waitPeriod >50){
              if (isPointNearEdgeGeo(currentLat, currentLong, polygonLat, polygonLong, numVertices, threshold)) {
                if(waitPeriod == 51){//wait period just ended
                  boundaryShifts +=1;
                }else{
                  boundaryShifts =0;
                }
                if(boundaryShifts >=4 ){
                  currentState = STUCK_DETECTED;
                }else{
                  currentState = BOUNDARY_DETECTED;
                  Serial.println("Boundary Detected. Making a course correction.");
                }
              } 
            }
            break;

        case BOUNDARY_DETECTED:
            Serial.println("STATE:BOUNDARY_DETECTED");
            boundaryCorrection();
            break;

        case STUCK_DETECTED:
            Serial.println("STATE:STUCK_DETECTED");
            stopMotors();
            currentState = START;
//            Serial.println("Robot is stuck. Returning to start state.");
            break;

        case RETURN_TO_BASE:
          if (return_flag == false){
            Serial.println("STATE:RETURN_TO_BASE");
          }
          returnToBase();
            
            // if (isPointNearEdgeGeo(currentLat, currentLong, polygonLat, polygonLong, numVertices, threshold)) {
            //     currentState = BOUNDARY_DETECTED;
            if (atBase()) {
                stopMotors();
                currentState = START;
//                Serial.println("Robot returned to base. Mission completed.");
            }
            break;
    }
    button_pressed =false;
  //Serial.println("Loop complete");  
}

void initializeESCs(Servo &motor, Servo &motor2) {
    motor.write(89); // Initialize ESC
    Serial.println("LEFT:"+String(89)+"_RIGHT:"+ String(89));
    currentLeftSpeed = 89;
    currentRightSpeed = 89;
    motor2.write(89); // Initialize ESC
    delay(10000);     // Wait 10 seconds for ESCs to be ready
//    Serial.println("ESC Initialized");
}

void initializeESCs_wait(Servo &motor, Servo &motor2) {
    motor.write(89); // Initialize ESC
    digitalWrite(led1Pin, 1);
    digitalWrite(led2Pin, 1);
    Serial.println("LEFT:"+String(89)+"_RIGHT:"+ String(89));
    currentLeftSpeed = 89;
    currentRightSpeed = 89;
    motor2.write(89); // Initialize ESC
    delay(10000);     // Wait 30 seconds for ESCs to be ready
    digitalWrite(led1Pin, 0);
    digitalWrite(led2Pin, 0);
//    flash_LEDs();
//    Serial.println("Wait completed, should be in the water now.");
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

        Serial.println("Turning Left");
        setMotorSpeed(zeroThrottle, zeroThrottle + 30);
        while(abs(trueBearing - aimedBearing) > 10){
            //Serial.println("True"+String(trueBearing)+ " Aimed"+ String(aimedBearing));
            setTrueBearing();
            getAbstractedData();
         }
    } else {
       //turn right by generated degrees off current direction
        
        aimedBearing = (trueBearing - generateRandomAngle());
        aimedBearing = aimedBearing - 360.0 * static_cast<int>(aimedBearing / 360.0); // Manual modulo
        if (aimedBearing < 0) aimedBearing += 360.0; // Ensure the value is non-negative
        Serial.println("Turning Right");
        setMotorSpeed(zeroThrottle + 30, zeroThrottle);
        while(abs(trueBearing - aimedBearing) > 10){
            Serial.println("True"+String(trueBearing)+ " Aimed"+ String(aimedBearing));
            setTrueBearing();
            getAbstractedData();
         }
    }
    updateGPS();
//    if (isPointNearEdgeGeo(currentLat, currentLong, polygonLat, polygonLong, numVertices, threshold)) {
//        correctionCount++;
//        if (correctionCount >= 4) {
//            currentState = STUCK_DETECTED;
//        }
//    }
    currentState = STRAIGHT_NAVIGATION;
    waitPeriod = 0;

}

void returnToBase() {
    Serial.println("TEST");
    stopMotors();
    // calculatePathToBase();
    if (return_flag == false){
    getAbstractedData();
    aimedBearing = atan2(baseLong - currentLong, baseLat - currentLat) * 180 / PI;
    aimedBearing = aimedBearing - 360.0 * static_cast<int>(aimedBearing / 360.0); // Manual modulo
      if (aimedBearing < 0) aimedBearing += 360.0; // Ensure the value is non-negative

      Serial.println("Turning Left");
      setMotorSpeed(zeroThrottle, zeroThrottle + 30);
      while(abs(trueBearing - aimedBearing) > 10){
          //Serial.println("True"+String(trueBearing)+ " Aimed"+ String(aimedBearing));
          setTrueBearing();
          getAbstractedData();
        }
    }
    return_flag = true;
    Serial.println("Turning complete");
    setMotorSpeed(zeroThrottle + 20, zeroThrottle + 20); // Straight-line speed to base
    getAbstractedData();
    updateGPS();
}

void stopMotors() {
    setMotorSpeed(zeroThrottle, zeroThrottle); // Neutral throttle
     Serial.println("LEFT:"+String(zeroThrottle)+"_RIGHT:"+ String(zeroThrottle));
    currentLeftSpeed = zeroThrottle;
    currentRightSpeed = zeroThrottle;
}

void setMotorSpeed(int leftSpeed, int rightSpeed) {
    leftMotor.write(leftSpeed);
    Serial.println("LEFT:"+String(leftSpeed)+"_RIGHT:"+ String(rightSpeed));
    rightMotor.write(rightSpeed);
    currentLeftSpeed = leftSpeed;
    currentRightSpeed = rightSpeed;
}

void updateGPS() {
//  currentLat = getLat();
//  currentLong = getLong();
  
}

bool atBase() {
    // Simulate base detection
    // Calculate the Euclidean distance between current location and base
    double distance = sqrt(pow(currentLat - baseLat, 2) + pow(currentLong - baseLong, 2));
    
    // Check if the distance is within the threshold
    Serial.println("DISTANCE : "+ String(distance));
    return distance <= 100;
}

// void calculatePathToBase() {
//     // Simulate path calculation to the base
//     aimedBearing = atan2(baseLong - currentLong, baseLat - currentLat) * 180 / PI;
// }

float generateRandomAngle() {
     return random(90, 180);
}
void setTrueBearing(){
//    trueBearing = getIMUinfo();
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
// Helper function to convert latitude and longitude back to pixel coordinates
void convertToPixel(float lat, float lon, float& x, float& y) {
    const float gpsOriginLat = 37.7749; // Origin latitude
    const float gpsOriginLon = -122.4194; // Origin longitude
    const float pixelToDegreeScaleLat = 0.01; // Scaling factor for latitude
    const float pixelToDegreeScaleLon = 0.01; // Scaling factor for longitude
    const float baseX = 650; // Pixel base point x
    const float baseY = 700; // Pixel base point y

    // Convert lat/lon to pixel coordinates
    x = baseX + (lon - gpsOriginLon) / pixelToDegreeScaleLon;
    y = baseY - (lat - gpsOriginLat) / pixelToDegreeScaleLat;
}


float pointToSegmentDistanceGeo(float lat, float lon, float lat1, float lon1, float lat2, float lon2) {
    float x, y, x1, y1, x2, y2;

    // Convert all lat/lon values to pixel coordinates
    convertToPixel(lat, lon, x, y);
    convertToPixel(lat1, lon1, x1, y1);
    convertToPixel(lat2, lon2, x2, y2);

    // Treat the problem in pixel space
    float dx = x2 - x1; // x difference
    float dy = y2 - y1; // y difference

    // Degenerate segment case
    if (dx == 0 && dy == 0) {
        float dX = x - x1;
        float dY = y - y1;
        return sqrt(dX * dX + dY * dY); // Distance in pixels
    }

    // Project point onto line segment (clamped to segment bounds)
    float t = ((x - x1) * dx + (y - y1) * dy) / (dx * dx + dy * dy);
    t = fmax(0, fmin(1, t)); // Clamp t to the range [0, 1]

    // Closest point on segment
    float closestX = x1 + t * dx;
    float closestY = y1 + t * dy;

    // Distance from point to closest point on segment
    float dX = x - closestX;
    float dY = y - closestY;
    return sqrt(dX * dX + dY * dY); // Distance in pixels
}




//
//float pointToSegmentDistanceGeo(float lat, float lon, float lat1, float lon1, float lat2, float lon2) {
//    // Convert latitudes and longitudes to radians
//    lat *= M_PI / 180.0;
//    lon *= M_PI / 180.0;
//    lat1 *= M_PI / 180.0;
//    lon1 *= M_PI / 180.0;
//    lat2 *= M_PI / 180.0;
//    lon2 *= M_PI / 180.0;
//
//    // Approximation: Treat latitude/longitude as local Cartesian coordinates
//    float dx = (lon2 - lon1) * cos((lat1 + lat2) / 2.0); // Longitude difference scaled by average latitude
//    float dy = lat2 - lat1; // Latitude difference
//
//    // Degenerate segment case
//    if (dx == 0 && dy == 0) {
//        float dLat = lat - lat1;
//        float dLon = (lon - lon1) * cos(lat1);
//        return sqrt(dLat * dLat + dLon * dLon) * 111000.0; // Distance in meters
//    }
//
//    // Project point onto line segment (clamped to segment bounds)
//    float t = ((lat - lat1) * dy + (lon - lon1) * dx) / (dx * dx + dy * dy);
//    t = fmax(0, fmin(1, t)); // Clamp t to the range [0, 1]
//
//    // Closest point on segment
//    float closestLat = lat1 + t * dy;
//    float closestLon = lon1 + t * dx;
//
//    // Distance from point to closest point on segment
//    float dLat = lat - closestLat;
//    float dLon = (lon - closestLon) * cos(lat);
//    return sqrt(dLat * dLat + dLon * dLon) * 111000.0; // Distance in meters
//}
//

// Function to check if a point is near any edge of the polygon
bool isPointNearEdgeGeo(float latitude, float lon, const float* vertLat, const float* vertLon, int numVertices, float thresholdMeters) {
    for (int i = 0, j = numVertices - 1; i < numVertices; j = i++) {
        float dist = pointToSegmentDistanceGeo(latitude, lon, vertLat[i], vertLon[i], vertLat[j], vertLon[j]);
        //Serial.println("DISTANCE" + String(dist));
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

void getAbstractedData(){
   if (Serial.available()) {
        String data = Serial.readStringUntil('\n');
        Serial.println("LEFT:"+String(currentLeftSpeed)+"_RIGHT:"+ String(currentRightSpeed));
//        Serial.println("data recieved :" + String(data));

        int firstComma = data.indexOf(',');
        int secondComma = data.indexOf(',', firstComma + 1);
    
        // Extract latitude, longitude, and direction
        currentLat = data.substring(0, firstComma).toFloat();
        currentLong = data.substring(firstComma + 1, secondComma).toFloat();
        trueBearing = data.substring(secondComma + 1).toInt();
        Serial.println("data recieved :" + String(currentLat)+ " " +String(currentLong)+ " " +String(trueBearing));

            
    }
  
  }