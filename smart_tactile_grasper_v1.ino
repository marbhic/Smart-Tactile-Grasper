#include <Servo.h>

Servo myservo;
const byte strainGaugePin = A0; 

float desiredJawAngle = 0; 
float currentJawAngle = 0;
float Fjaws = 0; 
int currentActuatorPos = 0;
int i = 0; //iterator helps determine if code should wait for new jaw angle

const int open_position = 1200;
const int close_position = 1000;
const float maxFjaws = 18;
const float maxJawAngle = 80;

void setup()
{
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // linear actuator is connected to pin 9
  myservo.attach(9);
  
  // set linear actuator to home position
  myservo.writeMicroseconds(1000);
}

void loop() {
  // ask user for desired jaw angle
  if (i == 0){
    getUserInputForJawAngle();
    }
  else{
    checkUserInputForJawAngle();
    }
    
  // determine whether to close or open jaws or do nothing
  if (desiredJawAngle < currentJawAngle){
    currentActuatorPos = opening();
    }
  else if (desiredJawAngle > currentJawAngle){
    currentActuatorPos = closing();
    }
  else if (desiredJawAngle == currentJawAngle){
    Serial.println("Desired jaw angle achieved");
    i = 0;
    }
  else{
    Serial.println("Error");
    i = 0;
    }
}


void getUserInputForJawAngle() {
    bool validInput = false;
    while (!validInput) {
        Serial.println("Enter desired jaw angle (0 to 80 degrees):");
        while (Serial.available() == 0) {
            // Wait for user input
        }
        desiredJawAngle = Serial.parseFloat();  // Read user input

        // Validate the input
        if (desiredJawAngle >= 0 && desiredJawAngle <= maxJawAngle) {
            validInput = true;  // Input is valid
        } 
        else {
            Serial.println("Error: Desired jaw angle exceeds maximum allowed (80 degrees). Please re-enter.");
        }
    }
}

void checkUserInputForJawAngle() {
    float newJawAngle = 0;
    if(Serial.available() > 0) {
        // Wait for user input
    }
    newJawAngle = Serial.parseFloat();  // Read user input

    // Validate the input
    if (newJawAngle >= 0 && newJawAngle <= maxJawAngle) {
        desiredJawAngle = newJawAngle;  // Input is valid
    } 
    else {
        Serial.println("Error: Desired jaw angle exceeds maximum allowed (80 degrees). Please re-enter.");
    }
}


//function to relate pusher rod force to jaw force
int get_force(){
  int strainGaugeForce = analogRead(strainGaugePin);

  // insert transfer function
  int jaw_force = 0;
  Serial.print("Current jaw force: ");
  Serial.print(jaw_force);
  Serial.println();
  return jaw_force; 
  } 
  
int getJawAngle(){
  // insert transfer function
  int jawCalculation = 0;
  //jaw_position = x + y
  Serial.print("Current jaw angle: ");
  Serial.print(jawCalculation);
  return jawCalculation; 
  }

int checkJawAngleForce(int openClose, int newActuatorPos){
   // openClose is 0 for closing and 1 for opening

   // get jaw position and force
   currentJawAngle = getJawAngle();
   Fjaws = get_force();

   // check if jaw force has reached max jaw force
   if (Fjaws >= maxFjaws){
    
      // if jaw is closing and reached max force, the actuator will move forward by one step
      if (openClose ==0){
        newActuatorPos = newActuatorPos + 1;
        }
        
      // if jaw is opening and reached max force, the actuator will move back by one step
      else if (openClose == 1){
        newActuatorPos = newActuatorPos - 1;
        }
        
      else{
        }
        
      // move actuator to new position to prevent tissue damage  
      Serial.println("Exceeded max force");  
      myservo.writeMicroseconds(newActuatorPos);
      delay(250);

      // reset iterator
      i = 0;
    }
    
  //check if current jaw position is equivalent to user input or has reached max actuator position
  else if (currentJawAngle == desiredJawAngle){
    Serial.println("Desired jaw angle achieved");
    //reset iterator
    i = 0;
    }
    
  //if none of the condition are met the while loop will continue  
  else if (newActuatorPos == close_position && openClose == 0){
    Serial.println("Min jaw angle achieved");
    //reset iterator
    i = 0;
    }
    
  else if (newActuatorPos == open_position && openClose == 1){
    Serial.println("Max jaw angle achieved");
    //reset iterator
    i = 0;
    }
    
  else{
    //if jaws have not reached desired jaw angle or any threshold, increment counter
    i = i + 1;
    }
  return newActuatorPos;
  
  }
  
int opening(){
  //declare variables needed for feedback loop
  int newActuatorPos = currentActuatorPos;
  
  //increment linear actuator
  newActuatorPos = newActuatorPos + 1;
  myservo.writeMicroseconds(newActuatorPos);
  delay(250);

  //perform checks to see if jaw angle has been achieved or has reached a threshold
  newActuatorPos = checkJawAngleForce(1,newActuatorPos);
  
  return newActuatorPos;
  } 
  
int closing(){
  //declare variables needed for feedback loop
  int newActuatorPos = currentActuatorPos;
  
  //increment linear actuator
  newActuatorPos = newActuatorPos - 1;
  myservo.writeMicroseconds(newActuatorPos);
  delay(250);

  //perform checks to see if jaw angle has been achieved or has reached a threshold
  newActuatorPos = checkJawAngleForce(0,newActuatorPos);
  
  return newActuatorPos;
  } 