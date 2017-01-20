#include <Wire.h>

//STATES
//communication states
enum I2C_COMM_PROT_ACTION {GET_SENSACT_NUM, GET_SENSOR_NAME, GET_SENSOR_TYPE, SENS_ACT_READ, SENS_ACT_WRITE};


// Sensor return types
enum TRU2AIR_SENSOR_DATA_TYPE {SENS_DOUBLE, SENS_UINT32 };



byte STATE = GET_SENSACT_NUM;

unsigned char I2C_SLAVE_ADDRESS = 0;

typedef struct tru2air_header_t {
  unsigned char action;
  unsigned char specifier;
} tru2air_header_t;

tru2air_header_t HEADER = {0xff,0xff};

typedef struct sensact_descriptor_t {
  unsigned char name [24];
  unsigned char type;
} sensact_descriptor_t;


//TEMPORARY DUMMY VARIABLES
byte SENS_NUM = 0x03;
double double_answer = 1;
sensact_descriptor_t sensors[] = {{"1_st_sensor", SENS_DOUBLE}, {"2_nd_sensor", SENS_DOUBLE}, {"3_rd_sensor", SENS_UINT32}};
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
  Wire.requestFrom(0x10, 1,false);    // request 6 bytes from slave device #8 

  

  
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
//  Serial.print("[DATA RECEIVED]\n");

  // reseting the specifier if there is any
  HEADER.specifier = 0xff;
  
  HEADER.action = Wire.read();
  
  if(numBytes >= 2) {
    HEADER.specifier = Wire.read();
  }
  
//  Serial.print(HEADER.action);
//  Serial.print("\n");
  
  switch(HEADER.action) {
    case GET_SENSACT_NUM:
//      Serial.print("[STATE] -> GET_SENSACT_NUM \n");
      STATE = GET_SENSACT_NUM;
      break;
    case GET_SENSOR_NAME:
//      Serial.print("[STATE] -> GET_SENSOR_NAME \n");
      STATE = GET_SENSOR_NAME;
      break;
    case GET_SENSOR_TYPE:
//      Serial.print("[STATE] -> GET_SENSOR_TYPE");
      STATE = GET_SENSOR_TYPE;
      break;
    
    case SENS_ACT_WRITE:
      Wire.write((char*)&double_answer, sizeof(double));
    
      break;
    default:
//      Serial.print("[ERROR] Invalid state receieved! -> ");
      break;
  }  

}

void requestCb() {
//  Serial.print("[REQUEST]\n");
  bool endOfMsg = false;
  double dummyMsg = 1;

  //TODO:remove this
  double ans = 0;

  switch(STATE) {
    case GET_SENSACT_NUM:
      Wire.write(device_addr, 4);
      Wire.write(sizeof(sensors)/sizeof(sensact_descriptor_t));
      break;
    case GET_SENSOR_NAME:
      //TODO: handle bad HEADER
  
      Wire.write((char*)(sensors[HEADER.specifier].name));
      Wire.write('\0');
      
      break;
      
    case GET_SENSOR_TYPE:
      Serial.print(sizeof(sensors[HEADER.specifier].type));
      Wire.write(sensors[HEADER.specifier].type);
      break;

    case SENS_ACT_READ:
      Wire.write((char*)&dummyMsg, sizeof(double));
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
