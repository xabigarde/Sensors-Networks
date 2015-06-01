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


// Running average for accelerometer sample smoothing
const int ACCSAMPLES = 5; // Samples to use
float accRavgStorage[ACCSAMPLES]; // storage for samples
int accRavgIndex = 0;
float accRafgSum = 0;
float accRavg = 0;

char* statusStrings[]={"idle", "walking", "stairs", "running"};
const int strWalking = 0;
const int strIdle = 1;
const int strStairs = 2;
const int strRunning= 3;

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
  float xAcc = (float)map(xRead, 268, 408, -1000, 1000);
  float yAcc = (float)map(yRead, 262, 398, -1000, 1000);
  float zAcc = (float)map(zRead, 266, 401, -1000, 1000);
  
  // Get real acceleration
  float accPow = (xAcc * xAcc + yAcc * yAcc + zAcc * zAcc);  
  float accSqrt = abs(sqrt(accPow) - 1000);
  
  // Save current value to buffer
  accRavgStorage[accRavgIndex] = accSqrt;
  
  // add current value to total sum
  accRafgSum = accRafgSum + accSqrt;
  
  // Substract first value from total sum
  accRafgSum = accRafgSum - accRavgStorage[(accRavgIndex + ACCSAMPLES - 1)%ACCSAMPLES];
  
  // increase sample
  accRavgIndex = (accRavgIndex+1)%ACCSAMPLES;  
  
  // calculate running average
  accRavg = accRafgSum / ACCSAMPLES;
  
  // Classify data
  /**
  
  000 <= ACC < 050 ... idle
  050 <= ACC < 200 ... walking
  200 <= ACC < 400 ... stairs
  400 <= ACC ... running   
  
  **/
  
  int accStatus = 0;
  
  if(accRavg < 50)  accStatus = strIdle;
  else if(accRavg >= 50 && accRavg < 200) accStatus = strWalking; // walking
  else if(accRavg > 200 && accRavg < 400) accStatus = strStairs;
  else accStatus = strRunning;
      
//  Serial.print(millis(), DEC);    // print the acceleration in the X axis
//  Serial.print(";");
//  Serial.print(xAcc);    // print the acceleration in the X axis
//  Serial.print(";");
//  Serial.print(yAcc);    // print the acceleration in the Y axis
//  Serial.print(";"); 
//  Serial.print(zAcc);  // print the acceleration in the Z axis
//  Serial.print(";");
//  Serial.print(accSqrt);  // Temperature readung
//  Serial.print(";");
//  Serial.print(accRavg);  // Temperature readung
////  Serial.print(";");
////  Serial.print(tempRead, DEC);  // Temperature readung
////  Serial.print(";");
////  Serial.print(lightRead, DEC);  // print the light sensor readout
//  Serial.println();
  
  Serial.println(statusStrings[accStatus]);

  
}
