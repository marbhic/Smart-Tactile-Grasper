#include <Servo.h>

//abunchoftesting
//moretesting2

Servo myservo;
const byte strain_gauge_pin = A0; 

void setup()
{
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // linear actuator is connected to pin 9
  myservo.attach(9);
  
  // set servo to mid-point
//  myservo.writeMicroseconds(1000);  
}

void loop() {
  int sensorValue = analogRead(strain_gauge_pin);
  // print out the value you read:
  Serial.println(sensorValue);
  delay(100);        // delay in between reads for stability
  }

