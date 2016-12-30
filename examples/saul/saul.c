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
#include "tru2air_i2c_com.h"

// temporary includes
#include "board-i2c.h"
#include "ti-lib.h"
#include <stdbool.h>
#include "i2c.h"
/*---------------------------------------------------------------------------*/

/*-----------------------------------TESTS-----------------------------------*/
#include "test_saul.h"
#include "dev/leds.h"

/* Tru2Air protocol defines */
#define TRU2AIR_HEADER_BUFF_LENGTH 2


/* Globals */
unsigned char master_dev_id_buff[4];
unsigned char rec_bytes = 4;
unsigned char buff [5];
typedef struct tru2air_sensor_t {
    unsigned int dev_addr;
    unsigned char i2c_addr;
    unsigned char sensact_num;
} tru2air_sensor_t;
volatile tru2air_sensor_t DEVICE = {0,0,0};
unsigned char currentSensor = 0; //TODO: reset on the proper place

/**
 * A stuct that hold an action (see I2C_COMM_PROT_ACTION and e specifier.
 * The specifier specifies the target of the action.
 */
typedef struct tru2air_header_t {
  unsigned char action;
  unsigned char specifier;
} tru2air_header_t;
tru2air_header_t HEADER;

/* BUFFERS */
unsigned char headerBuff[2];
unsigned char nameBuff[23];

/* States */
enum states {I2C_SLAVE_INIT, I2C_MASTER_INIT, GET_SENSACT_INFO, DEBUG };
volatile enum states STATE = I2C_SLAVE_INIT;
enum I2C_COMM_PROT_ACTION comm_type = GET_SENSACT_NUM;

//TEMPORARY VARIABLES
#define CC1310_IOID_SDA 13
#define CC1310_IOID_SCL 14
#define NO_INTERFACE 0xFF
static uint8_t slave_addr = 0x10;
static uint8_t interface = BOARD_I2C_INTERFACE_0;
void init_i2c_slave();
void clearBuffer();
unsigned int fasz1 = 0xdededede;
unsigned char fasz2 = 0x00;

/*---------------------------------------------------------------------------*/
PROCESS(saul, "saul");
AUTOSTART_PROCESSES(&saul);
/*---------------------------------------------------------------------------*/


void i2c_slave_data_isr () {
	// Reading the Slave Status
	uint32_t ss = I2CSlaveStatus(I2C0_BASE);

	// Clearing the event
	I2CSlaveIntClear(I2C0_BASE, I2C_SLAVE_INT_DATA | I2C_SLAVE_INT_START | I2C_SLAVE_INT_STOP);

	/* Waiting 2 clocks after clearing as suggested in Ti Driverlib CC13xx Ware */
	int i =0;
	i++;

	// If the first byte (FBR) or any master written byte arrived from the master
	if( (I2C_SLAVE_ACT_RREQ_FBR | I2C_SLAVE_ACT_RREQ) & ss) {
		master_dev_id_buff[--rec_bytes] = (unsigned char) I2CSlaveDataGet(I2C0_BASE);
		if (rec_bytes == 0) {
			rec_bytes = 4;
			memcpy(&(DEVICE.dev_addr), master_dev_id_buff, 4);
			DEVICE.i2c_addr = register_i2c_device(DEVICE.dev_addr);
		}
	}
	// If a read byte request came from the master
	else if ( I2C_SLAVE_ACT_TREQ & ss ) {
		I2CSlaveDataPut(I2C0_BASE, DEVICE.i2c_addr);

		// switching state to DEVICE init
		STATE = I2C_MASTER_INIT;
	}

	//TODO: make an else for error handling

}

PROCESS_THREAD(saul, ev, data)
{
  PROCESS_BEGIN();

  printf("\n[STATE] -> INIT \n[INFO] sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);

  /* Initing the Tru2Air Bus Manager */
  init_i2c_bus_manager();


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
	  	  case ( I2C_SLAVE_INIT ):
	  			break;

	  	  case ( I2C_MASTER_INIT ):

				printf("[STATE] -> I2C_MASTER_INIT\n");

				if (DEVICE.i2c_addr) {

					//Unregister the slave interrupt and turn off slave mode
					I2CIntUnregister(I2C0_BASE);
					printf("[INFO] unregistered the slave interrupts \n");

					disable_i2c_slave();
					printf("[INFO] disabling i2c slave \n");

					// Wake up as master
					board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
					board_i2c_write_single(GET_SENSACT_NUM);
					clearBuffer();
					board_i2c_read(buff,5); //TODO: error handling, check if the dev address is valid and maybe if the SENSACT num is >0?
					DEVICE.sensact_num = buff[4];
					currentSensor = 0;
					board_i2c_shutdown();

					printf("[INFO] tru2air sensor node: 0x%08x has 0x%02x sensors\n", DEVICE.dev_addr, DEVICE.sensact_num);

					STATE = GET_SENSACT_INFO;
				}
	  			break;

	  	  case ( GET_SENSACT_INFO ):
	  			headerBuff[0] = GET_SENSOR_DESC;
	  	  	  	headerBuff[1] = currentSensor;
	  	  	  	if (++currentSensor == DEVICE.sensact_num) STATE = 5;


				printf("[STATE] -> GET_SENSACT INFO\n");

				board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
				board_i2c_write(headerBuff, TRU2AIR_HEADER_BUFF_LENGTH);
				board_i2c_read(nameBuff,23);
				board_i2c_shutdown();
				break;

	  	  default:
	  		  if ((++i % 5000000) == 0 ) printf("[STATE] -> DEFAULT\n[INFO] tru2air sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);
	  		  break;
	  }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

void init_i2c_slave() {
	/* Initing the slave module */
	printf("[STATE] -> I2C_SLAVE_INIT \n[INFO] sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);

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

void clearBuffer() {
	unsigned char i;
	for(i=0 ; i<5; i++ ) {
		buff[i]=0;
	}
}


