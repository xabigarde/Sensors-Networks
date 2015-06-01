const int xPin = A0;
const int yPin = A1;
const int zPin = A2;
const int tempPin = A3;
const int lightPin = A4;


const int LED1 = 10;
const int LED2 = 11;
const int LED3 = 12;
const int LED4 = 13;

//ADXL335 accel(xPin, yPin, zPin, aref);

int minVal = 265;
int maxVal = 402;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);      // sets the serial port to 9600
}

void loop() {
  
  //read the analog values from the accelerometer
  int xRead = analogRead(xPin);
  int yRead = analogRead(yPin);
  int zRead = analogRead(zPin);
  
  int tempRead = analogRead(tempPin);
  int lightRead = analogRead(lightPin);

  //convert read values to degrees -90 to 90 - Needed for atan2
  int xAcc = map(xRead, 268, 408, -1000, 1000);
  int yAcc = map(yRead, 262, 398, -1000, 1000);
  int zAcc = map(zRead, 266, 401, -1000, 1000);
  
  // Get real acceleration
  double accSqrt = (xAcc * xAcc + yAcc * xAcc + zAcc * zAcc) - 1000000;
  
//  Serial.println("===== READ: ");
//  Serial.print(xRead, DEC);    // print the acceleration in the X axis
//  Serial.print(" ");
//  Serial.print(yRead, DEC);    // print the acceleration in the X axis
//  Serial.print(" ");
//  Serial.print(zRead, DEC);    // print the acceleration in the X axis
//  delay(300);              // wait 100ms for next reading
//  
    
  Serial.print(millis(), DEC);    // print the acceleration in the X axis
  Serial.print(";");
  Serial.print(xAcc, DEC);    // print the acceleration in the X axis
  Serial.print(";");
  Serial.print(yAcc, DEC);    // print the acceleration in the Y axis
  Serial.print(";"); 
  Serial.print(zAcc, DEC);  // print the acceleration in the Z axis
  Serial.print(";");
  Serial.print(accSqrt);  // Temperature readung
  Serial.print(";");
  Serial.print(tempRead, DEC);  // Temperature readung
  Serial.print(";");
  Serial.print(lightRead, DEC);  // print the light sensor readout
  Serial.println();
  
  

  
}
