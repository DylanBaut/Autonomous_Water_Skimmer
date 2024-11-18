#include <Servo.h>

Servo myservo; 
 
int pos = 0;

int zeroThrottle = 90; //90 Needs to be between 66 and 101.
int maxThrottle = 140; //140 seems to be where the maximum is. Above this doesn't give any extra speed.
int minThrottle = 60;  //60 This represents the "reverse" speed.
 
void setup()
{

Serial.begin(9600); //Pour some serial

myservo.attach(5); // attaches the servo on pin  to the servo object
}
 
void loop()
{
  
 
// Serial.println("ZERO TO MAX");
for(pos = zeroThrottle; pos < maxThrottle; pos += 1)  // goes from zero to max throttle
  {                                  
    myservo.write(pos);               // tell the ESC to position in variable 'pos'
    delay(200);                       // waits 200ms for the ESC to reach the position
    Serial.println(pos);              // print "pos"
  }

// Serial.println("TRIGGER ON");
myservo.write(89);               //APPARENTLY WRITING A VALUE ANYTHING LESS THAN ZERO THROTTLE FOR A SECOND OR TWO WILL TRIGGER THE ESC ON
                                  // THE ESC DOESNT EVEN NEED TO BE LESS THAN ZERO THROTTLE ON THE ESC'S STARTUP, JUST ANY POINT WILL TRIGGER IT ON
delay(2000);



  
}
