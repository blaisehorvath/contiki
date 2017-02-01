#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

/****************************/
//         Tru2Air Stuff
/****************************/
#include <Wire.h>

//STATES
//communication states
enum I2C_COMM_PROT_ACTION {GET_SENSACT_NUM, GET_SENSOR_NAME, GET_SENSOR_TYPE, SENS_ACT_READ, SENS_ACT_WRITE};

//SENSACT MESUREMENT DATA SIZE
const unsigned char SENSACT_DATA_SIZE = 32;
unsigned char SENSACT_MEASURE_RESULT[SENSACT_DATA_SIZE];

// Sensor return types
enum TRU2AIR_SENSACT_TYPE {
	SENSACT_TRU2AIR_LED,
	SENSACT_TRU2AIR_RELAY,
	SENSACT_BME280_TEMP,
	SENSACT_BME280_PRESSURE,
	SENSACT_BME280_HUMIDITY,
	SENS_MAX_RANGE=65535
};

// DEVICE STATE
byte STATE = GET_SENSACT_NUM;

unsigned char I2C_SLAVE_ADDRESS = 0;

typedef struct tru2air_header_t {
  unsigned char action;
  unsigned char specifier;
} tru2air_header_t;

tru2air_header_t HEADER = {0xff,0xff};

typedef struct sensact_descriptor_t {
  unsigned char name [24];
  uint16_t type;
} sensact_descriptor_t;


//TEMPORARY DUMMY VARIABLES
byte SENS_NUM = 0x03;
unsigned int uint32_answer = 15;
sensact_descriptor_t sensors[] = {{"BME280_PRESSURE", SENSACT_BME280_PRESSURE}, {"BME280_TEMP", SENSACT_BME280_TEMP}, {"BME280_HUM", SENSACT_BME280_HUMIDITY}};
unsigned char device_addr[] = {0xde, 0xad, 0xbe, 0xef};


void setup() {
  Serial.begin(9600);  // start serial for output
  
  Wire.begin(); // join i2c bus (address optional for master)
  
  //initiating the BME driver

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
  // Starting the tru2air i2c protocol
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
      Wire.write(0x17);//uint32_answer);
    
      break;
    case SENS_ACT_READ:
      STATE = SENS_ACT_READ;
    default:
//      Serial.print("[ERROR] Invalid state receieved! -> ");
      break;
  }  

}

void requestCb() {
//  Serial.print("[REQUEST]\n");
  bool endOfMsg = false;

  //TODO:remove this

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
      //Serial.print(sizeof(sensors[HEADER.specifier].type));
      Wire.write((char*)(&sensors[HEADER.specifier].type),2);
      break;

    case SENS_ACT_READ:
      
      float measurement;
      char BEresultArr[4];
      memset(SENSACT_MEASURE_RESULT, 0x00, SENSACT_DATA_SIZE);
      
      switch(HEADER.specifier) {
        //pressure
        case 0:
          measurement = bme.readPressure();
          convertLEFloatToBE(&measurement, BEresultArr);
          
          memcpy(SENSACT_MEASURE_RESULT, BEresultArr, sizeof(float));
          Wire.write((char*)&SENSACT_MEASURE_RESULT, SENSACT_DATA_SIZE);
          break;
        //temp
        case 1:
          measurement = bme.readTemperature();
          convertLEFloatToBE(&measurement, BEresultArr);
          
          memcpy(SENSACT_MEASURE_RESULT, BEresultArr, sizeof(float));
          Wire.write((char*)&SENSACT_MEASURE_RESULT, SENSACT_DATA_SIZE);
          break;
        //hum
        case 2:
          measurement = bme.readHumidity();
          convertLEFloatToBE(&measurement, BEresultArr);
          
          memcpy(SENSACT_MEASURE_RESULT, BEresultArr, sizeof(float));
          Wire.write((char*)&SENSACT_MEASURE_RESULT, SENSACT_DATA_SIZE);
          break;
          
        default:
          ;
      }
      
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

void convertLEFloatToBE (float* fl, char* BEFloatArr) {
     char i = 0;   
     while(i<4) {
       BEFloatArr[3-i] = *(((char*)fl)+i);
       i++;
     }
}



