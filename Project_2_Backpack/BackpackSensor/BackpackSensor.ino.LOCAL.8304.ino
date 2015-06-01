// BOF preprocessor bug prevent - insert me on top of your arduino-code
// From: http://www.a-control.de/arduino-fehler/?lang=en
#if 1
__asm volatile ("nop");
#endif

//#define DEBUG

const int tempPin = A3;
const int lightPin = A4;

/* ==============================================
LEDs
============================================== */

const int LEDred = 11;
const int LEDyellow = 12;
const int LEDonboard = 13;

/* ==============================================
OPenClosed
============================================== */
const int LEDgreen = 10;
bool isOpen;
const String strBagOpen = "open";
const String strBagClosed = "closed";

/* ==============================================
Communication
============================================== */
String context;
unsigned long dataLastUpdate = 0;
const int dataSendInterval = 1000;

/* ==============================================
Accelerometer setup
============================================== */
const int xPin = A0;
const int yPin = A1;
const int zPin = A2;

const int ACCSAMPLES = 5; // Samples to use
float accRavgStorage[ACCSAMPLES]; // storage for samples
int accRavgIndex = 0;
float accRafgSum = 0;
float accRavg = 0;

const String strIdle = "idle";
const String strWalking = "walking";
const String strStairs = "stairs";
const String strRunning = "running";
String accStatus = "";

const int cWALKING = 30;
const int cSTAIRS = 200;
const int cRUNNING = 400;

// timeout for accdetection
const int ACCTimeOut = 5000;
unsigned long accLastUpdate = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDyellow, OUTPUT);
  pinMode(LEDred, OUTPUT);
  pinMode(LEDonboard, OUTPUT);


  Serial.begin(9600);      // sets the serial port to 9600

  setup_BluetoothBridge(); // Call the setup() method of the BluetoothBridge sketch
}

void loop() {

#ifdef DEBUG
  Serial.println("loop");
#endif

  // Process the accelerometer-data
  processAcc();

  // Detect bag Open or closed
  isOpen = checkOpenClose();

  prepareData();

  loop_BluetoothBridge(); // Call the loop() method of the BluetoothBridge sketch
  //  execution blocked until loop_BluetoothBridge() ends

  context = "";



}


void prepareData()
{
  if (millis() - dataLastUpdate >= dataSendInterval)
  {

#ifdef DEBUG
    Serial.println("Prepare Data");
#endif

    // Prepare data to send
    context = "blah";

    // Update data send time
    dataLastUpdate = millis();
  }



}

bool checkOpenClose() {

#ifdef DEBUG
  Serial.println("Check open close");
#endif

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

String processAcc()
{
#ifdef DEBUG
  Serial.println("Process Acc");
#endif

  //read the analog values from the accelerometer
  int xRead = analogRead(xPin);
  int yRead = analogRead(yPin);
  int zRead = analogRead(zPin);

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

  // Substract oldest buffer-value from total sum
  accRafgSum = accRafgSum - accRavgStorage[(accRavgIndex + ACCSAMPLES - 1) % ACCSAMPLES];

  // increase ring-buffer index
  accRavgIndex = (accRavgIndex + 1) % ACCSAMPLES;

  // calculate running average
  float newAccRavg = accRafgSum / ACCSAMPLES;

#ifdef DEBUG
  Serial.print("New Acceleration (RAVG): ");
  Serial.println(newAccRavg);
#endif

  // Update status only when certain time has passed or value is higher
  if (newAccRavg > accRavg || (millis() - accLastUpdate) > ACCTimeOut)
  {
#ifdef DEBUG
    Serial.println("Update");
#endif

    // Update values
    accRavg = newAccRavg;
    accLastUpdate = millis();

    // Classify data
    /**

    000 <= ACC < 050 ... idle
    050 <= ACC < 200 ... walking
    200 <= ACC < 400 ... stairs
    400 <= ACC ... running

    **/

    String newAccStatus = "";

    if (accRavg < cWALKING)  newAccStatus = strIdle;
    else if (accRavg >= cWALKING && accRavg < cSTAIRS) newAccStatus = strWalking; // walking
    else if (accRavg > cSTAIRS && accRavg < cRUNNING) newAccStatus = strStairs;
    else newAccStatus = strRunning;

    if (newAccStatus != accStatus)
    {
      accStatus = newAccStatus;

      Serial.print(accStatus);
      Serial.print(" (");
      Serial.print(accRavg);
      Serial.println(")");

    }
  }

  return accStatus;
}
