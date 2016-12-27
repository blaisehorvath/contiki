#include <Wire.h>

//STATES
//communication states
enum I2C_COMM_PROT_HEADER {GET_SENSACT_NUM, GET_SENSOR_DESC, SENSOR_READ, SENS_ACT_WRITE};

unsigned char I2C_SLAVE_ADDRESS = 0;
unsigned char ii = 0;
unsigned char message[] = {0xde, 0xad, 0xbe, 0xaf, 0x05};

//TEMPORARY DUMMY VARIABLES
byte SENS_NUM = 0x03;
typedef unsigned char sensor_name_t[24];
sensor_name_t sensors[] = {"1st_sensor", "2nd_sensor", "3rd_sensor"};


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
  
  //TODO: handle when no proper i2c id was received
  
  Wire.begin(I2C_SLAVE_ADDRESS);
  Serial.print("[I2C SLAVE INIT] Started i2c slave on: ");
  printHex4(&I2C_SLAVE_ADDRESS);
  Serial.print("\n");
  Wire.onReceive(receiveCb);
  Wire.onRequest(requestCb);
}

void loop() {
  delay(500);
}

void receiveCb(int fasz) {
  unsigned char data = Wire.read();
  Serial.print("[INCOMING DATA] ");
  Serial.print("Receieved: ");
  printHex4(&data);
  Serial.print("\n");
  
  switch(data) {
    case GET_SENSACT_NUM:
      Serial.print("[SWITCH TO STATE] -> GET_SENSACT_NUM \n");
      break;
    default:
      Serial.print("[ERROR] Invalid state receieved! \n");
  }
}

void requestCb() {
  Serial.print("[DATA REQUEST]");
  Wire.write(message, 5);
  Serial.print("\n");
}

void printHex4 (byte* data) {
    char tmp[4];
    sprintf(tmp, "0x%02x", *data);
    Serial.print(tmp);
}
