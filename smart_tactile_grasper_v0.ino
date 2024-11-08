// Constants for equations and mechanical properties
const float maxJawAngle = 80.0;  // Maximum jaw opening angle (degrees)
const float maxActuatorMovement = 2.91;  // Maximum linear actuator movement (mm)
const float forceConstant = 488.76;  // Constant for converting bit value to force in N
const float adcResolution = 1023.0;  // ADC resolution for bit value
const float mechanicalAdvantage = 363.74 / 22.83;  // Ratio for converting Fgauge to Fjaws
const float maxFjaws = 18.0;  // Maximum allowable force at the grasper jaws in N
const float angleAdjustment = 5.0;  // Angle adjustment when exceeding max force (in degrees)

// Variables to store the current and desired jaw angles
float currentJawAngle = 0.0;  // Initial current jaw angle (assumed or measured)
float desiredJawAngle = 0.0;  // Desired jaw angle set by the user

// Function to calculate the actuator displacement (y) from a given jaw angle (x)
float calculateActuatorDisplacement(float jawAngle) {
    // Equation for displacement: y = 0.0367x - 0.0318
    return 0.0367 * jawAngle - 0.0318;
}

// Function to convert ADC bit value to force at the strain gauge (Fgauge)
float calculateForceAtStrainGauge(int bitValue) {
    return (forceConstant * bitValue) / adcResolution;
}

// Function to calculate the force at the grasper jaws (Fjaws) from Fgauge
float calculateForceAtJaws(float Fgauge) {
    return Fgauge / mechanicalAdvantage;
}

// Placeholder function to read the ADC bit value from the strain gauge
int readAdcBitValue() {
    // Replace with actual ADC reading logic
    return analogRead(A0);  // Example reading from pin A0
}

// Function to determine if the grasper jaws are opening or closing and adjust if force exceeds limits
void determineMotionDirection(float currentAngle, float desiredAngle) {
    if (desiredAngle > currentAngle) {
        Serial.println("Grasper jaws are opening.");
        if (calculateForceAtJaws(calculateForceAtStrainGauge(readAdcBitValue())) > maxFjaws) {
            Serial.println("Warning: Force at grasper jaws exceeds 18 N. Adjusting and halting.");
            desiredAngle -= angleAdjustment;  // Adjust angle for safety by closing
            moveToDesiredAngle();  // Re-attempt move with adjusted angle
            return;
        }
    } else if (desiredAngle < currentAngle) {
        Serial.println("Grasper jaws are closing.");
        if (calculateForceAtJaws(calculateForceAtStrainGauge(readAdcBitValue())) > maxFjaws) {
            Serial.println("Warning: Force at grasper jaws exceeds 18 N. Adjusting and halting.");
            desiredAngle += angleAdjustment;  // Adjust angle for safety by opening
            moveToDesiredAngle();  // Re-attempt move with adjusted angle
            return;
        }
    } else {
        Serial.println("Grasper jaws are not moving.");
    }
}

// Function to get user input for the desired jaw angle
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
        } else {
            Serial.println("Error: Desired jaw angle exceeds maximum allowed (80 degrees). Please re-enter.");
        }
    }
}

// Main loop to demonstrate calculations and motion detection
void loop() {
    getUserInputForJawAngle();  // Prompt user to enter the desired jaw angle
    moveToDesiredAngle();       // Move and update current jaw angle incrementally
    verifyJawAngle();           // Verify if current jaw angle matches the desired angle
    delay(1000);                // Wait before the next iteration
}

// Function to simulate incremental movement to the desired angle with continuous force checking
void moveToDesiredAngle() {
    Serial.print("Moving towards desired jaw angle: ");
    Serial.print(desiredJawAngle);
    Serial.println(" degrees.");
    
    // Define step size to simulate incremental movement
    float stepSize = 1.0;  // Adjust the angle by 1 degree per step for smooth simulation

    // Move incrementally towards the desired angle
    while (currentJawAngle != desiredJawAngle) {
        // Increment or decrement the current angle towards the desired angle
        if (currentJawAngle < desiredJawAngle) {
            currentJawAngle += stepSize;
            if (currentJawAngle > desiredJawAngle) currentJawAngle = desiredJawAngle;  // Avoid overshooting
        } else if (currentJawAngle > desiredJawAngle) {
            currentJawAngle -= stepSize;
            if (currentJawAngle < desiredJawAngle) currentJawAngle = desiredJawAngle;  // Avoid overshooting
        }

        // Check force at each step
        float Fgauge = calculateForceAtStrainGauge(readAdcBitValue());
        float Fjaws = calculateForceAtJaws(Fgauge);
        if (Fjaws > maxFjaws) {
            Serial.println("Warning: Force at grasper jaws exceeds 18 N. Adjusting and halting.");
            
            // Adjust angle based on the current motion direction
            if (currentJawAngle < desiredJawAngle) {
                currentJawAngle -= angleAdjustment;  // Close jaws by 5 degrees
            } else {
                currentJawAngle += angleAdjustment;  // Open jaws by 5 degrees
            }

            Serial.print("Adjusted angle to relieve force: ");
            Serial.print(currentJawAngle);
            Serial.println(" degrees.");
            Serial.println("System halted. Awaiting further instructions.");
            return;  // Halt the system
        }

        // Print the current position for debugging
        Serial.print("Current jaw angle: ");
        Serial.print(currentJawAngle);
        Serial.println(" degrees.");

        delay(100);  // Small delay for smooth operation
    }

    Serial.println("Reached desired angle.");
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
    Serial.begin(9600);  // Start serial communication for debugging
}
