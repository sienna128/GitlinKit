/***************************************************************************
  Laser Relay Code: Receive signal from a Gitlin laser transmitter and re-
  transmit it to a Gitlin laser receiver / decoder.

  Hamming encoder/decoder and Optical modulator/demodulator are based on
  the LumenWire library written by Andrew Ramanjooloo and modified by
  Tom Gitlin 
  https://github.com/HobbyTransform/Encoded-Laser-and-LED-Serial-Communication

  Arduino sketch by Tom Gitlin and J. Acevedo
  version 0.0 09 Aug 2018 -- First stab.

***************************************************************************/
#include <HammingEncDec.h>        // include the Hamming encoder/decoder functionality
#include <OpticalModDemod.h>      // include the modulator/demodulator functionality

const int      linktimeout = 1000;    // if no valid characters received in this time period, assume the link is bad
const int      laserreceivespeed = 2000; //MUST MATCH THE SPEED AS DEFINED IN THE OPTICALTRANSMIT CODE

char           c;                     // variable to hold the byte returned from the receiver
int            i;                     // loop counter
int            measurand;             // tracks the number of measurands (3 for the demo, Temp, Press and Humidity)
unsigned long  timelastchar;          // variable to hold the time the last valid character was received
bool           linkgood;              // flag to store optical link status
bool           timetoswitch;          // flag to cycle through the measurands
bool           blankedvalues;         // flag to determine if values have been blanked for a bad laser link
unsigned long  timenow;               // holds the Arduino running time in milliseconds for display times
int            LED_transmit_timer_ticker = 0;      //Since we're avoiding using delay(), this will make sure the Xmit LED stays on long enough to be human-noticeable.
                                      
/* IMPORTANT: The Delay function is used sparingly because it stalls program execution
(except for interrupts) and can affects the ability to check if characters are ready. */
                                         
String         parameterValue;      // holds the measurand being built up character-by-character
String         strTemperature, strPressure, strHumidity; // holds the values of the measurands
char           tempChar;
const int      LASERRATE      = 2000;
const int      CHAR_DELAY     = 30;// delay between individual characters of a message (nominally 30ms)
const int      LED_RECEIVE    = 6; // Pin value for status lights.
const int      LED_XMIT       = 5; // Pin value for status lights.
const int      LASERPIN       = 13;// Pin to drive laser output
const int      PHOTOT_RECEIVE = 7; // Pin value for the phototransistor input.

char incomingByte;                // variable to hold the byte to be encoded
uint16_t msg;                     // variable to hold the message (character)

OpticalReceiver phototransistor;  // create an instance of the receiver
OpticalTransmitter laser;         // create an instance of the transmitter

void setup()
{
  Serial.begin(9600);                 // start the serial port on the Arduino
  
  // *** FOR THE MODULES THAT JIMMY BOUGHT, THIS NEEDS TO BE FLIPPED W/R/T TOM'S RECEIVERS ***
  phototransistor.set_speed(laserreceivespeed);        // laser receive speed - should be 500+ bits/second, nominal 2000 (=2KHz)
  phototransistor.set_rxpin(PHOTOT_RECEIVE);           // pin the phototransistor is connected to
  phototransistor.set_txpin(LASERPIN);      // pin the laser is connected to
  phototransistor.set_inverted(false);                 // if receive signal is inverted (Laser on = logic 0) set this to true
  phototransistor.begin();                             // initialize the receiver
  
  laser.set_speed(LASERRATE);     // laser modulation speed - should be 500+ bits/second, nominal 2000 (=2KHz)
  laser.set_txpin(LASERPIN);      // pin the laser is connected to
  laser.begin();                  // initialize the laser

  // Set up the LED pins to receive output from the Arduino to display status.
  pinMode(LED_RECEIVE,  OUTPUT);
  pinMode(LED_XMIT,     OUTPUT);
} //END of setup()

// Set up an interrupt service routine to receive characters
// Arduino Timer2 reads the LIGHT_RECEIVE_PIN at laser receive speed to receive each half bit
ISR(TIMER2_COMPA_vect)
{
  phototransistor.dummy_echo();
  //phototransistor.receive();
  //laser.transmit();
}

void loop()
{
  c = phototransistor.GetByte();     // get a character from the laser receiver if one is available
  if (c > 0)
  {
   // if a character is ready, pass it down the line
    //laser.manchester_modulate(c);       // modulate the character using the laser
    laserTransmit(String(c)); //Just trying to throw this down the proverbial hall
    //delay(CHAR_DELAY);                // wait delay between transmitting individual characters of the message
    
    Serial.println("I just received and passed along " + String(c));
    blankedvalues=false;
    timelastchar=millis();
    switch (c)
    {       
      // if the character is a terminator, store what was built in a variable and display it
      case 84:         // ASCII T termination character for temperature, use string built to this point for temp
        strTemperature = parameterValue;
        parameterValue = "";
        //Serial.println(strTemperature);
        laserTransmit(strTemperature);
        break;
      case 80:        // ASCII P termination character for pressure, use string built to this point for pressure
        strPressure = parameterValue;
        parameterValue = "";
        //Serial.println(strPressure);
        laserTransmit(strPressure);
        break;    
      case 72:        // ASCII H termination character for humidity, use string built to this point for humidity
        strHumidity = parameterValue;
        parameterValue = "";
        //Serial.println(strHumidity);
        laserTransmit(strHumidity);
        break;
      default :
        //Serial.println(parameterValue);
        parameterValue=parameterValue+=(char)c;  // keep building a string character-by-character until a terminator is found
    }
  }
  linkgood = !(millis() > (timelastchar + linktimeout));  // update the link status based on the timeout value

  digitalWrite(LED_RECEIVE, LOW);
  if (linkgood)
  {
    // Power RECEIVE LED if the link is good.
    digitalWrite(LED_RECEIVE, HIGH);
  }
  else
  {
  // link is bad, so display a distinctive pattern,
   if (!blankedvalues)
   {
     blankedvalues=true;
     strTemperature = " ";
     strPressure    = " ";
     strHumidity    = " ";
    }
  } //end IFCHECK-ELSE (link is bad)
} // end main loop()

  void  laserTransmit(String xmitmsg){
   for (i=0; i<(xmitmsg.length()+1); i++){  // transmit the string byte by byte
      incomingByte=xmitmsg.charAt(i);       // get the character at position i
      //Serial.print(incomingByte);
      msg = hamming_byte_encoder(incomingByte); // encode the character
      laser.manchester_modulate(msg);       // modulate the character using the laser
      delay(CHAR_DELAY); // wait delay between transmitting individual characters of the message

    }
  }
