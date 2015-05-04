//////////////////////////////////////////////////////////////////
// 2015 Sensors and Networks
// Project 1
// Felix Auer, Elena Zueva, Xabier Gardeazabal
//
// Simple code for the ADXL335 accelerometer and thermomether. Prints calculated orientation and temperature via serial (LCD and USB)
//////////////////////////////////////////////////////////////////

// Use the softwareserial library to create a new "soft" serial port
// for the display. This prevents display corruption when uploading code.
//#include <SoftwareSerial.h>

// ADXL335 accelerometer library
#include <ADXL335.h>

// Attach the serial display's RX line to digital pin 2
//SoftwareSerial mySerial(3, 2); // pin 2 = TX, pin 3 = RX (unused)
#define TIMECTL_MAXTICKS  4294967295L
#define TIMECTL_INIT      0

// Stuff for the click-rotary-encoder
#include <ClickEncoder.h>
#include <TimerOne.h>
ClickEncoder *encoder;
int16_t enc_last, enc_value;

//Timer constants
unsigned long flashTimeMark=0;  //a millisecond time stamp used by the IsTime() function. initialize to 0
unsigned long int flashTimeInterval=1000;  //How many milliseconds we want for the flash cycle. 1000mS is 1 second.

// LCD stuff
int dataPage = 0; // Identifier for the current data set that's going to be printed
const int MAX_PAGE = 10;
const int MIN_PAGE = 0;

//Analog read pins for the ADXL335 accelerometer
const int xPin = 0;
const int yPin = 1;
const int zPin = 2;
const float aref = 3.3;
ADXL335 accel(xPin, yPin, zPin, aref);

const int tempPin = 3; // Analog read pin for the temperature sensor

// Global variables to hold the maximum and minimum recorded temperature
double tempRead;
double maxTemp = -10000;
double minTemp = 10000;
double thermistorPad = 680;

// ALARM TRIGGER LIMITS
const double MAX_TEMPERATURE = 20;
const double MAX_TOTAL_ACCELERATION = 2;

// Digital Pins for the alarms
const int YELLOW_LED_1 = 13;
const int YELLOW_LED_2 = 12;
const int RED_LED_1 = 11;
const int RED_LED_2 = 10;

//The minimum and maximum values that came from the accelerometer while standing still (may need to change these)
const int minVal = 268;
const int maxVal = 407;

// Global variables to hold the caculated tilt values
double roll; 
double pitch;
double yaw;    // Will not ever change, because we don't have a compass

// Global variables to hold the calculated acceleration values
double x;
double y;
double z;

// Global variables to hold the maximum absolute (+/-) accelerations for all three axes in [g]
double maxX = -10000;
double maxY = -10000;
double maxZ = -10000;

void setup() {
  Serial.begin(9600);
  //mySerial.begin(9600); // set up serial port for 9600 baud
  initializeLCD();
  
  encoder = new ClickEncoder(4, 5, 6);
  
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  enc_last = -1;

  pinMode(YELLOW_LED_1, OUTPUT);
  pinMode(YELLOW_LED_2, OUTPUT);
  pinMode(RED_LED_1, OUTPUT);
  pinMode(RED_LED_2, OUTPUT);
}
 
void loop() {
  accel.update();  // poll the acceleromether (library function)
  getAccValues();   // Poll the accelerometer
  getTemperature(); // poll the thermomether
  getEconderValue(); // poll the encoder

  printData(); // prints the current data-page to the LCD and Serial bus (USB)

  delay(100);//just here to slow down the serial output - Easier to read
}

void checkAlarms()
{
  // Acceleration
  if( (x+y+z) > MAX_TOTAL_ACCELERATION)
    digitalWrite(YELLOW_LED_1, HIGH);
  else
    digitalWrite(YELLOW_LED_1, LOW);
  
  // Temperature
  if(tempRead > MAX_TEMPERATURE)
    digitalWrite(YELLOW_LED_2, HIGH);
  else
    digitalWrite(YELLOW_LED_2, LOW);
}

void getAccValues()
{
  // Read the acceleration values from the accelerometer (library function)
  x = accel.getX();
  y = accel.getY();
  z = accel.getZ();
  
  // Check if we have achieved a maximum acceleration in any axis
  if(abs(x) > maxX) maxX = abs(x);
  if(abs(y) > maxY) maxY = abs(y);
  if(abs(z) > maxZ) maxZ = abs(z);

  //read the analog values from the accelerometer
  int xRead = analogRead(xPin);
  int yRead = analogRead(yPin);
  int zRead = analogRead(zPin);

  //convert read values to degrees -90 to 90 - Needed for atan2
  int xAng = map(xRead, minVal, maxVal, -90, 90);
  int yAng = map(yRead, minVal, maxVal, -90, 90);
  int zAng = map(zRead, minVal, maxVal, -90, 90);

  // Used for manual calibration of min and max-values
  //  Serial.print("xRead: ");
  //  Serial.print(xRead);
  //  Serial.print(" | yRead: ");
  //  Serial.print(yRead);
  //  Serial.print(" | zRead: ");
  //  Serial.println(zRead);

  //Caculate 360deg values like so: atan2(-yAng, -zAng)
  //atan2 outputs the value of -π to π (radians)
  //We are then converting the radians to degrees
  roll = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  pitch = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  yaw = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);
}


void getTemperature()
{
  //tempRead =  ( analogRead(tempPin) / 1024.0) * 500;
  tempRead =  analogRead(tempPin);
  if(tempRead < minTemp)
    minTemp = tempRead;
  if(tempRead > maxTemp)
    maxTemp = tempRead;
}


void getEconderValue(){
  // Rotary encoder
  enc_value += encoder->getValue();
  
  if(enc_value > MAX_PAGE )
    enc_value = enc_value % MAX_PAGE;
  else if(enc_value < MIN_PAGE )
    enc_value = MAX_PAGE - abs(enc_value % MAX_PAGE);
  
  if (enc_value != enc_last) {
    enc_last = enc_value;
    dataPage = enc_last;
    Serial.print("Encoder Value: ");
    Serial.println(enc_value);
  }
}

void printData(){
  if( IsTime( &flashTimeMark, flashTimeInterval ) ) {  //Is it time to print the data?
    switch (dataPage) {
      case 0: // TILT
        printXYZ("0 Tilt", roll, pitch, 0);
        
        /*
        float rho;
        float phi;
        float theta;
        rho = accel.getRho();
        phi = accel.getPhi();
        theta = accel.getTheta();
        printXYZ("2.-Angles", rho, phi, theta);
        Serial.print("rho");
        Serial.println(rho);
        Serial.print("xRead: ");
        Serial.println(analogRead(yPin));
        Serial.print("X: ");
        Serial.println(roll);
        */
        
        break;
      case 1: // ACCELERATION
        printAccel("1 Accel", x, y, z);
        //Serial.print("Accel. X: ");
        //Serial.println(accel.getX());
        break;
      case 2: // MAX ACCELERATION
        printAccel("2 MAX", maxX, maxY, maxZ);
        break;        
      case 3: // Temperature
        printTemp("3 Temp", tempRead, minTemp, maxTemp);
      break;  
      case 4:

      break;
      default:
        // if nothing else matches, do the default
        // default is optional
        clearLCD();
    }

  }  
}

//////////////////////////////////////////////////
//		Timers		                //
//////////////////////////////////////////////////

//  Uses IsTime() to control the flash rahter then the delay() function.
int IsTime(unsigned long *timeMark, unsigned long timeInterval){
  unsigned long timeCurrent;
  unsigned long timeElapsed;
  int result=false;
  
  timeCurrent=millis();
  if(timeCurrent<*timeMark) {  //Rollover detected
    timeElapsed=(TIMECTL_MAXTICKS-*timeMark)+timeCurrent;  //elapsed=all the ticks to overflow + all the ticks since overflow
  }
  else {
    timeElapsed=timeCurrent-*timeMark;  
  }

  if(timeElapsed>=timeInterval) {
    *timeMark=timeCurrent;
    result=true;
  }
  return(result);  
}


// Interrupt service Routine
void timerIsr() {
  encoder->service();
}
