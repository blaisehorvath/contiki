#ifndef SIMULATED
#include "bus_manager.h"
#include <stdio.h>
#include <string.h>

#define I2C_BUS_ADDRESS_RANGE 127

static uint8_t slave_addr = 0x07;
static uint8_t interface = NO_INTERFACE;

void init_i2c_bus_manager () {

	int i;
	for (i = 0; i<I2C_BUS_ADDRESS_RANGE; i++) {
		i2c_devices[i] = 0;
	}
}

void bus_manager_clear_i2c_slave_data_int () {
	// Clearing the event
	I2CSlaveIntClear(I2C0_BASE, I2C_SLAVE_INT_DATA | I2C_SLAVE_INT_START | I2C_SLAVE_INT_STOP);

	(void)I2CSlaveDataGet(I2C0_BASE);
}

void bus_manager_r_sensact(sensact_descriptor_t* sensact, sensact_rw_result_t* result) {

	uint8_t i2c_addr = bus_manager_get_sensact_i2c_id(&sensact->dev_id);

	unsigned char headerBuff[2] = {SENS_ACT_READ, sensact->sensact_id};
	unsigned resultBuff[sizeof(int)];

	board_i2c_select(BOARD_I2C_INTERFACE_0, i2c_addr);
	board_i2c_write(headerBuff, TRU2AIR_HEADER_BUFF_SIZE);
	board_i2c_read((char*)&(result->data), sizeof(int)) ;
	board_i2c_shutdown();
}

void bus_manager_w_sensact(sensact_descriptor_t* sensact, unsigned int* toWrite, sensact_rw_result_t* result) {

	unsigned char i2c_addr = bus_manager_get_sensact_i2c_id(&sensact->dev_id);

	if (i2c_addr > 0 && i2c_addr <= I2C_BUS_ADDRESS_RANGE) {
		unsigned char outBuff[2 + sizeof(int)];
		outBuff[0] = SENS_ACT_WRITE;
		outBuff[1] = sensact->sensact_id;
		memcpy((char*) &outBuff[2], (char*) toWrite, 2+sizeof(int));

		board_i2c_select(BOARD_I2C_INTERFACE_0, i2c_addr);
		board_i2c_write(outBuff, 2 + sizeof(int));
		board_i2c_shutdown();

		result->data = 0;
		result->err = NO_SENSACT_ERROR;
	} else {
		result->data = 0;
		result->err = SENSACT_MISSING;
	}

}

uint8_t bus_manager_get_sensact_i2c_id (uint32_t* device_id) {
	unsigned char i;
	for(i=1; i<I2C_BUS_ADDRESS_RANGE; i++) {
		if(i2c_devices[i] == *device_id) return i;
	}
	return 0x00;
}

uint8_t bus_manager_register_i2c_device(uint32_t dev_addr) {
	unsigned char i; //the 0 i2c address is reserved for the msp430
	for (i = 1; i<I2C_BUS_ADDRESS_RANGE; i++) {
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

//TODO: maybe remove by dev addr?
void bus_manager_unregister_i2c_device (uint8_t i2c_addr) {
	i2c_devices[i2c_addr] = 0;
}

void bus_manager_init_i2c_slave(uint8_t slave_addr) {
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
	I2CSlaveIntEnable(I2C0_BASE, I2C_SLAVE_INT_DATA);

	/* Enable and initialize the I2C slave module */
	I2CSlaveInit(I2C0_BASE, slave_addr);
}

void bus_manager_register_i2c_isr (void (i2c_slave_data_isr)()) {

	/* Setting up the interrupt */
	I2CIntRegister(I2C0_BASE , i2c_slave_data_isr);

}

void bus_manager_disable_i2c_slave() {
	I2CSlaveIntDisable(I2C0_BASE, I2C_SLAVE_INT_DATA);

	if (accessible()) {
		I2CSlaveDisable(I2C0_BASE);
	}

	PRCMPeripheralRunDisable(PRCM_PERIPH_I2C0);
	PRCMLoadSet();
	while (!PRCMLoadGet());

	/*
	 * Set all pins to GPIO Input and disable the output driver. Set internal
	 * pull to match external pull
	 *
	 * SDA and SCL: external PU resistor
	 * SDA HP and SCL HP: MPU PWR low
	 */
	//  ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SDA_HP);
	//  ti_lib_ioc_io_port_pull_set(CC1310_IOID_SDA_HP, IOC_IOPULL_DOWN);
	//  ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SCL_HP);
	//  ti_lib_ioc_io_port_pull_set(CC1310_IOID_SCL_HP, IOC_IOPULL_DOWN);
	ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SDA);
	ti_lib_ioc_io_port_pull_set(CC1310_IOID_SDA, IOC_IOPULL_UP);
	ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SCL);
	ti_lib_ioc_io_port_pull_set(CC1310_IOID_SCL, IOC_IOPULL_UP);
}


bool board_i2c_read_until(uint8_t *data, char end)
{
  uint8_t i;
  bool success;
  bool isLastChar = false;

  /* Set slave address */
  ti_lib_i2c_master_slave_addr_set(I2C0_BASE, slave_addr, true);

  /* Check if another master has access */
  while(ti_lib_i2c_master_bus_busy(I2C0_BASE));

  /* Assert RUN + START + ACK */
  ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);


  i = 0;
  success = true;
  while(!isLastChar && success) {
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
    if(success) {
      data[i] = ti_lib_i2c_master_data_get(I2C0_BASE);
      if(data[i] != end) {
		ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
		i++;
      }
      else {
		ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
		while(ti_lib_i2c_master_bus_busy(I2C0_BASE));
    	isLastChar = true;
      }
    }
  }
  return success;
}


static bool accessible(void)
{
  /* First, check the PD */
  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
     != PRCM_DOMAIN_POWER_ON) {
    return false;
  }

  /* Then check the 'run mode' clock gate */
  if(!(HWREG(PRCM_BASE + PRCM_O_I2CCLKGR) & PRCM_I2CCLKGR_CLK_EN)) {
    return false;
  }

  return true;
}

void board_i2c_shutdown()
{
  interface = NO_INTERFACE;

  if(accessible()) {
    ti_lib_i2c_master_disable(I2C0_BASE);
  }

  ti_lib_prcm_peripheral_run_disable(PRCM_PERIPH_I2C0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /*
   * Set all pins to GPIO Input and disable the output driver. Set internal
   * pull to match external pull
   *
   * SDA and SCL: external PU resistor
   * SDA HP and SCL HP: MPU PWR low
   */
//  ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SDA_HP);
//  ti_lib_ioc_io_port_pull_set(CC1310_IOID_SDA_HP, IOC_IOPULL_DOWN);
//  ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SCL_HP);
//  ti_lib_ioc_io_port_pull_set(CC1310_IOID_SCL_HP, IOC_IOPULL_DOWN);

  ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SDA);
  ti_lib_ioc_io_port_pull_set(CC1310_IOID_SDA, IOC_IOPULL_UP);
  ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SCL);
  ti_lib_ioc_io_port_pull_set(CC1310_IOID_SCL, IOC_IOPULL_UP);
}
/*---------------------------------------------------------------------------*/
bool board_i2c_write(uint8_t *data, uint8_t len)
{
  uint32_t i;
  bool success;

  /* Write slave address */
  ti_lib_i2c_master_slave_addr_set(I2C0_BASE, slave_addr, false);

  /* Write first byte */
  ti_lib_i2c_master_data_put(I2C0_BASE, data[0]);

  /* Check if another master has access */
  while(ti_lib_i2c_master_bus_busy(I2C0_BASE));

  /* Assert RUN + START */
  ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
  while(ti_lib_i2c_master_busy(I2C0_BASE));
  success = i2c_status();

  for(i = 1; i < len && success; i++) {
    /* Write next byte */
    ti_lib_i2c_master_data_put(I2C0_BASE, data[i]);
    if(i < len - 1) {
      /* Clear START */
      ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
      while(ti_lib_i2c_master_busy(I2C0_BASE));
      success = i2c_status();
    }
  }

  /* Assert stop */
  if(success) {
    /* Assert STOP */
    ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
    while(ti_lib_i2c_master_bus_busy(I2C0_BASE));
  }

  return success;
}

bool board_i2c_write_single(uint8_t data)
{
  /* Write slave address */
  ti_lib_i2c_master_slave_addr_set(I2C0_BASE, slave_addr, false);

  /* Write first byte */
  ti_lib_i2c_master_data_put(I2C0_BASE, data);

  /* Check if another master has access */
  while(ti_lib_i2c_master_bus_busy(I2C0_BASE));

  /* Assert RUN + START + STOP */
  ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
  while(ti_lib_i2c_master_busy(I2C0_BASE));

  return i2c_status();
}

bool board_i2c_read(uint8_t *data, uint8_t len)
{
  uint8_t i;
  bool success;

  /* Set slave address */
  ti_lib_i2c_master_slave_addr_set(I2C0_BASE, slave_addr, true);

  /* Check if another master has access */
  while(ti_lib_i2c_master_bus_busy(I2C0_BASE));

  /* Assert RUN + START + ACK */
  ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

  i = 0;
  success = true;
  while(i < (len - 1) && success) {
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
    if(success) {
      data[i] = ti_lib_i2c_master_data_get(I2C0_BASE);
      ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
      i++;
    }
  }

  if(success) {
    ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
    if(success) {
      data[len - 1] = ti_lib_i2c_master_data_get(I2C0_BASE);
      while(ti_lib_i2c_master_bus_busy(I2C0_BASE));
    }
  }

  return success;
}

void board_i2c_wakeup()
{
  /* First, make sure the SERIAL PD is on */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
        != PRCM_DOMAIN_POWER_ON));

  /* Enable the clock to I2C */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_I2C0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Enable and initialize the I2C master module */
  ti_lib_i2c_master_init_exp_clk(I2C0_BASE, ti_lib_sys_ctrl_clock_get(),
                                 true);
}
void board_i2c_select(uint8_t new_interface, uint8_t address)
{
  slave_addr = address;

  if(accessible() == false) {
    board_i2c_wakeup();
  }

  if(new_interface != interface) {
    interface = new_interface;

    ti_lib_i2c_master_disable(I2C0_BASE);

    if(interface == BOARD_I2C_INTERFACE_0) {
      ti_lib_ioc_io_port_pull_set(CC1310_IOID_SDA, IOC_NO_IOPULL);
      ti_lib_ioc_io_port_pull_set(CC1310_IOID_SCL, IOC_NO_IOPULL);
//      printf("sda %i , scl %i \n", CC1310_IOID_SDA, CC1310_IOID_SCL);
      ti_lib_ioc_pin_type_i2c(I2C0_BASE, CC1310_IOID_SDA, CC1310_IOID_SCL);
//      ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SDA_HP);
//      ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SCL_HP);
    } else if(interface == BOARD_I2C_INTERFACE_1) {
//      ti_lib_ioc_io_port_pull_set(CC1310_IOID_SDA_HP, IOC_NO_IOPULL);
//      ti_lib_ioc_io_port_pull_set(CC1310_IOID_SCL_HP, IOC_NO_IOPULL);
//      ti_lib_ioc_pin_type_i2c(I2C0_BASE, CC1310_IOID_SDA_HP, CC1310_IOID_SCL_HP);
      ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SDA);
      ti_lib_ioc_pin_type_gpio_input(CC1310_IOID_SCL);
    }

    /* Enable and initialize the I2C master module */
    ti_lib_i2c_master_init_exp_clk(I2C0_BASE, ti_lib_sys_ctrl_clock_get(),
                                   false);
  }
}

static bool i2c_status()
{
  uint32_t status;

  status = ti_lib_i2c_master_err(I2C0_BASE);
  if(status & (I2C_MSTAT_DATACK_N_M | I2C_MSTAT_ADRACK_N_M)) {
    ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
  }

  return status == I2C_MASTER_ERR_NONE;
}
#endif
