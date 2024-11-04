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
    if (jawAngle > maxJawAngle) {
        jawAngle = maxJawAngle;  // Limit to max allowable angle
    } else if (jawAngle < 0) {
        jawAngle = 0;  // Ensure angle is not negative
    }

    // Equation for displacement: y = 0.0367x - 0.0318
    float displacement = 0.0367 * jawAngle - 0.0318;
    return displacement;
}

// Function to convert ADC bit value to force at the strain gauge (Fgauge)
float calculateForceAtStrainGauge(int bitValue) {
    float Fgauge = (forceConstant * bitValue) / adcResolution;
    return Fgauge;
}

// Function to calculate the force at the grasper jaws (Fjaws) from Fgauge
float calculateForceAtJaws(float Fgauge) {
    float Fjaws = Fgauge / mechanicalAdvantage;
    return Fjaws;
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
            float SafetyAngle = desiredAngle - angleAdjustment; // Calculate safety angle for closing
            desiredAngle = SafetyAngle;  // Update desired angle to the safety angle
            float actuatorDisplacement = calculateActuatorDisplacement(SafetyAngle);
            
            Serial.print("Adjusted actuator displacement for ");
            Serial.print(SafetyAngle);
            Serial.print(" degrees: ");
            Serial.print(actuatorDisplacement);
            Serial.println(" mm");
            Serial.println("System halted. Awaiting further instructions.");
            getUserInputForJawAngle();  // Prompt user to input a new desired jaw angle
            return;
        }
    } else if (desiredAngle < currentAngle) {
        Serial.println("Grasper jaws are closing.");
        if (calculateForceAtJaws(calculateForceAtStrainGauge(readAdcBitValue())) > maxFjaws) {
            Serial.println("Warning: Force at grasper jaws exceeds 18 N. Adjusting and halting.");
            float SafetyAngle = desiredAngle + angleAdjustment; // Calculate safety angle for opening
            desiredAngle = SafetyAngle;  // Update desired angle to the safety angle
            float actuatorDisplacement = calculateActuatorDisplacement(SafetyAngle);
            
            Serial.print("Adjusted actuator displacement for ");
            Serial.print(SafetyAngle);
            Serial.print(" degrees: ");
            Serial.print(actuatorDisplacement);
            Serial.println(" mm");
            Serial.println("System halted. Awaiting further instructions.");
            getUserInputForJawAngle();  // Prompt user to input a new desired jaw angle
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
    getUserInputForJawAngle();  // Prompt user for the desired jaw angle

    // Calculate the actuator displacement needed for the desired jaw angle
    float actuatorDisplacement = calculateActuatorDisplacement(desiredJawAngle);
    Serial.print("Calculated actuator displacement for ");
    Serial.print(desiredJawAngle);
    Serial.print(" degrees: ");
    Serial.print(actuatorDisplacement);
    Serial.println(" mm");

    // Read ADC bit value from the strain gauge and calculate forces
    int adcBitValue = readAdcBitValue();
    float Fgauge = calculateForceAtStrainGauge(adcBitValue);
    float Fjaws = calculateForceAtJaws(Fgauge);

    // Display force readings
    Serial.print("ADC bit value: ");
    Serial.print(adcBitValue);
    Serial.print(" | Force at strain gauge: ");
    Serial.print(Fgauge);
    Serial.print(" N | Force at grasper jaws: ");
    Serial.print(Fjaws);
    Serial.println(" N");

    // Determine if the grasper jaws are opening or closing and check force limit
    determineMotionDirection(currentJawAngle, desiredJawAngle);

    // Update the current jaw angle to the desired angle after movement (simulated)
    currentJawAngle = desiredJawAngle;

    delay(1000);  // Wait before the next reading
}

// Setup function to initialize the system
void setup() {
    Serial.begin(9600);  // Start serial communication for debugging
}
