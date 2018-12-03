/***************************************************************************
  Laser Receiver Code

  Hamming encoder/decoder and Optical modulator/demodulator are based on
  the LumenWire library written by Andrew Ramanjooloo and modified by
  Tom Gitlin 
  https://github.com/HobbyTransform/Encoded-Laser-and-LED-Serial-Communication

  Arduino sketch by Tom Gitlin

IMPORTANT: The Delay function is used sparingly because it stalls program 
execution (except for interrupts) and can affects the ability to check if
characters are ready.
***************************************************************************/

#include "LedControl.h"           // include the library to drive the LED Control
#include <HammingEncDec.h>        // include the Hamming encoder/decoder functionality
#include <OpticalModDemod.h>      // include the modulator/demodulator functionality
#include <MD_Parola.h>            // LED Maxtrix Display Library
#include <MD_MAX72xx.h>           // MD_MAX72XX library https://github.com/MajicDesigns/MD_MAX72XX
#include <SPI.h>              

OpticalReceiver phototransistor;  // create an instance of the receiver

/***************************************************************************
Define the number of devices we have in the LED Matrix display chain and the
hardware interface.  NOTE: These pin numbers will probably not work with your
hardware and may need to be adapted
****************************************************************************/
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10
#define SPEED_TIME  30
#define PAUSE_TIME  0

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

#define PACKET_SIZE 20      // Size of the number of characters for each measurand
byte c;                     // variable to hold the byte returned from the receiver
int i, n;                   // loop counters
int tempVal;                // temporary value holder for sanity check
unsigned long timelastchar; // variable to hold the time the last valid character was received
bool linkgood;              // flag to store optical link status
bool linkdropped;           // flag to determine initial acquisition after dropped link
int linktimeout=1000;       // if no valid characters received in this time period, assume the link is bad
unsigned long timenow;      // holds the Arduino running time in milliseconds for display times
unsigned long nolinkanimate=500; // cycle through the no link msg variations
unsigned long lastanimatetime;
String parameterValue;      // holds the measurand being built up character-by-character
char tempChar;
int LED_Temp = 6, LED_Press = 5, LED_Humid=4;  // Pins for the Temp, Press, and Humidity LEDs
char tlmTemp[PACKET_SIZE]{};
char tlmPress[PACKET_SIZE]{};
char tlmHumid[PACKET_SIZE]{};
uint8_t DegFSymbol[] = { 7, 7,5,7,0,124,20,4}; // Deg F character for LED display
uint8_t SqrWavSymbol[] = { 8, 1,1,127, 64, 64, 64, 127,1 }; // Square wave character 0 for LED display
uint8_t inchSymbol[] = {6,116,0,120,16,8,112}; 
uint8_t mercurySymbol[] = {7,126,8,120,0,152,164,248};
uint8_t relhumidSymbol[] = {8,0,124,20,104,0,124,16,124};

String Temp_Prefix = "Temp: ";
String Temp_Suffix = " ^";
String Press_Prefix = "Press: ";
String Press_Suffix = " `~";
String Humid_Prefix = "Humid: ";
String Humid_Suffix = " %_";
String NO_LINK[4] = {"-Link-","+Link+", "*Link*","+Link+"};
const char *intromsg = {"&&&&&  Laser Comm with Arduino  &&&&&"};

void setup(){

  //Serial.begin(9600);                 // start the serial port on the Arduino
  phototransistor.set_speed(2000);    // laser receive speed - should be 500+ bits/second, nominal 2000 (=2KHz)
  phototransistor.set_rxpin(7);       // pin the phototransistor is connected to
  phototransistor.set_inverted(true); // if receive signal is inverted (Laser on = logic 0) set this to true
  phototransistor.begin();            // initialize the receiver

  // define the LED Matrix display parameters
  P.setInvert(false);
  P.begin();
  P.addChar('^', DegFSymbol);               // redefine a few characters to print custom chars       
  P.addChar('&', SqrWavSymbol);
  P.addChar('`', inchSymbol);
  P.addChar('~', mercurySymbol);
  P.addChar('_', relhumidSymbol);

  if (P.displayAnimate()){
    P.displayText(intromsg, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
}

// Set up an interrupt service routine to receive characters
// Arduino Timer2 reads the LIGHT_RECEIVE_PIN at laser receive speed to receive each half bit

ISR(TIMER2_COMPA_vect){
  phototransistor.receive();   // check for a received character every timer interrupt
}

void loop(){
  
  c = phototransistor.GetByte();     // get a character from the laser receiver if one is available
  if (c>45 && c<90){                 // if a character is ready, and is a printable ASCII character, look at it
    timelastchar=millis();
    if (linkdropped){linkdropped=false; P.print("");}
    switch (c){       // if the character is a terminator, store what was built in a variable and display it
      case 84:         // ASCII T termination character for temperature found, use string built to this point for temp
        tempVal=parameterValue.toInt();
        if ((tempVal>10) && (tempVal<110)){
          //parameterValue=Temp_Prefix+parameterValue+Temp_Suffix;
          parameterValue=parameterValue+Temp_Suffix;
          strcpy(tlmTemp, parameterValue.c_str());
          //Serial.println(tlmTemp);
        }
        parameterValue="";    
        break;
      case 80:        // ASCII P termination character for pressure found, use string built to this point for pressure
        tempVal=parameterValue.toInt();
        if ((tempVal>10) && (tempVal<40)){
          //parameterValue=Press_Prefix+parameterValue+Press_Suffix;
          parameterValue=parameterValue+Press_Suffix;
          strcpy(tlmPress, parameterValue.c_str());
        }
        parameterValue="";    
        break;    
      case 72:        // ASCII H termination character for humidity found, use string built to this point for humidity
        tempVal=parameterValue.toInt();
        if ((tempVal>1) && (tempVal<100)){
          //parameterValue=Humid_Prefix+parameterValue+Humid_Suffix;
          parameterValue=parameterValue+Humid_Suffix;
          strcpy(tlmHumid, parameterValue.c_str());
        }
        parameterValue="";    
        break;
      default :
        //Serial.println(parameterValue);
        parameterValue=parameterValue+=(char)c;  // keep building a string character-by-character until a terminator is found
    }

  }

  linkgood = !(millis()>(timelastchar+linktimeout));  // update the link status based on the timeout value

  if (linkgood){
    // cycle through displaying the values if the link is good
      if (i>2){i=0;}
      switch (i){
        case 0:
        if( P.displayAnimate()){ // animates and returns true when an animation is completed
          P.displayText(tlmTemp, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
          i++;
        }
        break;
        case 1:
        if (P.displayAnimate()){
          P.displayText(tlmPress, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
          i++;
        }
        break;
        case 2:
        if (P.displayAnimate()){
          P.displayText(tlmHumid, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
          i++;
        }   
      }
    } else {

        linkdropped=true;
        parameterValue="";
        P.print(NO_LINK[n]);
        timenow=millis();
        if (timenow>(lastanimatetime+nolinkanimate)){  // if link dropped, cycle through the animation message
          lastanimatetime=millis();
          n++;
          n=n%4;
        }

    }
}

