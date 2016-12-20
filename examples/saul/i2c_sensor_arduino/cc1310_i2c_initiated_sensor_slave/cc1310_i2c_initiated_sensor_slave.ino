#include <Wire.h>

unsigned char I2C_SLAVE_ADDRESS = 0;

void setup() {
  Serial.begin(9600);  // start serial for output
  Wire.begin(); // join i2c bus (address optional for master)
  
  Wire.beginTransmission(0x10); // transmit to device #8
  Wire.write(0xde);      
  Wire.write(0xad);      
  Wire.write(0xbe);      
  Wire.write(0xef);     
  Wire.endTransmission(false);    // stop transmitting
 
  
  Wire.requestFrom(0x10, 1);    // request 6 bytes from slave device #8 
  
  while (Wire.available()) { // slave may send less than requested
    I2C_SLAVE_ADDRESS = Wire.read(); // receive a byte as characterl
  }
  
  Serial.print("received i2c ID: ");
  Serial.print(I2C_SLAVE_ADDRESS, HEX);
  Serial.print("\n");
}

byte x = 0;

void loop() {
  delay(500);
}
