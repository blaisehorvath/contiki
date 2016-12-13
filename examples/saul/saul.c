#define ROM_BOOTLOADER_ENABLE                 1
#define BOOTLOADER_ENABLE 0xC5
#define BL_LEVEL 0x00
#define BL_PIN_NUMBER 0x0B
#define BL_ENABLE 0xC5
#define MMEM_CONF_SIZE 2048

/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "contiki-conf.h"
#include "spgbz.h"
#include "bus_manager.h"
#include "tru2air_spi.h"

// temporary includes
#include "board-i2c.h"
#include "ti-lib.h"
#include <stdbool.h>
#include "i2c.h"
/*---------------------------------------------------------------------------*/

/*-----------------------------------TESTS-----------------------------------*/
#include "test_saul.h"
#include "dev/leds.h"

//TEMPORARY VARIABLES
int slaveStatus;
unsigned int i2c_received_data;
#define CC1310_IOID_SDA 13
#define CC1310_IOID_SCL 14
#define NO_INTERFACE 0xFF
static uint8_t slave_addr = 0x10;
static uint8_t interface = BOARD_I2C_INTERFACE_0;



/*---------------------------------------------------------------------------*/
PROCESS(saul, "saul");
AUTOSTART_PROCESSES(&saul);
/*---------------------------------------------------------------------------*/


void i2c_slave_data_isr () {
	I2CSlaveIntClear(I2C0_BASE, I2C_SLAVE_INT_DATA);
	(void)I2CSlaveDataGet;
	;
}

PROCESS_THREAD(saul, ev, data)
{
  PROCESS_BEGIN();

  /* Initing the slave module */

  /* First, make sure the SERIAL PD is on */
  PRCMPowerDomainOn(PRCM_DOMAIN_SERIAL);
  while((PRCMPowerDomainStatus(PRCM_DOMAIN_SERIAL)
        != PRCM_DOMAIN_POWER_ON));

  /* Enable the clock to I2C */
  PRCMPeripheralRunEnable(PRCM_PERIPH_I2C0);
  PRCMLoadSet();
  while(!PRCMLoadGet());

  /* Enable and initialize the I2C master module */
  I2CSlaveInit(I2C0_BASE, slave_addr);

  printf("1");
  //------------------------------------------------------------------

  IOCPinTypeI2c(I2C0_BASE, CC1310_IOID_SDA, CC1310_IOID_SCL);

  //------------------------------------------------------------------

  /* Setting up the interrupt */
  I2CIntRegister(I2C0_BASE , i2c_slave_data_isr);
  printf("2");

  I2CSlaveIntEnable(I2C0_BASE, I2C_SLAVE_INT_DATA);
  printf("3");

  while(1);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/





/*
 *
  IntMasterEnable();
  printf("1");
  I2CIntRegister(I2C0_BASE , i2c_slave_data_isr);
  printf("2");
  board_i2c_select_slave(interface, slave_addr);
  printf("3");
  I2CSlaveIntEnable(I2C0_BASE, I2C_SLAVE_INT_DATA);
  printf("4");
//  while(1);
 *
*/








//    leds_arch_init();
//  	leds_on(LEDS_BLUE);
//  	int j=0;
//  	while(++j < 1000000);
//  	leds_off(LEDS_BLUE);
  //runTests();


// from isr

//	I2CSlaveIntClear(BOARD_I2C_INTERFACE_0, I2C_SLAVE_INT_DATA);
//	i2c_received_data = ti_lib_i2c_slave_data_get(I2C0_BASE);
//	printf("int");
