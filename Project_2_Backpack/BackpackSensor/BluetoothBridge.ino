#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <SoftwareSerial.h>

//A0 (Analog 0) pin is an Light Diode Resistor (LDR)
int ldr =0;

#define rxPin 2 // RX address for the Bluetooth serial port
#define txPin 3 // TX address for the Bluetooth serial port

// Configura un nuevo puerto serie
SoftwareSerial btSerial = SoftwareSerial(rxPin, txPin);

// Variables for incoming Bluetooth-serial data bytes
byte in_byte;  // auxiliar variable for the STX and ETX bytes
byte in_msgID; // Message ID byte
byte in_seqNum; // Sequence number byte 
byte in_dlc[4]; // DLC bytes (size of the payload)
int in_payloadSize; // Numbere of bytes in the payload
byte in_crc[4]; // CRC bytes

const int btPower = 8; // Power switch pin address for the Bluetooth antenna
boolean started = false; // control flag
boolean flag_seconds = false; // control flag
int seconds = 0;
int ticks = 0;
const int CONTEXT_SEND_FREQENCY = 999; // miliseconds

// Variables for writing messages to the Bluetooth serial
const int STX = 0x02; // STX byte
const int MSGID_PING = 0x26; // Ping data-type byte
const int MSGID_STRESS= 0x27; // Stress data-type byte
const int DLC = 55; // 
const int ETX = 0x03; // ETX byte
String payload = ""; 
int frameNum = 0;

void setup_BluetoothBridge() {
  //Setup Timer2 to fire every 1ms
  TCCR2B = 0x00;        //Disbale Timer2 while we set it up
  TCNT2  = 130;         //Reset Timer Count to 130 out of 255
  TIFR2  = 0x00;        //Timer2 INT Flag Reg: Clear Timer Overflow Flag
  TIMSK2 = 0x01;        //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
  TCCR2A = 0x00;        //Timer2 Control Reg A: Wave Gen Mode normal
  TCCR2B = 0x05;        //Timer2 Control Reg B: Timer Prescaler set to 128
  
  //Serial.begin(57600); // set baud-rate for the default serial port
  btSerial.begin(57600); // set baud-rate for the bluetooth serial port

  pinMode(btPower, OUTPUT); // initialize the digital pin as an output.
  digitalWrite(btPower, HIGH); // Power ON the Bluetooth antenna
  
  started = true;
}

void loop_BluetoothBridge() {

  // Echo all incoming Bluetooth data (for PING tests)
  while(btSerial.available()) {
    in_byte = btSerial.read();
    
    // wait until the begining of a new message
    if(in_byte == STX) {
      in_msgID   = btSerial.read(); //read message ID
      in_seqNum  = btSerial.read(); //read sequence number
      in_dlc[0] = btSerial.read();  //read DCL bytes (4 bytes)
      in_dlc[1] = btSerial.read();
      in_dlc[2] = btSerial.read();
      in_dlc[3] = btSerial.read();
      
      in_payloadSize =   (in_dlc[0] & 0xFF) << 24 | (in_dlc[1] & 0xFF) << 16 | (in_dlc[2] & 0xFF) << 8 | (in_dlc[3] & 0xFF);  //convert DLC bytes to integer (bit shifting)
      
      char in_payload[in_payloadSize]; // char array to store the payload (note: array size = DLC)
      for(int i = 0; i< in_payloadSize; i++){
        in_payload[i] = btSerial.read(); //read while payload bytes remaining
      }
      in_payload[in_payloadSize]=0; // mandatory end of string at the end of the payload
      // String s = String(in_payload); // char[] to String
      
      // get latitude and longitude from the payload message:
      char *coordinates[4]; // auxiliar variable for strtok() (string tokenizer)
      coordinates[0] = strtok(in_payload, " "); // first part = latitude
      coordinates[1] = strtok(NULL, " "); // segond part = longitude
      coordinates[2] = strtok(NULL, " "); // third part = velocity
      coordinates[3] = strtok(NULL, " "); // fourth part = distance
      coordinates[4] = strtok(NULL, " "); // fourth part = locality
      latitude = String(coordinates[0]).toFloat(); // get latitude
      longitude = String(coordinates[1]).toFloat(); // get longitude
      velocity = String(coordinates[2]).toFloat(); // get velocity
      distance = String(coordinates[3]).toFloat(); // get distance
      locality = String(coordinates[4]); // get locality

      in_crc[0] = btSerial.read();  //read CRC bytes (4 bytes)
      in_crc[1] = btSerial.read();
      in_crc[2] = btSerial.read();
      in_crc[3] = btSerial.read();
      
      in_byte = btSerial.read(); // read the last byte (it must be the ETX byte)
      if(in_byte!=ETX) {
        Serial.println("Error reading the message!");
      } else {
        Serial.print("Coordinates: lat. ");Serial.print(latitude,7);Serial.print(" long. ");
        Serial.print(longitude,7);Serial.print(" || Vel.: ");Serial.print(velocity,8);Serial.print(" distance: ");Serial.print(distance,2);
        Serial.println("");
        // TODO: send asyncronous call or message to main thread with the new coordinates
      }
    }//if STX
  }//end while(btSerial.available())
  
  payload = context;
  if(payload != ""){
    sendMessageIfPossible();
    context = "";
  }
  
  //delay(50);
}//end loop()


//------------------------------------------------------------
//------------------------------------------------------------


// Check flags and send a new message if possible
void sendMessageIfPossible(){
   if(check_clock()) { 
    if(started) {
      ldr = analogRead(0);
      int length = payload.length(); 
      char payloadBytes[length+1]; // Length (with one extra character for the null terminator)
      payload.toCharArray(payloadBytes, length+1);
      
      sendMessage(MSGID_STRESS, payloadBytes, length);
    }
  } 
}

// Send a message to the Bluetooth serial port
void sendMessage(int MSGID, char out_payload[], int length) {
  unsigned char buf[4];
  buf[0] = (length >> 24) & 0xFF;
  buf[1] = (length >> 16) & 0xFF;
  buf[2] = (length >> 8) & 0xFF;
  buf[3] = (length) & 0xFF;
  
  long crc = crc_string(out_payload);
  unsigned char crcBytes[sizeof(long int)];
  memcpy(crcBytes,&crc,sizeof(long int));
    
  if(frameNum<99)
    frameNum+=1;
  else
    frameNum = 1;
  
  btSerial.write(STX);
  btSerial.write(MSGID);
  btSerial.write(frameNum);
  btSerial.write(buf, sizeof(buf));
  btSerial.write(out_payload);
  btSerial.write(crcBytes,sizeof(crcBytes));
  btSerial.write(ETX);
  
  Serial.print(length);
  Serial.print(": ");
  Serial.print(out_payload);
  Serial.println("");
}

// --------------------------------
//              TIMER
// --------------------------------

// This method updates a counter of seconds and shows information through the Serial interface
boolean check_clock() {
  if(flag_seconds){
    flag_seconds = false;
    return true;
  } 
  return false;
}

//Timer2 Overflow Interrupt Vector, called every 1ms
ISR(TIMER2_OVF_vect) {
  ticks++;               //Increments the interrupt counter
  if(ticks > CONTEXT_SEND_FREQENCY){
    ticks = 0;           //Resets the interrupt counter
    flag_seconds = true;
    seconds++;
  }
  TCNT2 = 130;           //Reset Timer to 130 out of 255
  TIFR2 = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
}; 


//---------------------------------------------
//      CRC (Checksum for error detection)
//---------------------------------------------

// constant CRC table
const uint32_t PROGMEM crc_table[16] = {
//static PROGMEM prog_uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

// Returns the CRC-32 checksum value for the given byte
unsigned long crc_update(unsigned long crc, byte data) {
    byte tbl_idx;
    tbl_idx = crc ^ (data >> (0 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    tbl_idx = crc ^ (data >> (1 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    return crc;
}

// Returns the CRC-32 checksum value of the given char array
unsigned long crc_string(char *s) {
  unsigned long crc = ~0L;
  while (*s)
    crc = crc_update(crc, *s++);
  crc = ~crc;
  return crc;
}


