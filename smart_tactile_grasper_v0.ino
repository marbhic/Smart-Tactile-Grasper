#include <Servo.h>

// Constants for equations and mechanical properties
const float maxJawAngle = 80.0;  // Maximum jaw opening angle (degrees)
const float maxActuatorMovement = 2.91;  // Maximum linear actuator movement (mm)
const float forceConstant = 488.76;  // Constant for converting bit value to force in N
const float adcResolution = 1023.0;  // ADC resolution for bit value
const float mechanicalAdvantage = 363.74 / 22.83;  // Ratio for converting Fgauge to Fjaws
const float maxFjaws = 18.0;  // Maximum allowable force at the grasper jaws in N
const float stepSize = 1.0;  // Angle increment for smooth movement

// Variables to store the current and desired jaw angles
float currentJawAngle = 0.0;  // Initial current jaw angle (assumed or measured)
float previousJawAngle = 0.0; // Stores the last valid jaw angle before each step
float desiredJawAngle = 0.0;  // Desired jaw angle set by the user
bool systemHalted = false;    // System halt flag

// Servo and strain gauge setup
Servo myservo;
const byte strain_gauge_pin = A0;  // Strain gauge connected to analog pin A0

// Timing variables for non-blocking delays
unsigned long lastInputTime = 0;
unsigned long lastMoveTime = 0;
const unsigned long inputInterval = 1000;
const unsigned long moveInterval = 100;

// Function to calculate the actuator displacement (y) from a given jaw angle (x)
float calculateActuatorDisplacement(float jawAngle) {
    return 0.0367 * jawAngle - 0.0318;
}

// Function to convert displacement to pulse width for actuator control (0 to 2000 us)
int calculateActuatorPulseWidth(float jawAngle) {
    float displacement = calculateActuatorDisplacement(jawAngle);
    float pulseWidth = (displacement / maxActuatorMovement) * 2000.0;
    return static_cast<int>(constrain(pulseWidth, 0, 2000));
}

// Function to convert ADC bit value to force at the strain gauge (Fgauge)
float calculateForceAtStrainGauge(int bitValue) {
    return (forceConstant * bitValue) / adcResolution;
}

// Function to calculate the force at the grasper jaws (Fjaws) from Fgauge
float calculateForceAtJaws(float Fgauge) {
    return Fgauge / mechanicalAdvantage;
}

// Function to read the strain gauge value and convert it to force
float readForceAtJaws() {
    int sensorValue = analogRead(strain_gauge_pin);
    float Fgauge = calculateForceAtStrainGauge(sensorValue);
    return calculateForceAtJaws(Fgauge);
}

// Function to get user input for the desired jaw angle and handle system halt reset
void getUserInputForJawAngle() {
    bool validInput = false;
    while (!validInput) {
        Serial.println("Enter desired jaw angle (0 to 80 degrees):");
        while (Serial.available() == 0) {
            // Allow other tasks to proceed if needed
        }
        desiredJawAngle = Serial.parseFloat();

        if (desiredJawAngle >= 0 && desiredJawAngle <= maxJawAngle) {
            validInput = true;
            if (systemHalted) {
                Serial.println("System was halted due to excessive force. Type 'resume' to continue.");
                while (Serial.readString() != "resume") {
                    Serial.println("Awaiting 'resume' command to continue.");
                }
                systemHalted = false;  // Reset halt flag after user confirmation
                Serial.println("System resumed from halt.");
            }
        } else {
            Serial.println("Error: Desired jaw angle exceeds maximum allowed (80 degrees). Please re-enter.");
        }
    }
}

// Main loop to handle periodic tasks without blocking
void loop() {
    if (millis() - lastInputTime >= inputInterval) {
        lastInputTime = millis();
        if (!systemHalted) {
            getUserInputForJawAngle();  // Prompt user to enter the desired jaw angle
            moveToDesiredAngle();       // Move and update current jaw angle incrementally
            verifyJawAngle();           // Verify if current jaw angle matches the desired angle
        } else {
            Serial.println("System is halted. Awaiting user input to resume.");
        }
    }
}

// Function to simulate incremental movement to the desired angle with continuous force checking
void moveToDesiredAngle() {
    Serial.print("Moving towards desired jaw angle: ");
    Serial.print(desiredJawAngle);
    Serial.println(" degrees.");

    if (currentJawAngle != desiredJawAngle && millis() - lastMoveTime >= moveInterval) {
        lastMoveTime = millis();
        previousJawAngle = currentJawAngle;  // Save the previous angle before each step

        // Adjust current jaw angle towards desired angle
        if (currentJawAngle < desiredJawAngle) {
            currentJawAngle += stepSize;
            if (currentJawAngle > desiredJawAngle) currentJawAngle = desiredJawAngle;
        } else if (currentJawAngle > desiredJawAngle) {
            currentJawAngle -= stepSize;
            if (currentJawAngle < desiredJawAngle) currentJawAngle = desiredJawAngle;
        }

        // Read force and handle over-limit case
        float Fjaws = readForceAtJaws();
        Serial.print("Force at grasper jaws: ");
        Serial.print(Fjaws);
        Serial.println(" N");

        if (Fjaws > maxFjaws) {
            Serial.println("Warning: Force at grasper jaws exceeds 18 N. Reverting to previous angle.");
            currentJawAngle = previousJawAngle;  // Revert to last safe position
            int pulseWidth = calculateActuatorPulseWidth(currentJawAngle);
            myservo.writeMicroseconds(pulseWidth);
            Serial.print("Reverted to previous angle: ");
            Serial.print(currentJawAngle);
            Serial.println(" degrees.");
            Serial.println("System halted. Awaiting further instructions.");
            systemHalted = true;  // Set system to halt
            return;
        }

        // Update servo position
        int pulseWidth = calculateActuatorPulseWidth(currentJawAngle);
        myservo.writeMicroseconds(pulseWidth);
        Serial.print("Current jaw angle: ");
        Serial.print(currentJawAngle);
        Serial.println(" degrees.");
    }
}

// Function to verify if the current jaw angle matches the desired jaw angle
void verifyJawAngle() {
    if (currentJawAngle == desiredJawAngle) {
        Serial.println("Verification successful: Current jaw angle matches the desired angle.");
    } else {
        Serial.println("Verification failed: Current jaw angle does not match the desired angle.");
        Serial.print("Current jaw angle: ");
        Serial.print(currentJawAngle);
        Serial.print(" degrees, Desired jaw angle: ");
        Serial.print(desiredJawAngle);
        Serial.println(" degrees.");
        Serial.println("Re-initiating process to reach the desired angle.");
        moveToDesiredAngle();  // Re-attempt movement
    }
}

// Setup function to initialize the system
void setup() {
    Serial.begin(9600);
    myservo.attach(9);
}
