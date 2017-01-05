#include "bus_manager.h"
#include <stdio.h>
#include <string.h>
#include "dev/leds.h"

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

	int i;
	for (i = 0; i<127; i++) {
		i2c_devices[i] = 0;
	}
}

uint8_t register_i2c_device(uint32_t dev_addr) {
	unsigned char i; //the 0 i2c address is reserved for the msp430
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

void init_i2c_slave(uint8_t slave_addr, void (i2c_slave_data_isr)()) {
	/* Initing the slave module */

	/* First, make sure the SERIAL PD is on */
	PRCMPowerDomainOn(PRCM_DOMAIN_SERIAL);
	while((PRCMPowerDomainStatus(PRCM_DOMAIN_SERIAL)
		!= PRCM_DOMAIN_POWER_ON));

	/* Enable the clock to I2C */
	PRCMPeripheralRunEnable(PRCM_PERIPH_I2C0);
	PRCMLoadSet();
	while(!PRCMLoadGet());


	//------------------------------------------------------------------

	IOCPinTypeI2c(I2C0_BASE, CC1310_IOID_SDA, CC1310_IOID_SCL);

	//------------------------------------------------------------------

	/* Setting up the interrupt */
	I2CIntRegister(I2C0_BASE , i2c_slave_data_isr);

	I2CSlaveIntEnable(I2C0_BASE, I2C_SLAVE_INT_DATA);

	/* Enable and initialize the I2C slave module */
	I2CSlaveInit(I2C0_BASE, slave_addr);
}
