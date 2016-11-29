#include <Wire.h>
#include <SPI.h>

const int blue = PF_2;
const int green = PF_3;
const int red = PF_1;

const int slaveSelectPin = PA_7;
int pressed = false;


int state = 0;
byte i2c_addr;

void setup() {
  // i2c
  //Serial.begin(9600);
  //Serial.print("tivac spi-i2c\n");
    Wire.setModule(0);
  Wire.begin(8);                // join i2c bus with address #8

  Wire.onRequest(reqEvent); // register event
  Wire.onReceive(recEvent);

  // leds
  pinMode(green,OUTPUT);
  pinMode(blue,OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(slaveSelectPin, OUTPUT);

  //spi
  digitalWrite(slaveSelectPin,HIGH);
  SPI.setModule(0);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  SPI.begin(); 
}

void loop() {
  delay(100);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void reqEvent() {
  Serial.print(state);Serial.print("i2caddr:");Serial.print(i2c_addr);Serial.print("\n");
  //Wire.write("hello "); // respond with message of 6 bytes
  if(state) Wire.write(i2c_addr);
  state= 0;
  i2c_addr= 0;
  
    if(i2c_addr != 0 && i2c_addr != 1 && i2c_addr !=0xFF) {
      digitalWrite(green,HIGH);
      int i=0;
      delay(50);
      digitalWrite(green,LOW);
    }
  // as expected by master
}

void recEvent(int numBytes) {
  Serial.print("recevent\n");
  Serial.print(numBytes);
  byte arr[4];
  int i;
  for (i=0; i<4; i++){
   arr[i] = Wire.read();
   Serial.print(arr[i]);
  }
  i2c_addr = getAddr(*(uint32_t *)arr);
  state = 1;
}


void toggleLed(int led){
  digitalWrite(led,HIGH);
  delay(50);  
  digitalWrite(led,LOW);
}

byte getAddr(uint32_t devAddr) {
  uint8_t* addr = (uint8_t * )&devAddr;
  // Sending the address
  for (int i=0; i<4; i++) {
      digitalWrite(slaveSelectPin,LOW);
      int res = SPI.transfer(*(addr+(i)));
      digitalWrite(slaveSelectPin,HIGH);
      delay(100);
    }


  //getting the i2c address
    digitalWrite(slaveSelectPin,LOW);
    Serial.print("i2c_addr in getaddr:");Serial.print(i2c_addr);Serial.print("\n");
    i2c_addr = SPI.transfer(1);
    digitalWrite(slaveSelectPin,HIGH);

    return i2c_addr;
}
