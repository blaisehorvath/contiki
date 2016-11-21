#include "bus_manager.h"
#include <stdio.h>

#define NO_INTERFACE 0xFF

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
