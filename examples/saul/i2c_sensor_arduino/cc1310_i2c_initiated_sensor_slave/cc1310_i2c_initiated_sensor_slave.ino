#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define TRU2AIR_MAIN_NODE_SLAVE_ADDR 0x25
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

/****************************/
//         Tru2Air Stuff
/****************************/
#include <Wire.h>

enum TRU2AIR_I2C_HEADER_ACTION {
    GET_SENSACT_NUM,
    GET_SENSOR_NAME,
    GET_SENSOR_TYPE,
    SENS_ACT_READ,
    SENS_ACT_WRITE,
    CHECK_ALIVE
};

enum TRU2AIR_CLIENT_NODE_I2C_HANDLER_STATE {
    I2C_SLAVE_LISTEN,
    NODE_I2C_MASTER_INIT,
    REQUIRE_SENSACT_NAME,
    REQUIRE_SENSOR_RETURN_TYPE,
    REGISTER_TO_SAM,
    I2C_ERROR
};

enum TRU2AIR_I2C_ERROR {
    NO_I2C_ERROR,
    I2C_SENSACT_NUM_NULL_ERROR,
    I2C_SENSACT_NUM_ERROR
};

typedef struct tru2air_sensor_node_t {
    unsigned int dev_addr;
    unsigned char i2c_addr;
    unsigned char sensact_num;
} tru2air_sensor_node_t;


typedef struct i2c_pkt_t {
    uint32_t dev_id;
    uint8_t action;
    uint8_t error;
    unsigned char data[32];
    uint16_t CRC;
} i2c_pkt_t;

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
    SENS_MAX_RANGE = 65535
};

// DEVICE STATE
byte STATE = GET_SENSACT_NUM;

unsigned char I2C_SLAVE_ADDRESS = 0;

typedef struct tru2air_header_t {
    unsigned char action;
    unsigned char specifier;
} tru2air_header_t;

tru2air_header_t HEADER = {0xff, 0xff};

typedef struct sensact_descriptor_t {
    unsigned char name[24];
    uint16_t type;
} sensact_descriptor_t;


//TEMPORARY DUMMY VARIABLES
byte SENS_NUM = 0x03;
int resetTimer = 0;
unsigned int uint32_answer = 15;
sensact_descriptor_t sensors[] = {{"TRU2AIR_RELAY", SENSACT_TRU2AIR_RELAY}};
unsigned char device_addr[] = {0xde, 0xad, 0xbe, 0xef};
i2c_pkt_t lastReceivedPkt;
int resetVal = 0;

void setup() {

    Serial.begin(9600);  // start serial for output
    Serial.println(sizeof(i2c_pkt_t));
    Wire.begin(); // join i2c bus (address optional for master)
    // Starting the tru2air i2c protocol
    Wire.beginTransmission(TRU2AIR_MAIN_NODE_SLAVE_ADDR); // transmit to device #8
    Wire.write(0xde);
    Wire.write(0xad);
    Wire.write(0xbe);
    Wire.write(0xef);
    Wire.endTransmission(false);    // stop transmitting
    Wire.requestFrom(TRU2AIR_MAIN_NODE_SLAVE_ADDR, 1,
                     true);   // true means that repeated start is sent after the request
    while (!Wire.available()) { //
        /*Serial.println("4");
          delay(250);
          resetTimer++;
          if(resetTimer > 4) break;*/
    }
    I2C_SLAVE_ADDRESS = Wire.read(); // receive a byte as characterl //this read will be a Repeated start becouse of the Wire.requestFrom's true argument!!!!!


    //TODO: handle when no proper i2c id was received
    Wire.onReceive(receiveCb);
    Wire.onRequest(requestCb);
    Wire.begin(I2C_SLAVE_ADDRESS);
    Serial.print("[I2C SLAVE] Started i2c slave on: ");
    printHex4(&I2C_SLAVE_ADDRESS);
    Serial.print("\n");
}

void loop() {
    //if(resetVal) {resetVal = 0; delay(10);setup();};
}

void receiveCb(int numBytes) {
    Serial.print("numbYtes:");
    Serial.println(numBytes);
    i2c_pkt_t *pkt;
    uint8_t receivedData[sizeof(i2c_pkt_t)];
    int i = 0;
    for (i = 0; i < numBytes; i++) receivedData[i] = Wire.read();
    pkt = (i2c_pkt_t *) receivedData;
    printI2CPkt(pkt);
    if (pkt->CRC == crc16((uint8_t *) pkt, sizeof(i2c_pkt_t) - sizeof(uint16_t)))
        Serial.println("Good pkt");
    else Serial.println("Bad pkt"); //TODO: Check device ID TOO!!
    lastReceivedPkt = *pkt;
    switch (pkt->action) {
        case GET_SENSACT_NUM:
            Serial.print("[STATE] -> GET_SENSACT_NUM \n");
            STATE = GET_SENSACT_NUM;
            break;
        case GET_SENSOR_NAME:
            Serial.print("[STATE] -> GET_SENSOR_NAME \n");
            STATE = GET_SENSOR_NAME;
            break;
        case GET_SENSOR_TYPE:
            Serial.print("[STATE] -> GET_SENSOR_TYPE");
            STATE = GET_SENSOR_TYPE;
            break;

        case SENS_ACT_WRITE:
            STATE = SENS_ACT_WRITE;
            break;
        case SENS_ACT_READ:
            STATE = SENS_ACT_READ;
        default:
            Serial.print("[ERROR] Invalid state receieved! -> ");
            break;
    }
}

void requestCb() {
    Serial.print("[REQUEST]\n");
    i2c_pkt_t pkt;
    pkt = lastReceivedPkt;
    int i = 0;
//    for (i = 0; i < 32; i++) pkt.data[i]++;
//    pkt.action++;
    //delay(200);
    //setup();

    switch (STATE) {
        case GET_SENSACT_NUM:
            pkt.data[0] = sizeof(sensors) / sizeof(sensact_descriptor_t);
            //Wire.write(sizeof(sensors) / sizeof(sensact_descriptor_t));
            break;

        case GET_SENSOR_NAME:
            //TODO: handle bad HEADER
            for(i = 0; i < sizeof(sensors[HEADER.specifier].name); i++) pkt.data[i] = sensors[lastReceivedPkt.data[0]].name[i];
            break;

        case GET_SENSOR_TYPE:
            for(i = 0; i < 2; i++) pkt.data[i] = ((uint8_t*)( &sensors[lastReceivedPkt.data[0]].type))[i];
            //Wire.write((char *) (&sensors[HEADER.specifier].type), 2);
            break;
        case SENS_ACT_WRITE:
            break;
        case SENS_ACT_READ:

            float measurement;
            char BEresultArr[4];
            memset(SENSACT_MEASURE_RESULT, 0x00, SENSACT_DATA_SIZE);

            switch (HEADER.specifier) {

                /*

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
                  */
            }

            break;

        default:
            break;
    }
    pkt.CRC = crc16((uint8_t * ) & pkt, sizeof(i2c_pkt_t) - sizeof(uint16_t));
    Wire.write((uint8_t * ) & pkt, sizeof(i2c_pkt_t));
    Serial.print("[END OF REQUEST]\n");
    resetVal = 1;
}

void printHex4(byte *data) {
    char tmp[4];
    sprintf(tmp, "0x%02x", *data);
    Serial.print(tmp);
}

void convertLEFloatToBE(float *fl, char *BEFloatArr) {
    char i = 0;
    while (i < 4) {
        BEFloatArr[3 - i] = *(((char *) fl) + i);
        i++;
    }
}

void printI2CPkt(i2c_pkt_t *pkt) {
    return;
    int i = 0;
    Serial.print("pkt->dev_id: ");
    Serial.println(pkt->dev_id);
    Serial.print("pkt->action: ");
    Serial.println(pkt->action);
    Serial.print("pkt->error: ");
    Serial.println(pkt->error);
    for (i = 0; i < 32; i++) {
        Serial.print("pkt->data[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(pkt->data[i]);
    }
    Serial.print("pkt->CRC: ");
    Serial.println(pkt->CRC);
}

uint16_t crc16(uint8_t *data_p, uint8_t length) {
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t) x);
    }
    return crc;
}