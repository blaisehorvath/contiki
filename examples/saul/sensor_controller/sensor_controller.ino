#include <Wire.h>
//#include <SoftwareSerial.h>

//SoftwareSerial mySerial(9, 8); // RX, TX
#define led P1_0

void setup() {
  
  pinMode(led, OUTPUT);   
    Wire.setModule(0);
  Wire.begin();        // join i2c bus (address optional for master)

  //mySerial.begin(9600);  // start mySerial for output
//mySerial.print("Init\n");

  byte sendData[4] = {0xde, 0xad, 0xbe, 0xef};
  Wire.beginTransmission(8);
  Wire.write(sendData, 4);
  Wire.endTransmission(false);
  
  Wire.requestFrom(8, 1);
  while (Wire.available()) { // slave may send less than requested
    char c = Wire.read(); // receive a byte as character
    //mySerial.print(c);         // print the character
  }

}

void loop() {
}
