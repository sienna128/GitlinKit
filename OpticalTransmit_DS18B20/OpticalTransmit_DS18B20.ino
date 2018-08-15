/***************************************************************************
  Laser Transmitter Code
  
  This is a program using a Dallas DS18B20 temperature sensor with the OneWire
  library to communicate to an Arduino.
  
  The sensor is connected to VCC -> 5V, GND -> GND, DATA -> Pin7, with DATA
  pulled up to the VCC line with a ~4.4Kohm resistor.

  A laser is also connected to the Arduino to send data to a receiver.

  Hamming encoder/decoder and Optical modulator/demodulator are based on
  the LumenWire library written by Andrew Ramanjooloo and modified by
  Tom Gitlin.
  https://github.com/HobbyTransform/Encoded-Laser-and-LED-Serial-Communication

  Arduino sketch by Tom Gitlin with some added features hacked on by Jimmy Acevedo.
  
***************************************************************************/

#include <DallasTemperature.h>
#include <HammingEncDec.h>    // include the Hamming encoder/decoder functionality
#include <OpticalModDemod.h>  // include the modulator/demodulator functionality

#define THERMOMETER_PIN     7
#define LED_SENSORERROR_PIN 5

OpticalTransmitter laser;     // create an instance of the transmitter
OneWire            oneWire_thermometer(THERMOMETER_PIN); // create an instance of the OneWire listener monitoring the THERMOMETER_PIN.
DallasTemperature  thermometer(&oneWire_thermometer);    // create an instance of the DallasTemperature reader service on the OneWire listener

// delay between transmitting measurands in milliseconds (nominally 500ms)
// need a 'long' value due to it being compared to the Arduino millis value
unsigned long delaytime = 500;  
int     CHAR_DELAY = 30;         // delay between individual characters of a message (nominally 30ms)
float   temperature;
boolean english = false;         // flag to use english units, otherwise use SI
String  strTemperature;
char    incomingByte;            // variable to hold the byte to be encoded
uint16_t msg;                    // varible to hold the message (character)
int i;                        

void setup() 
{
  pinMode(LED_SENSORERROR_PIN, OUTPUT);
  Serial.begin(9600);

  laser.set_speed(2000);      // laser modulation speed - should be 500+ bits/second, nominal 2000 (=2KHz)
  laser.set_txpin(13);        // pin the laser is connected to
  laser.begin();              // initialize the laser
}

// Set up an interrupt service routine to transmit characters
// Arduino Timer2 interrupt toggles the LIGHT_SEND_PIN at the laser send speed to transmit each half bit
ISR(TIMER2_COMPA_vect)
{
  laser.transmit(); // transmit a character if one is ready
}

void loop() 
{
    thermometer.requestTemperatures();    // Request temperatures from all connected DS18B20s.
    temperature =  thermometer.getTempCByIndex(0); // Query the first thermometer that's connected.
    
    // if english units are desired, perform the conversion
    if (english) 
    {
      temperature = temperature * 1.8 + 32.0; // convert C to F
    }
    strTemperature = String(temperature) += "T";
    laserTransmit(strTemperature);
    delay(delaytime);
}

void laserTransmit(String xmitmsg)
{
   for (i=0; i<(xmitmsg.length()+1); i++)   // transmit the string byte by byte
   {
      incomingByte=xmitmsg.charAt(i);       // get the character at position i
      //Serial.print(incomingByte);
      msg = hamming_byte_encoder(incomingByte); // encode the character
      laser.manchester_modulate(msg);       // modulate the character using the laser
      delay(CHAR_DELAY);                    // wait delay between transmitting individual characters of the message
    }
}

