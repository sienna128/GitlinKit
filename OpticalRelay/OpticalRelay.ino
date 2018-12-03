/***************************************************************************
  Laser Relay Code: Receive signal from a Gitlin laser transmitter and re-
  transmit it to a Gitlin laser receiver / decoder.

  Hamming encoder/decoder and Optical modulator/demodulator are based on
  the LumenWire library written by Andrew Ramanjooloo and modified by
  Tom Gitlin 
  https://github.com/HobbyTransform/Encoded-Laser-and-LED-Serial-Communication

  Arduino sketch by Tom Gitlin and J. Acevedo
  version 0.6 03 Dec 2018 -- Getting ready for v1.0 release.

***************************************************************************/
#include <HammingEncDec.h>        // include the Hamming encoder/decoder functionality
#include <OpticalModDemod.h>      // include the modulator/demodulator functionality

const int      linktimeout       = 1000;  // if no valid characters received in this time period, assume the link is bad
const int      laserreceivespeed = 2000;  //MUST MATCH THE SPEED AS DEFINED IN THE OPTICALTRANSMIT CODE

unsigned long  timelastchar;          // variable to hold the time the last valid character was received
bool           linkgood;              // flag to store optical link status
bool           bit_bucket;            // temporary bucket to hold the inbound bit
                                      
/* IMPORTANT: The Delay function is used sparingly because it stalls program execution
(except for interrupts) and can affects the ability to check if characters are ready. */
                                         
const int      LASERRATE      = 2000;
const int      CHAR_DELAY     = 30;// delay between individual characters of a message (nominally 30ms)
const int      LED_LINKGOOD   = 6; // Pin value for status lights.
const int      LED_LINKBAD    = 5; // Pin value for status lights.
const int      LASERPIN       = 13;// Pin to drive laser output
const int      PHOTOT_RECEIVE = 7; // Pin value for the phototransistor input.

OpticalReceiver phototransistor;  // create an instance of the receiver
OpticalTransmitter laser;         // create an instance of the transmitter

void setup()
{
  Serial.begin(9600);                 // start the serial port on the Arduino
  
  phototransistor.set_speed(laserreceivespeed);        // laser receive speed - should be 500+ bits/second, nominal 2000 (=2KHz)
  phototransistor.set_rxpin(PHOTOT_RECEIVE);           // pin the phototransistor is connected to
  phototransistor.set_txpin(LASERPIN);      // pin the laser is connected to
  phototransistor.set_inverted(true);                
  phototransistor.begin();                             // initialize the receiver
  
  laser.set_speed(LASERRATE);     // laser modulation speed - should be 500+ bits/second, nominal 2000 (=2KHz)
  laser.set_txpin(LASERPIN);      // pin the laser is connected to
  laser.begin();                  // initialize the laser

  // Set up the LED pins to receive output from the Arduino to display status.
  pinMode(LED_LINKGOOD,  OUTPUT);
  pinMode(LED_LINKBAD,     OUTPUT); 
} //END of setup()

// Set up an interrupt service routine to receive characters.
// Arduino Timer2 reads the LIGHT_RECEIVE_PIN at laser receive speed to receive each half bit.
// *** DO NOT PUT ANY DEBUG MESSAGES IN THIS FUNCTION; IT IS FINELY-TUNED FOR TIMING.***
ISR(TIMER2_COMPA_vect)
{
  bit_bucket = PIND & (1 << PHOTOT_RECEIVE); //Grab the bit via direct pin access.
  if (bit_bucket)
  {
    digitalWrite(LASERPIN, LOW); //Re-inverting this to be re-re-inverted at the "ground station."
  }
  else
  {
    digitalWrite(LASERPIN, HIGH); //Re-inverting this to be re-re-inverted at the "ground station."
  }
}

void loop()
{
  linkgood = !(millis() > (timelastchar + linktimeout));  // update the link status based on the timeout value
/*  digitalWrite(LED_LINKGOOD, LOW);
  digitalWrite(LED_LINKBAD,   LOW);
  if (linkgood)
  {
   // Power GOOD LED if the link is good.
    digitalWrite(LED_LINKGOOD, HIGH);
  }
  else
  {
    // Power BAD LED if the link is bad.
    digitalWrite(LED_LINKBAD,     HIGH);    
  }*/
} // end main loop()
