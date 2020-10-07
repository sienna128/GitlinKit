#include <Wire.h>

#define CARDKB_ADDR 0x5F

void setup()
{
  Serial.begin(9600);
  Serial.print("\n\n\nCardKB to Serial Demo\n\n");

  //UNO or Nano use Wire.begin(): defaults to SDA=A4 (white), SCL=A5 (yellow)
  Wire.begin(); 
  
  //ESP32 use Wire.begin(I2C_SDA, I2C_SCL): SDA=IO32 (white), SCL=IO33 (yellow)
  //Wire.begin(32,33);   

}

void loop()
{
  Wire.requestFrom(CARDKB_ADDR, 1);
  while (Wire.available())
  {
    char c = Wire.read(); 
    if (c != 0)
    {
      Serial.print(c);
    }
  }
}
