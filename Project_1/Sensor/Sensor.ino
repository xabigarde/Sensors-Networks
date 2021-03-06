//////////////////////////////////////////////////////////////////
// 2015 Sensors and Networks
// Project 1
// Felix Auer, Elena Zueva, Xabier Gardeazabal
//
// Simple code for the ADXL335 accelerometer and thermomether. Prints calculated orientation and temperature via serial (LCD and USB)
//////////////////////////////////////////////////////////////////

#include <math.h>         //loads the more advanced math functions
#include <ADXL335.h>       // ADXL335 accelerometer library
#include <avr/pgmspace.h>  // For the thermoresistor

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
const int MAX_PAGE = 4;
const int MIN_PAGE = 0;

//Analog read pins for the ADXL335 accelerometer
const int xPin = A0;
const int yPin = A1;
const int zPin = A2;
const float aref = 3.3;
ADXL335 accel(xPin, yPin, zPin, aref);

const int tempPin = A3; // Analog read pin for the thermoresustir ( // 10ktherm & 10k resistor as divider )
// Global variables to hold the maximum and minimum recorded temperature
double tempRead;
double maxTemp = -10000;
double minTemp = 10000;
double thermistorPad = 680;

// Big lookup Table (approx 750 entries), subtract 238 from ADC reading to start at 0*C. Entries in 10ths of degree i.e. 242 = 24.2*C Covers 0*C to 150*C For 10k resistor/10k thermistor voltage divider w/ therm on the + side.
//const int temps[] PROGMEM = { 0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 143, 144, 145, 146, 147, 148, 149, 150, 151, 151, 152, 153, 154, 155, 156, 157, 158, 159, 159, 160, 161, 162, 163, 164, 165, 166, 167, 167, 168, 169, 170, 171, 172, 173, 174, 175, 175, 176, 177, 178, 179, 180, 181, 182, 182, 183, 184, 185, 186, 187, 188, 189, 190, 190, 191, 192, 193, 194, 195, 196, 197, 197, 198, 199, 200, 201, 202, 203, 204, 205, 205, 206, 207, 208, 209, 210, 211, 212, 212, 213, 214, 215, 216, 217, 218, 219, 220, 220, 221, 222, 223, 224, 225, 226, 227, 228, 228, 229, 230, 231, 232, 233, 234, 235, 235, 236, 237, 238, 239, 240, 241, 242, 243, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 255, 256, 257, 258, 259, 260, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 422, 423, 424, 425, 426, 427, 428, 429, 430, 432, 433, 434, 435, 436, 437, 438, 439, 441, 442, 443, 444, 445, 446, 448, 449, 450, 451, 452, 453, 455, 456, 457, 458, 459, 460, 462, 463, 464, 465, 466, 468, 469, 470, 471, 472, 474, 475, 476, 477, 479, 480, 481, 482, 484, 485, 486, 487, 489, 490, 491, 492, 494, 495, 496, 498, 499, 500, 501, 503, 504, 505, 507, 508, 509, 511, 512, 513, 515, 516, 517, 519, 520, 521, 523, 524, 525, 527, 528, 530, 531, 532, 534, 535, 537, 538, 539, 541, 542, 544, 545, 547, 548, 550, 551, 552, 554, 555, 557, 558, 560, 561, 563, 564, 566, 567, 569, 570, 572, 574, 575, 577, 578, 580, 581, 583, 585, 586, 588, 589, 591, 593, 594, 596, 598, 599, 601, 603, 604, 606, 608, 609, 611, 613, 614, 616, 618, 620, 621, 623, 625, 627, 628, 630, 632, 634, 636, 638, 639, 641, 643, 645, 647, 649, 651, 653, 654, 656, 658, 660, 662, 664, 666, 668, 670, 672, 674, 676, 678, 680, 683, 685, 687, 689, 691, 693, 695, 697, 700, 702, 704, 706, 708, 711, 713, 715, 718, 720, 722, 725, 727, 729, 732, 734, 737, 739, 741, 744, 746, 749, 752, 754, 757, 759, 762, 764, 767, 770, 773, 775, 778, 781, 784, 786, 789, 792, 795, 798, 801, 804, 807, 810, 813, 816, 819, 822, 825, 829, 832, 835, 838, 842, 845, 848, 852, 855, 859, 862, 866, 869, 873, 877, 881, 884, 888, 892, 896, 900, 904, 908, 912, 916, 920, 925, 929, 933, 938, 942, 947, 952, 956, 961, 966, 971, 976, 981, 986, 991, 997, 1002, 1007, 1013, 1019, 1024, 1030, 1036, 1042, 1049, 1055, 1061, 1068, 1075, 1082, 1088, 1096, 1103, 1110, 1118, 1126, 1134, 1142, 1150, 1159, 1168, 1177, 1186, 1196, 1206, 1216, 1226, 1237, 1248, 1260, 1272, 1284, 1297, 1310, 1324, 1338, 1353, 1369, 1385, 1402, 1420, 1439, 1459, 1480, 1502 };


// ALARM TRIGGER LIMITS
const double MAX_TEMPERATURE = 25;
const double MAX_TOTAL_ACCELERATION = 4.5;

// Digital Pins for the alarms
const int YELLOW_LED_1 = 13;
const int YELLOW_LED_2 = 12;
const int RED_LED_1 = 11;
const int RED_LED_2 = 10;

//PROTOCOL const
const char START = '$';
const char STOP = '*';
const char CHECKSUM = '#';
const char SEPERATOR = ';';	
	//Thermister
const int REQ_TEMP = 1;
const int RESP_TEMP = 2;
const int REQ_MAX_TEMP = 3;
const int RESP_MAX_TEMP = 4;
const int REQ_MIN_TEMP = 5;
const int RESP_MIN_TEMP = 6;	
	//Accelerometer
const  int REQ_CURR_X = 10;
const  int RESP_CURR_X = 11;
const  int REQ_CURR_Y = 12;
const  int RESP_CURR_Y = 13;
const  int REQ_CURR_Z = 14;
const  int RESP_CURR_Z = 15;
const  int REQ_MAX_X = 16;
const  int RESP_MAX_X = 17;
const  int REQ_MAX_Y = 18;
const  int RESP_MAX_Y = 19;
const  int REQ_MAX_Z = 20;
const  int RESP_MAX_Z = 21;
const  int REQ_TILT = 22;
const  int RESP_TILT = 23;
const  int REQ_PITCH = 24;
const  int RESP_PITCH = 25;
const  int REQ_ROLL = 26;
const  int RESP_ROLL = 27;	
//Operational
	
const  int MAX_TEMP_TRESHOLD = 52;
const  int MAX_ACC_TRESHOLD = 67;
const  int ALARM_ACC = 42;

const  int RESET_MIN_MAX = 43;


//from java
String content = "";
char character;


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
  
  checkAlarms();    // check if any alarms must be triggered
  checkResetButton();
  
  //reads from java and changes thresholds
  /* while(Serial.available()) {
      character = Serial.read();
      content.concat(character);
  }

  if (content != "") {
    Serial.println("What I get here:"+content);
    setupNewVal(content);
  }

  */
  sendDataToJava();
  
  delay(100);//just here to slow down the serial output - Easier to read
}

void checkResetButton()
{
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked){
      switch (dataPage) {
        case 0: // TILT
          break;
        case 1: // ACCELERATION
          break;
        case 2: // MAX ACCELERATION
          maxX = -10000;
          maxY = -10000;
          maxZ = -10000;
          break;        
        case 3: // Temperature
          maxTemp = -10000;
          minTemp = 10000;
          break;  
        case 4:
          break;
      } //switch
    } //if
  } //if
}

void checkAlarms()
{
  // Acceleration
  double totalAccel = abs(x)+abs(y)+abs(z);
  /*
  Serial.println(totalAccel);
  Serial.print("X: ");
  Serial.print(x);
  Serial.print("y: ");
  Serial.print(y);
  Serial.print("z: ");
  Serial.println(z);
  */
  if( totalAccel > MAX_TOTAL_ACCELERATION)
    digitalWrite(YELLOW_LED_1, HIGH);
  else
    digitalWrite(YELLOW_LED_1, LOW);
  
  // Temperature
  if(tempRead > MAX_TEMPERATURE)
    digitalWrite(YELLOW_LED_2, HIGH);
  else
    digitalWrite(YELLOW_LED_2, LOW);
    
  // Accelerometer plausibility check
  if(analogRead(xPin) > 600 && analogRead(yPin) > 600 && analogRead(zPin) > 66)
    digitalWrite(RED_LED_1, HIGH);
  else
    digitalWrite(RED_LED_1, LOW);
  
  // Thermistor plausibility check
  Serial.println(analogRead(tempPin));
  if(analogRead(tempPin) < 150 || analogRead(tempPin) > 700)
      digitalWrite(RED_LED_2, HIGH);
  else
    digitalWrite(RED_LED_2, LOW);
}

void getAccValues()
{
  // Read the acceleration values from the accelerometer (library function gives acceleration badly calibrated -> correct to set it to 0)
  x = accel.getX()+1.6;
  y = accel.getY()+1.7;
  z = accel.getZ()+2.1;
  
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
   /*
   Serial.print("xRead: ");
   Serial.print(xRead);
   Serial.print(" | yRead: ");
   Serial.print(yRead);
   Serial.print(" | zRead: ");
   Serial.println(zRead);
   */
   

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
  /*
  int therm =  analogRead(tempPin) -238;
  therm= pgm_read_word(&temps[therm]);
  tempRead = therm/10;
  */
  tempRead = Thermister(analogRead(tempPin));
  if(tempRead < minTemp)
    minTemp = tempRead;
  if(tempRead > maxTemp)
    maxTemp = tempRead;
  /*
  Serial.print("Temp pin: ");
  Serial.println(analogRead(tempPin));
  Serial.print("Temperature: ");
  Serial.println(tempRead);
  */
}

double Thermister(int RawADC) {  //Function to perform the fancy math of the Steinhart-Hart equation
 double Temp;
 Temp = log(((10240000/RawADC) - 10000));
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
 Temp = Temp - 273.15;              // Convert Kelvin to Celsius
 //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Celsius to Fahrenheit - comment out this line if you need Celsius
 return Temp+14;
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
        printTilt("0 Tilt", roll, pitch, yaw);
        
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


/**********PRINTING PROTOCOL STUFF********************************/
String protocolData(int _cmd, double _val){
    String command = START + String(_cmd) + SEPERATOR + String(_val);
    String str = command + CHECKSUM + command.length()+ STOP;
    return(str); 
}

void sendDataToJava(){
  // ROLL PITCH
        Serial.println(protocolData(RESP_ROLL, roll));
        Serial.println(protocolData(RESP_PITCH, pitch));       
  // ACCELERATION
        Serial.println(protocolData(RESP_CURR_X, x));
        Serial.println(protocolData(RESP_CURR_Y, y));
        Serial.println(protocolData(RESP_CURR_Z, z));
    // MAX ACCELERATION
        Serial.println(protocolData(RESP_MAX_X, maxX));
        Serial.println(protocolData(RESP_MAX_Y, maxY));
        Serial.println(protocolData(RESP_MAX_Z, maxZ));
   // Temperature
        Serial.println(protocolData(RESP_TEMP, tempRead));
        Serial.println(protocolData(RESP_MAX_TEMP, minTemp));
        Serial.println(protocolData(RESP_MIN_TEMP, maxTemp));
  //ALARM THRESHOLDS
      Serial.println(protocolData(MAX_TEMP_TRESHOLD, MAX_TEMPERATURE));
      Serial.println(protocolData(MAX_ACC_TRESHOLD, MAX_TOTAL_ACCELERATION));

    Serial.flush();
   
}
void setupNewVal(String content){
    int startIndex = content.indexOf(START);
    int sepIndex = content.indexOf(SEPERATOR);
    int shecksumIndex = content.indexOf(CHECKSUM);
    
    int cmd = content.substring(startIndex+1, sepIndex).toInt();
    String  valStr = content.substring(sepIndex+1, shecksumIndex);
    char buf[valStr.length()];
    valStr.toCharArray(buf,valStr.length()+1);
    double val=atof(buf);
    
    Serial.println(cmd,val);
}
/**********PRINTING PROTOCOL STUFF********************************/
