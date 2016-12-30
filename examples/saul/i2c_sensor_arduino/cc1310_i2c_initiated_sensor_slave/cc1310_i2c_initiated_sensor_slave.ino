#include <Wire.h>

//STATES
//communication states
enum I2C_COMM_PROT_HEADER {GET_SENSACT_NUM, GET_SENSOR_DESC, SENSOR_READ, SENS_ACT_WRITE};
byte STATE = GET_SENSACT_NUM;

unsigned char I2C_SLAVE_ADDRESS = 0;

typedef struct tru2air_header_t {
  unsigned char action;
  unsigned char specifier;
} tru2air_header_t;

tru2air_header_t HEADER = {0xff,0xff};

//TEMPORARY DUMMY VARIABLES
byte SENS_NUM = 0x03;
typedef unsigned char sensor_name_t[24];
sensor_name_t sensors[] = {"1st_sensor", "2nd_sensor", "3rd_sensor"};
unsigned char device_addr[] = {0xde, 0xad, 0xbe, 0xef};


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
  //Serial.print("[I2C SLAVE] Started i2c slave on: ");
  //printHex4(&I2C_SLAVE_ADDRESS);
  //Serial.print("\n");
  Wire.onReceive(receiveCb);
  Wire.onRequest(requestCb);
}

void loop() {
  delay(500);
}

void receiveCb(int numBytes) {
  Serial.print("[DATA RECEIVED]\n");

  // reseting the specifier if there is any
  HEADER.specifier = 0xff;
  
  HEADER.action = Wire.read();
  
  if(numBytes == 2) {
    HEADER.specifier = Wire.read();
  }
  
  switch(HEADER.action) {
    case GET_SENSACT_NUM:
//      Serial.print("[STATE] -> GET_SENSACT_NUM \n");
      STATE = GET_SENSACT_NUM;
      break;
    case GET_SENSOR_DESC:
//      Serial.print("[STATE] -> GET_SENSOR_DESC \n");
      STATE = GET_SENSOR_DESC;
      break;
    default:
//      Serial.print("[ERROR] Invalid state receieved! -> ");
      break;
  }  

}

void requestCb() {
  Serial.print("[REQUEST]\n");
  bool endOfMsg = false;
  unsigned char msgChar = 0;

  switch(STATE){
    case GET_SENSACT_NUM:
      Wire.write(device_addr, 4);
      Wire.write(sizeof(sensors)/sizeof(sensor_name_t));
      break;
    case GET_SENSOR_DESC:
      //TODO: handle bad HEADER
  
      Wire.write((char*)(sensors[HEADER.specifier]));
      Wire.write('\0');
      
      /*
      while (!endOfMsg) {
        if (sensors[HEADER.specifier][msgChar] != '\0') {
          Wire.write((char)sensors[HEADER.specifier][msgChar]);
          msgChar++;
        }
        else {
          Wire.write('\0');
          endOfMsg = true;
        };
      };
      */
      break;
      
    default:
      break;
  }
}

void printHex4 (byte* data) {
    char tmp[4];
    sprintf(tmp, "0x%02x", *data);
    Serial.print(tmp);
}
