#include <Wire.h>

const int led = 13;

void setup() {
  Serial.print("Init\n");
  pinMode(led, OUTPUT);     
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  toggleLed(led);

/*
  Wire.requestFrom(8, 6);    // request 6 bytes from slave device #8

  while (Wire.available()) { // slave may send less than requested
    char c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
  }
*/
  byte sendData[4] = {0xde, 0xad, 0xbe, 0xef};
  Wire.beginTransmission(8);
  Wire.write(sendData, 4);
  Wire.endTransmission(false);
  
  Wire.requestFrom(8, 1);
  while (Wire.available()) { // slave may send less than requested
    char c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
  }

}

void loop() {
  delay(500);
}

void toggleLed(int led){
  digitalWrite(led,HIGH);
  delay(50);  
  digitalWrite(led,LOW);
}
