#include <PS2Keyboard.h>

/*
Pin DATA     _NC_     GND    V++     CLK     _NC_    
Pin____1_______2_______3_______4_______5_______6 
_____yellow___none____brown___green_____red____none
_____yellow___none____green____blue____purple____none
_____black____none_____red____orange___yellow___none
_____black____none_____red____green____yellow___none
These are a few options I've seen. As you can see, 
there isn't really a color standard that manufacturers 
adhere to (unlike usb). 
LEDs light up with Red Vin Green GND
 */

#define DATA_PIN 4
#define IRQ_PIN 3
PS2Keyboard keyboard;

void setup() 
{
  keyboard.begin(DATA_PIN, IRQ_PIN);

  Serial.begin(9600);
  Serial.println("hi");
  delay(1000);
}

void loop() 
{
  if(keyboard.available()) 
  {
    byte dat = keyboard.read();
    byte val = dat - '0';

    if(val >= 0 && val <= 9) 
    {
      Serial.print(val, DEC);
    } 
    else if(dat == PS2_ENTER) 
    {
      Serial.println();
    } 
    else if(dat == PS2_ESC) 
    {
      Serial.println("[ESC]");
    } 
  }
}
