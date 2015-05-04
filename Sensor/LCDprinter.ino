//////////////////////////////////////////////////////////////////
// 2015 Sensors and Networks
// Project 1
// Felix Auer, Elena Zueva, Xabier Gardeazabal
//
// Simple code for printing in the LCD display
//////////////////////////////////////////////////////////////////

/* To move the cursor, send the special character 254 decimal (0xFE hex), followed by the cursor position you'd like to set. 
*  Each cursor position is represented by a number, see the table below to determine the number to send:
      position 	        1 	2 	3 	4 	5 	6 	7 	8 	9 	10 	11 	12 	13 	14 	15 	16
      line   1   	128 	129 	130 	131 	132 	133 	134 	135 	136 	137 	138 	139 	140 	141 	142 	143
      line   2   	192 	193 	194 	195 	196 	197 	198 	199 	200 	201 	202 	203 	204 	205 	206 	207
*/

// Use the softwareserial library to create a new "soft" serial port
// for the display. This prevents display corruption when uploading code.
#include <SoftwareSerial.h>

// Attach the serial display's RX line to digital pin 2
SoftwareSerial LCD(3,2); // pin 2 = TX, pin 3 = RX (unused)


void initializeLCD(){
  LCD.begin(9600); // set up serial port for 9600 baud

// LCD setup commands: uncomment the ones you want to use
// Note: These codes (i.e. the ones following 254) may have to be changed for 
// different manufacturer's displays

// Turn Auto scroll ON
//   LCD.write(254);
//   LCD.write(81);     
// 
// Turn Auto scroll OFF
//   LCD.write(254);
//   LCD.write(82); 

// Turn ON AUTO line wrap
//   LCD.write(254); 
//   LCD.write(67);              

// Turn OFF AUTO line wrap
//   LCD.write(254); 
//   LCD.write(68); 

// Turn OFF the block cursor    
// Note that setting both block and underline 
// cursors may give unpredictable results. 
//   LCD.write(254);
//   LCD.write(84);               

// Turn ON the block cursor
//   LCD.write(254);
//   LCD.write(83);  

// Turn ON the underline cursor
//   LCD.write(254);
//   LCD.write(74);               

// Turn OFF the underline cursor
//   LCD.write(254);
//   LCD.write(75);    
}

/*
void clearLCD(){
  LCD.write(254); // move cursor to beginning of first line
  LCD.write(128);

  LCD.write("                "); // clear display
  LCD.write("                ");

  LCD.write(254); // move cursor to beginning of first line
  LCD.write(128);
}
*/

/////////////////////////////////////////////////////////////////////
//                    LCD GENERAL FUNCTIONS                        //
/////////////////////////////////////////////////////////////////////


// clear the LCD
void clearLCD(){
  //LCD.write(12);
  LCD.write(0xFE);   //command flag
  LCD.write(0x01);   //clear command.
}

/*
// start a new line
void newLine() { 
  LCD.write(10); 
}
*/

void selectLineOne(){  //puts the cursor at line 0 char 0.
   LCD.write(0xFE);   //command flag
   LCD.write(128);    //position first line first cell
}
void selectLineTwo(){  //puts the cursor at line 1 char 0.
   LCD.write(0xFE);   //command flag
   LCD.write(192);    //position second line first cell
}

// move the cursor to the home position
void cursorHome(){
  LCD.write(254);
  LCD.write(72);
}

// move the cursor to a specific place (X= column, Y = Row)
// e.g.: cursorSet(3,2) sets the cursor to x = 3 and y = 2
void cursorSet(int xpos, int ypos){  
  LCD.write(254);
  LCD.write(71);               
  LCD.write(xpos);   //Column position   
  LCD.write(ypos); //Row position 
} 

// backspace and erase previous character
void backSpace() { 
  LCD.write(8); 
}

// move cursor left
void cursorLeft(){    
  LCD.write(254); 
  LCD.write(76);   
}

// move cursor right
void cursorRight(){
  LCD.write(254); 
  LCD.write(77);   
}

// set LCD contrast
void setContrast(int contrast){
  LCD.write(254); 
  LCD.write(80);   
  LCD.write(contrast);   
}

// turn on backlight
void backlightOn(int minutes){
  LCD.write(254); 
  LCD.write(66);   
  LCD.write(minutes); // use 0 minutes to turn the backlight on indefinitely   
}

// turn off backlight
void backlightOff(){
  LCD.write(254); 
  LCD.write(70);   
}


////////////////////////////////////////////////////////////////////
//                    CUSTOM PRINT FUNCTIONS                      //
////////////////////////////////////////////////////////////////////

void printXYZ(String what, int x, int y, int z){
  clearLCD();
  selectLineOne();
  LCD.print(what+": ");
  LCD.print("X: ");
  LCD.print(x);
  selectLineTwo();
  LCD.print("Y: ");
  LCD.print(y);
  LCD.print("  Z:");
  LCD.print(z);
}



void printAccelerometerValues(double x, double y, double z){
  clearLCD();
  
  LCD.print("Axis X: ");
  LCD.print((double) x);

  // create strings from the numbers, right-justified to 4 spaces
  /*sprintf(xString,"X: %4d",x); 
  sprintf(yString,"Y: %4d",y);
  sprintf(zString,"Z: %4d",z); 
  */
}

void printAccel(String what, double maxX, double maxY, double maxZ){
  clearLCD();
  selectLineOne();
  LCD.print(what+": ");
  LCD.print("X: ");
  printDouble(x, 2);
  selectLineTwo();
  LCD.print("Y: ");
  printDouble(y, 2);
  LCD.print("  Z:");
  printDouble(z, 2);
}

void printMinAccel(double minX, double minY, double minZ){

}


void printTemp(String what, double currentT, double minT, double maxT){
  clearLCD();
  selectLineOne();
  LCD.print(what+": ");
  printDouble(currentT, 1);
  selectLineTwo();
  LCD.print("Min ");
  printDouble(minT, 1);
  LCD.print("  Max ");
  printDouble(maxT, 1);
}


void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

    LCD.print (int(val));  //prints the int part
    LCD.print("."); // print the decimal point
    unsigned int frac;
    if(val >= 0)
        frac = (val - int(val)) * precision;
    else
        frac = (int(val)- val ) * precision;
    LCD.print(frac,DEC) ;
}


