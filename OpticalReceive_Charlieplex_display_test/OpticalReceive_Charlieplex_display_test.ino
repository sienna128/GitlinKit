#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>

// SDA - YEL
// SCL - BLU
// PWR (3.3V) RED
// GND BLK

//Adafruit_IS31FL3731 ledmatrix = Adafruit_IS31FL3731();
Adafruit_IS31FL3731_Wing ledmatrix = Adafruit_IS31FL3731_Wing();

void setup() 
{
  Serial.begin(9600);
  Serial.println("Text display test");

  if (! ledmatrix.begin()) 
  {
    Serial.println("IS31 not found");
    while (1);
  }
  Serial.println("IS31 found!");
}

void loop() 
{
  ledmatrix.setTextSize(1);
  ledmatrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  for (int8_t x=0; x>=-96; x--) 
  {
    ledmatrix.clear();
    ledmatrix.setCursor(x,0);
    //ledmatrix.print("**F %");
    //ledmatrix.print(" 91.23F");
    ledmatrix.print(" HI MARIA <3");
    delay(100);
  }

  /*ledmatrix.setTextSize(2);
  ledmatrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  for (int8_t x=7; x>=-64; x--) 
  {
    ledmatrix.clear();
    ledmatrix.setCursor(x,0);
    ledmatrix.print("World");
    delay(100);
  }*/
}
