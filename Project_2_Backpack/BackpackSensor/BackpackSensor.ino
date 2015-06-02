// BOF preprocessor bug prevent - insert me on top of your arduino-code
// From: http://www.a-control.de/arduino-fehler/?lang=en
#if 1
__asm volatile ("nop");
#endif

// Ucomment to see excessive debug output on console
//#define DEBUG


/* ==============================================
Temperature
============================================== */
const int tempPin = A3;
float temperature;

/* ==============================================
LEDs
============================================== */
const int LEDred = 11;
const int LEDyellow = 12;
const int LEDonboard = 13;

/* ==============================================
OPenClosed
============================================== */
const int lightPin = A4;
const int LEDgreen = 10;
const String strBagOpen = "open";
const String strBagClosed = "closed";
String backpackState = "";

/* ==============================================
Communication
============================================== */
String context;
unsigned long dataLastUpdate = 0;
const int dataSendInterval = 1000;

float latitude, longitude, velocity, distance; //incoming data from GPS sensor
String locality;
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

const int cWALKING = 50;
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
  checkOpenClose();

  //Get current temperature
  getTemperature();

  // Prepare data for sending (when time)
  prepareData();

  // Get gps-data and send data
  loop_BluetoothBridge(); // Call the loop() method of the BluetoothBridge sketch
  //  execution blocked until loop_BluetoothBridge() ends

  // Clear context to avoid sending data again
  //context = "";
}

// Prepares data to be sent to the client
void prepareData()
{
  if (millis() - dataLastUpdate >= dataSendInterval)
  {

#ifdef DEBUG
    Serial.println("Prepare Data");
#endif

    //context = activity+" "+backpackState+" "+temperature+" "+locality;

    // Prepare data to send
    context = accStatus + " " + backpackState + " " + temperature + " " + locality;
    //context = accStatus + " " + backpackState + " " + temperature + " " + " nope ";

    Serial.print("Data to send: ");
    Serial.println(context);

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
    backpackState = strBagOpen;
    return true;
  }
  else
  {
    digitalWrite(LEDgreen, LOW);
    backpackState = strBagClosed;
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

float getTemperature()
{

  temperature = Thermister(analogRead(tempPin));

#ifdef DEBUG
  Serial.println(temperature);
#endif

  return temperature;
}

float Thermister(int RawADC) {  //Function to perform the fancy math of the Steinhart-Hart equation
  float Temp;
  Temp = log(((10240000 / RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp )) * Temp );
  Temp = Temp - 273.15;              // Convert Kelvin to Celsius
  //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Celsius to Fahrenheit - comment out this line if you need Celsius
  return Temp + 14;
}
