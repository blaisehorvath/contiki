#include <Wire.h>

void setup() {

 // pinMode(led, OUTPUT);
  Wire.begin();        // join i2c bus (address optional for master)

  Serial.begin(9600);  // start mySerial for output
  Serial.print("Init\n");

  Serial.print("Beefing the slave\n");
  byte sendData[4] = {0xde, 0xad, 0xbe, 0xef};
  Wire.beginTransmission(16);
  Wire.write(sendData,4);
  Wire.endTransmission(false);
  /*
   
   Serial.write("Requesting 4 bytes from slave\n"); 
    Wire.requestFrom(8, 4);
    while (Wire.available()) { // slave may send less than requested
      char c = Wire.read(); // receive a byte as character
      Serial.write("Received a byte: ");
      Serial.write(c);
      Serial.write("\n");
    }
    
   */
}

void loop() {
}
