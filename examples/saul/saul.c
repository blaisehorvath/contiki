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
volatile unsigned int sensor_node_dev_id = 0;
unsigned char rec_bytes = 4;
volatile unsigned char sensor_node_i2c_id = 0;
unsigned char buff [0];

/* States */
enum states {I2C_SLAVE, DEVICE_INIT };
volatile enum states STATE = I2C_SLAVE;

//TEMPORARY VARIABLES
#define CC1310_IOID_SDA 13
#define CC1310_IOID_SCL 14
#define NO_INTERFACE 0xFF
static uint8_t slave_addr = 0x10;
static uint8_t interface = BOARD_I2C_INTERFACE_0;
void init_i2c_slave();
unsigned int fasz1 = 0xdededede;
unsigned char fasz2 = 0x00;
unsigned char debug_initiated_i2c = 0;

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
			memcpy(&sensor_node_dev_id, master_dev_id_buff, 4);
			debug_initiated_i2c = register_i2c_device(sensor_node_dev_id);
			sensor_node_i2c_id = debug_initiated_i2c;
		}
	}
	// If a read byte request came from the master
	else if ( I2C_SLAVE_ACT_TREQ & ss ) {
		I2CSlaveDataPut(I2C0_BASE, sensor_node_i2c_id);

		// switching state to DEVICE init
		STATE = DEVICE_INIT;
	}

	//TODO: make an else for error handling

}

PROCESS_THREAD(saul, ev, data)
{
  PROCESS_BEGIN();

  printf("sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", sensor_node_i2c_id, sensor_node_dev_id);

  /* Initing the Tru2Air Bus Manager */
  init_i2c_bus_manager();

  fasz2 = register_i2c_device(fasz1);
  printf("recieved dummy i2c from bus manager: 0x%02x \n", fasz2);

  /* Inititng I2C SLAVE */
  init_i2c_slave();


//  /* Delay 1 second */
//  etimer_set(&et, CLOCK_SECOND);

  /*=====================================================================
   * 						Get DevID as Slave
   * ====================================================================*/


  int i = 0;
  while(1) {
	  switch (STATE) {
	  	  case ( I2C_SLAVE ):
	  			break;
	  	  case ( DEVICE_INIT ):

				if (sensor_node_i2c_id) {
//
					//Unregister the slave interrupt and turn off the slave mode
					I2CIntUnregister(I2C0_BASE);
					printf("unregistered the slave interrupts \n");

					disable_i2c_slave();
					printf("disabling i2c slave \n");

					// Wake up as master
					board_i2c_select(BOARD_I2C_INTERFACE_0, sensor_node_i2c_id);
					board_i2c_read(buff,1);
					board_i2c_shutdown();
					printf("Recieved data from arduino slave: 0x%02x \n", buff[0]);

//					sensor_node_i2c_id = 0;
//
					STATE = 5;
				}
	  			break;

	  	  default:
	  		  if ((++i % 5000000) == 0 ) printf("in default state: sensor node i2c_id: 0x%02x debug_init_i2c: 0x%02x dev_addr: 0x%08x \n", sensor_node_i2c_id, debug_initiated_i2c, sensor_node_dev_id);
	  		  break;
	  }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

void init_i2c_slave() {

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

  /* Enable and initialize the I2C slave module */
  I2CSlaveInit(I2C0_BASE, slave_addr);


  printf("slave init\n");
}




