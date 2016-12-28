#include <Wire.h>

//STATES
//communication states
enum I2C_COMM_PROT_HEADER {GET_SENSACT_NUM, GET_SENSOR_DESC, SENSOR_READ, SENS_ACT_WRITE};

unsigned char I2C_SLAVE_ADDRESS = 0;
unsigned char ii = 0;
unsigned char message[] = {0xde, 0xad, 0xbe, 0xaf, 0x05};
typedef struct tru2air_header_t {
  unsigned char action;
  unsigned char specifier;
} tru2air_header_t;

tru2air_header_t HEADER = {0xff,0xff};

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
  Serial.print("[I2C SLAVE] Started i2c slave on: ");
  printHex4(&I2C_SLAVE_ADDRESS);
  Serial.print("\n");
  Wire.onReceive(receiveCb);
  Wire.onRequest(requestCb);
}

void loop() {
  delay(500);
}

void receiveCb(int numBytes) {
  Serial.print("[DATA RECEIVED]");
  
  HEADER.action = Wire.read();
  
  if(numBytes == 2) {
    HEADER.specifier = Wire.read();

  }
  
  switch(HEADER.action) {
    case GET_SENSACT_NUM:
      Serial.print("[SWITCH TO STATE] -> GET_SENSACT_NUM \n");
      break;
    default:
      Serial.print("[ERROR] Invalid state receieved! \n");
  }
  
  HEADER.action = 0xff;
  HEADER.specifier = 0xff;
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

void printHEADER() {
  Serial.print("ACTION is: ");
  printHex4(&(HEADER.action));
  Serial.print(" SPECIFIER is: ");
  printHex4(&(HEADER.specifier));
  Serial.print("\n");
}
