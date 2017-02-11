/*
 * tru2air_i2c_protocol.c
 *
 *  Created on: Jan 20, 2017
 *      Author: blaise
 */

#ifndef SIMULATED

#include <string.h>
#include <stdbool.h>
#include "tru2air_i2c_protocol.h"
#include "bus_manager.h"
#include "SAM.h"
#include "clock.h"
#include "sys/etimer.h"


/* Temporary Variables */
extern void i2c_slave_data_isr(); //TODO: init_tru2air_snesor_node should require a function pointer to this instead of extern

/* Globals */
volatile tru2air_sensor_node_t DEVICE = {0, 0, 0};
volatile unsigned char STATE = I2C_SLAVE_LISTEN;
volatile unsigned char ERROR = NO_I2C_ERROR;
unsigned char currentSensor = 0; //TODO: reset on the proper place

void printI2CPkt(i2c_pkt_t *pkt) {
    int i = 0;
    printf("pkt->dev_id: ");
    printf("%d\n", pkt->dev_id);
    printf("pkt->action: ");
    printf("%d\n", pkt->action);
    printf("pkt->error: ");
    printf("%d\n", pkt->error);
    for (i = 0; i < 32; i++) {
        printf("pkt->data[");
        printf("%d", i);
        printf("]: ");
        printf("%d\n", pkt->data[i]);
    }
    printf("pkt->CRC: ");
    printf("%d\n", pkt->CRC);
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

void convertLEToBE4(uint32_t *from, uint32_t *to) {
    char i = 0;
    while (i < 4) {
        ((char *) to)[3 - i] = *(((char *) from) + i);
        i++;
    }
}

uint32_t convertLEToBE4Return(uint32_t *from) {
    uint32_t retVal = 0;
    char i = 0;
    while (i < 4) {
        ((char *) &retVal)[3 - i] = *(((char *) from) + i);
        i++;
    }
    return retVal;
}

bool bus_manager_exchange_pkts(i2c_pkt_t *pkt_out, i2c_pkt_t *pkt_in, uint8_t i2c_addr) {
    i2c_pkt_t pktToSend = *pkt_out;
    convertLEToBE4(&pkt_out->dev_id, &pktToSend.dev_id);
    pktToSend.CRC = crc16(((uint8_t * )(&pktToSend)), sizeof(i2c_pkt_t) - sizeof(uint16_t));
    //TODO:Fn should be called here which from action/sensor type gets the right information.

    board_i2c_select(BOARD_I2C_INTERFACE_0, i2c_addr);
    if (!board_i2c_write(&pktToSend, sizeof(i2c_pkt_t))) return false;// TODO:Possible wait here, less errors
    if (!board_i2c_read(pkt_in, sizeof(i2c_pkt_t))) return false;
    //convertLEToBE4(&temp,&pkt_in->dev_id); //TODO: Who will be responsible for le2be??CRC checking etc...
    board_i2c_shutdown();
    return true;
}

volatile int testSum = 0;
volatile int testGood = 0;

void i2c_bus_checker() {
    //printf("in bus checker %d\n",STATE);
    uint8_t sensactBuff, i, retvalread, retvalwrite;
    for (i = 8; i < I2C_BUS_ADDRESS_RANGE; i++) {
        if (i2c_devices[i]) {
            //printf("%d device exsists \n",i);
            board_i2c_select(BOARD_I2C_INTERFACE_0, i);
            retvalwrite = board_i2c_write_single(GET_SENSACT_NUM);
            sensactBuff = 0;
            retvalread = board_i2c_read_single(&sensactBuff); // TODO: Check return value too.
            if (!retvalwrite || (!sensactBuff && retvalread)) {
                //printf("%d device deleted \n",i);
                sam_del_device(i2c_devices[i]);
                i2c_devices[i] = 0;
                // TODO: unregister device from SAM and i2c_devices
            }
            board_i2c_shutdown();
        }
    }
    /* Register I2C Slave Interrupt */
    bus_manager_register_i2c_isr(i2c_slave_data_isr);
    /* Inititng I2C SLAVE as 0x25  and  enabling the registered I2C Slave Interrupt */
    bus_manager_init_i2c_slave(0x25);
}

bool errorHandler(bool error, unsigned char errorType) {
    if (error) {
        STATE = I2C_ERROR;
        ERROR = errorType;
    }
    return error;
}

void init_tru2air_sensor_node() {
    int i;
    bool tru2air_sensor_device_inited = false;
    unsigned char headerBuff[2];
    unsigned char nameBuff[23];
    unsigned char typeBuff[2];
    unsigned char sensactBuff;
    i2c_pkt_t in_pkt = {0, 0, 0,
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0}, 0};
    i2c_pkt_t out_pkt = {0, 0, 0,
                         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0}, 0};
    /* Timing variables */
    while (!tru2air_sensor_device_inited) {
        switch (STATE) {
            case (I2C_SLAVE_LISTEN):

                /* Reseting DEVICE */
                DEVICE = (tru2air_sensor_node_t) {0, 0, 0};

                /* Register I2C Slave Interrupt */
                bus_manager_register_i2c_isr(i2c_slave_data_isr);

                /* Inititng I2C SLAVE as 0x25  and  enabling the registered I2C Slave Interrupt */
                bus_manager_init_i2c_slave(0x25);

                //printf("\n[STATE] -> NODE_I2C_SLAVE_LISTEN \n\n");
                tru2air_sensor_device_inited = true;
                break;

            case (NODE_I2C_MASTER_INIT):
                //printf("[STATE] -> NODE_I2C_MASTER_INIT\n");

                if (DEVICE.i2c_addr) {
                    I2CIntUnregister(I2C0_BASE);
                    bus_manager_disable_i2c_slave();
                    //printf("[INFO] disabling i2c slave \n");
                    out_pkt.action = GET_SENSACT_NUM;
                    out_pkt.dev_id = DEVICE.dev_addr;
                    out_pkt.error = NO_I2C_ERROR;

                    testSum++;
                    if (!bus_manager_exchange_pkts(&out_pkt, &in_pkt, DEVICE.i2c_addr)) {
                        ERROR = I2C_SENSACT_NUM_ERROR;
                        STATE = I2C_ERROR;
                        break;
                    }
                    if (in_pkt.CRC == crc16((uint8_t * ) & in_pkt, sizeof(i2c_pkt_t) - sizeof(uint16_t))) {
                        DEVICE.sensact_num = in_pkt.data[0];//TODO: Should be mapped with a struct both here and on arduino.
                        currentSensor = 0;
                        printf("[INFO] tru2air sensor node: 0x%08x has 0x%02x sensors\n\n", DEVICE.dev_addr,
                               DEVICE.sensact_num);
                        STATE = REQUIRE_SENSACT_NAME;
                    }
                    else {
                        ERROR = I2C_CRC_ERROR;//TODO: So many todos...
                        STATE = I2C_ERROR;
                        break;
                    }

                }
                break;
            case (REQUIRE_SENSACT_NAME):
                printf("[STATE] -> GET_SENSOR_NAME\n");
                for (i = 0; i < sizeof(i2c_pkt_t); i++) ((char *) (&out_pkt))[i] = 0;//zero out.
                out_pkt.action = GET_SENSOR_NAME;
                out_pkt.data[0] = currentSensor;

                if (!bus_manager_exchange_pkts(&out_pkt, &in_pkt, DEVICE.i2c_addr)) {
                    ERROR = I2C_GET_SENSOR_NAME_ERROR;
                    STATE = I2C_ERROR;
                    break;
                }
                if (in_pkt.CRC == crc16((uint8_t * ) & in_pkt, sizeof(i2c_pkt_t) - sizeof(uint16_t))) {
                    strcpy(nameBuff, in_pkt.data);
                    STATE = REQUIRE_SENSOR_RETURN_TYPE; //TODO: Continue from here
                }
                else {
                    ERROR = I2C_CRC_ERROR;
                    STATE = I2C_ERROR;
                    break;
                }
                break;


            case (REQUIRE_SENSOR_RETURN_TYPE):
                printf("[STATE] -> GET_SENSOR_TYPE\n");

                for (i = 0; i < sizeof(i2c_pkt_t); i++) ((char *) (&out_pkt))[i] = 0;//zero out.
                out_pkt.action = GET_SENSOR_TYPE;
                out_pkt.data[0] = currentSensor;

                if (!bus_manager_exchange_pkts(&out_pkt, &in_pkt, DEVICE.i2c_addr)) {
                    ERROR = I2C_GET_SENSOR_TYPE_ERROR;
                    STATE = I2C_ERROR;
                    break;
                }
                if (in_pkt.CRC == crc16((uint8_t * ) & in_pkt, sizeof(i2c_pkt_t) - sizeof(uint16_t))) {
                    typeBuff[0] = in_pkt.data[1];
                    typeBuff[1] = in_pkt.data[0]; //TODO:BELE
                    STATE = REQUIRE_SENSOR_RETURN_TYPE; //TODO: Continue from here
                }
                else {
                    ERROR = I2C_CRC_ERROR;
                    STATE = I2C_ERROR;
                    break;
                }

                printf("[GOT SENSACT] Name: %s Type: 0x%04x \n", nameBuff, *(uint16_t *) typeBuff);
                STATE = REGISTER_TO_SAM;
                break;

            case (REGISTER_TO_SAM):
                printf("[STATE] -> REGISTER_TO_SAM\n");
                sensact_descriptor_t new_sensact;

                strcpy((char *) &(new_sensact.name), nameBuff);
                new_sensact.dev_id = DEVICE.dev_addr;
                new_sensact.sensact_id = currentSensor;
                new_sensact.read = bus_manager_r_sensact;
                new_sensact.write = bus_manager_w_sensact;
                new_sensact.sensact_type = *((uint16_t *) typeBuff);

                sam_add_sensact(new_sensact);

                testGood++;
                printf("testGood: %d, testSum:%d \n", testGood, testSum);

                if (++currentSensor == DEVICE.sensact_num) {
                    currentSensor = 0;
                    STATE = I2C_SLAVE_LISTEN;
                    printf("[I2C SENSACTS INIT SUCCESSFUL]\n");
                } else {
                    STATE = REQUIRE_SENSACT_NAME;
                }
                break;
            case (I2C_ERROR):

                tru2air_sensor_device_inited = false;

                switch (ERROR) {
                    case (I2C_SENSACT_NUM_NULL_ERROR):
                        ERROR = NO_I2C_ERROR;
                        STATE = I2C_SLAVE_LISTEN;
                        printf("[ERROR] I2C SENSACT NUM NULL ERROR\n");
                        break;

                    case (I2C_SENSACT_NUM_ERROR):
                        ERROR = NO_I2C_ERROR;
                        STATE = I2C_SLAVE_LISTEN;
                        printf("[ERROR] I2C SENSACT NUM ERROR\n");
                        break;
                    case (I2C_CRC_ERROR):
                        ERROR = NO_I2C_ERROR;
                        STATE = I2C_SLAVE_LISTEN;
                        printf("[ERROR] I2C CRC ERROR\n");
                        break;
                    case (I2C_GET_SENSOR_NAME_ERROR):
                        ERROR = NO_I2C_ERROR;
                        STATE = I2C_SLAVE_LISTEN;
                        printf("[ERROR] I2C CRC ERROR\n");
                        break;
                    case (I2C_GET_SENSOR_TYPE_ERROR):
                        ERROR = NO_I2C_ERROR;
                        STATE = I2C_SLAVE_LISTEN;
                        printf("[ERROR] I2C CRC ERROR\n");
                        break;

                    default:;
                }
                board_i2c_shutdown();
                break;
        }
    }
}


#endif /* If simulated */
