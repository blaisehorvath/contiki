#include <Wire.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(9, 8); // RX, TX

int pressed = false;


int state = 0;
byte i2c_addr;

int Cs = P1_0;
int clockPin = P1_4;
int dataPin = P1_2;
int SOMI = P1_1;

void setup() {

  mySerial.begin(9600);  // start mySerial for output
  mySerial.print("Initasd\n");

  Wire.setModule(0);
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(reqEvent); // register event
  Wire.onReceive(recEvent);

  pinMode(Cs, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(SOMI, INPUT);

}

void loop() {
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void reqEvent() {
  if (state) Wire.write(i2c_addr);
  state = 0;
  i2c_addr = 0;
}

void recEvent(int numBytes) {
  mySerial.print("recevent\n");
  byte arr[4];
  int i;
  for (i = 0; i < 4; i++) {
    arr[i] = Wire.read();
  }
  i2c_addr = getAddr(*(uint32_t *)arr);
  state = 1;
}

byte getAddr(uint32_t devAddr) {
  mySerial.print("Getaddr\n");
  uint8_t* addr = (uint8_t * )&devAddr;
  // Sending the address
  for (int i = 0; i < 4; i++) {
    digitalWrite(Cs, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, *(addr + (i)));
    digitalWrite(Cs, HIGH);
    //delay(100);
  }


  //getting the i2c address
  digitalWrite(Cs, LOW);
  //Serial.print("i2c_addr in getaddr:");Serial.print(i2c_addr);Serial.print("\n");
  i2c_addr = shiftIn(SOMI, clockPin, LSBFIRST);
  digitalWrite(Cs, HIGH);

  return i2c_addr;
}
