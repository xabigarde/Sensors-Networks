const int xPin = A0;
const int yPin = A1;
const int zPin = A2;
const int tempPin = A3;
const int lightPin = A4;
const int lightCalibrationPin = A5;


const int LEDgreen = 10;
const int LEDred = 11;
const int LEDyellow = 12;
const int LEDonboard = 13;

enum Color { RED, GREEN, BLUE };


void setup() {
  // put your setup code here, to run once:
  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDyellow, OUTPUT);
  pinMode(LEDred, OUTPUT);
  pinMode(LEDonboard, OUTPUT);
  
  
  Serial.begin(9600);      // sets the serial port to 9600
}

void loop() {

  // Process the accelerometer-data
  processAcc();

  // Detect bag Open or closed
  checkOpenClose();

}




bool checkOpenClose() {

  int lightRead = analogRead(lightPin);

  if (lightRead < 1000)
  {
    // Bag is open
    digitalWrite(LEDgreen, HIGH);
    return true;
  }
  else
  {
    digitalWrite(LEDgreen, LOW);
    return false;
  }

}

double processAcc()
{
  //read the analog values from the accelerometer
  int xRead = analogRead(xPin);
  int yRead = analogRead(yPin);
  int zRead = analogRead(zPin);

  // map sensor values within idle min and max ranges
  int xAcc = map(xRead, 268, 408, -1000, 1000);
  int yAcc = map(yRead, 262, 398, -1000, 1000);
  int zAcc = map(zRead, 266, 401, -1000, 1000);
  
  // Get real acceleration
  double accSqrt = sqrt(xAcc * xAcc + yAcc * xAcc + zAcc * zAcc) - 1000;

  return accSqrt;
}
