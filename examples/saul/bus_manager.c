#include "bus_manager.h"
#include <stdio.h>
#include <string.h>
#include "dev/leds.h"

#define NO_INTERFACE 0xFF

/**
 * Variables for dealing with the state.
 */
uint8_t received_bytes[4];
uint8_t rx_byte_count = 0;
uint8_t ack_byte_count = 0;

/**
 *  @brief This enum represents the state of the Tru2air sensor-node SPI communication protocol states
 */
enum STATUS {
	IDLE, /*!< Waiting for init from the master (sensor) */
	RX, /*!< Receiving data from master */
	RXTX, /*!< Receiving the last byte from the */
	END /*!< Getting ready for the nex init */
};

/**
 * Initializing the protocol. Idle is the state that handles the init.
 */
enum STATUS SPI_STATUS = IDLE;

void SPIcallback () {
	switch (SPI_STATUS) {
		case IDLE:
			//discarding the init (first) byte
			(void)getByteFromSPI();
			sendByteviaSPI((uint8_t)0xFF); //acking the init with 0xff
			SPI_STATUS = RX;
			break;
		case RX:
			received_bytes[rx_byte_count] = getByteFromSPI();
			rx_byte_count++;
			sendByteviaSPI((uint8_t)0x01); //acking the recieved byte with 0x01
			ack_byte_count++;
			if (ack_byte_count == 4) SPI_STATUS = RXTX;
			break;

		case RXTX:
			//receiving the last byte and sending a response to master
			received_bytes[rx_byte_count] = getByteFromSPI();
			sendByteviaSPI((uint8_t)0x07);
			SPI_STATUS = END;
			ack_byte_count = 0;
			rx_byte_count = 0;
			break;

		case END:
			(void)getByteFromSPI();
			sendByteviaSPI((uint8_t)0x01);
			SPI_STATUS = IDLE;
			printf("result is 0x%08x \n", *(uint32_t *)received_bytes);
			break;
	}
}

void requestData (uint8_t slave_addr);

void bus_scan() {
	uint8_t slaveAddr = 0x02;
	printf("Requesting from 0x%02x\n", slaveAddr);
	requestData(slaveAddr);
	printf("End of transmission");
}

_Bool detectDevice (uint8_t address) {
	uint8_t slave_addr = address;
	char messages[] = {'F', 'A', 'S', 'Z'};
	board_i2c_select(0,slave_addr);
	board_i2c_write(messages, sizeof(messages));
	board_i2c_shutdown();
	return (_Bool)1;
}

void requestData (uint8_t slave_addr) {
	char buff[1];
	board_i2c_select(0,slave_addr);
	board_i2c_read(buff,1);
	board_i2c_shutdown();
}

void init_i2c_bus_manager () {
	initSPISlave(SPIcallback);
	leds_arch_init();
	int i;
	for (i = 0; i<127; i++) {
		i2c_devices[i] = 0;
	}
}

uint8_t register_i2c_device(uint32_t dev_addr) {
	int i; //the 0 i2c address is reserved for the msp430
	for (i = 1; i<127; i++) {
		if(i2c_devices[i]==dev_addr) { // if the device reconnected before the manager had time to remove it
			return i;
		}
		else if (i2c_devices[i]==0) {
			i2c_devices[i] = dev_addr;
			return i;
		}
	}
	return 0;
}

void remove_i2c_device (uint8_t i2c_addr) {
	i2c_devices[i2c_addr] = 0;
	//TODO: remove the dev from spgbz too!!!
}
