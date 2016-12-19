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


/* Globals */
unsigned char master_dev_id_buff[4];
volatile unsigned int master_dev_id = 0;
unsigned char rec_bytes = 4;
volatile unsigned char sensor_node_i2c_id = 0;

//TEMPORARY VARIABLES
#define CC1310_IOID_SDA 13
#define CC1310_IOID_SCL 14
#define NO_INTERFACE 0xFF
static uint8_t slave_addr = 0x10;
static uint8_t interface = BOARD_I2C_INTERFACE_0;


int ansIndex = 0;

static struct etimer et;

/*---------------------------------------------------------------------------*/
PROCESS(saul, "saul");
AUTOSTART_PROCESSES(&saul);
/*---------------------------------------------------------------------------*/


void i2c_slave_data_isr () {

	// Reading the Slave Status
	uint32_t ss = I2CSlaveStatus(I2C0_BASE);

	// Clearing the event
	I2CSlaveIntClear(I2C0_BASE, I2C_SLAVE_INT_DATA | I2C_SLAVE_INT_START | I2C_SLAVE_INT_STOP);
	// printf("slave status: 0x%08x \n", ss);

	/* Waiting 2 clocks after clearing as suggested in Ti Driverlib CC13xx Ware */
	int i =0;
	i++;

	// If the first byte (FBR) or any master written byte arrived from the master
	if( (I2C_SLAVE_ACT_RREQ_FBR | I2C_SLAVE_ACT_RREQ) & ss) {
		master_dev_id_buff[--rec_bytes] = (unsigned char) I2CSlaveDataGet(I2C0_BASE);
		if (rec_bytes == 0) {
			rec_bytes = 4;
			memcpy(&master_dev_id, master_dev_id_buff, 4);
			sensor_node_i2c_id = register_i2c_device(master_dev_id);
		}
	}
	// If a read byte request came from the master
	else if ( I2C_SLAVE_ACT_TREQ & ss ) {
		I2CSlaveDataPut(I2C0_BASE, sensor_node_i2c_id);
	}

	//TODO: make an else for error handling

}

PROCESS_THREAD(saul, ev, data)
{
  PROCESS_BEGIN();

  /* Initing the Tru2Air Bus Manager */
  init_i2c_bus_manager();

//  leds_arch_init(); //for debugging
//  leds_on(LEDS_GREEN);
//  leds_on(LEDS_RED);

  /* Delay 1 second */
  etimer_set(&et, CLOCK_SECOND);

  /* Initing the slave module */

  /* First, make sure the SERIAL PD is on */
  PRCMPowerDomainOn(PRCM_DOMAIN_SERIAL);
  while((PRCMPowerDomainStatus(PRCM_DOMAIN_SERIAL)
        != PRCM_DOMAIN_POWER_ON));

  /* Enable the clock to I2C */
  PRCMPeripheralRunEnable(PRCM_PERIPH_I2C0);
  PRCMLoadSet();
  while(!PRCMLoadGet());


  printf("\n1\n");
  //------------------------------------------------------------------

  IOCPinTypeI2c(I2C0_BASE, CC1310_IOID_SDA, CC1310_IOID_SCL);

  //------------------------------------------------------------------

  /* Setting up the interrupt */
  I2CIntRegister(I2C0_BASE , i2c_slave_data_isr);
  printf("2\n");

  I2CSlaveIntEnable(I2C0_BASE, I2C_SLAVE_INT_DATA);
  printf("3\n");

  /* Enable and initialize the I2C master module */
  I2CSlaveInit(I2C0_BASE, slave_addr);


  printf("slave init\n");

  while(1) {
	  printf("master dev id: 0x%08x i2c_id: 0x%02x \n", master_dev_id, sensor_node_i2c_id);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
