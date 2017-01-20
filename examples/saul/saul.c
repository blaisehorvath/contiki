#define ROM_BOOTLOADER_ENABLE                 1
#define BOOTLOADER_ENABLE 0xC5
#define BL_LEVEL 0x00
#define BL_PIN_NUMBER 0x0B
#define BL_ENABLE 0xC5

/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "contiki-conf.h"
#include "SAM.h"
#include "bus_manager.h"
#include "ti-lib.h"
#include <stdbool.h>
#include "dev/leds.h"


/* Globals */
unsigned char master_dev_id_buff[4];
unsigned char rec_bytes = 4;
unsigned char buff [5];
volatile  tru2air_sensor_node_t DEVICE = {0,0,0};
unsigned char currentSensor = 0; //TODO: reset on the proper place

tru2air_header_t HEADER;

/* BUFFERS */
unsigned char headerBuff[2];
unsigned char nameBuff[23];
unsigned char typeBuff[2];

/* States */
enum states {NODE_I2C_SLAVE_INIT, NODE_I2C_MASTER_INIT, REQUIRE_SENSACT_NAME, REQUIRE_SENSOR_RETURN_TYPE, DEBUG };
volatile enum states STATE = NODE_I2C_SLAVE_INIT;
enum I2C_COMM_PROT_ACTION comm_type = GET_SENSACT_NUM;

//TEMPORARY VARIABLES
#define NO_INTERFACE 0xFF
void clearBuffer();

/*---------------------------------------------------------------------------*/
PROCESS(saul, "saul");
AUTOSTART_PROCESSES(&saul);
/*---------------------------------------------------------------------------*/


void saul_i2c_slave_isr () {
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
			DEVICE.i2c_addr = bus_manager_register_i2c_device(DEVICE.dev_addr);
		}
	}
	// If a read byte request came from the master
	else if ( I2C_SLAVE_ACT_TREQ & ss ) {
		I2CSlaveDataPut(I2C0_BASE, DEVICE.i2c_addr);

		// switching state to DEVICE init
		STATE = NODE_I2C_MASTER_INIT;
	}

	//TODO: make an else for error handling

}

PROCESS_THREAD(saul, ev, data)
{
  PROCESS_BEGIN();


  /* Initing the Tru2Air Bus Manager */
  init_i2c_bus_manager();

  /* Init SAM */
  sam_init();

  int slave_init = 0;
  int i = 0;
  while(1) {
	  switch (STATE) {
	  	  case ( NODE_I2C_SLAVE_INIT ):
	  			if (slave_init == 0) {

	  					printf("\n[STATE] -> INIT \n[INFO] sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);

	  					/* Register I2C Slave Interrupt */
	  					bus_manager_register_i2c_isr(saul_i2c_slave_isr);
	  					/* comment:
	  					 * this needs to be re registered EVEN IF THE I2CSlaveIntDisable is removed  from the bus_manager_disable_i2c_slave()...
	  					 * The first while loop works as intended, but when the second initiation i2c request comes to start the loop from NODE_I2C_SLAVE_INIT everything gets
	  					 * fucked up if the isr is not registered again with the code above. The following happens -> Interesting because it still generates
	  					 * one interrupt, but only one, and ACKs too. But after that last interrupt the devide drops the bus.
						 */

	  					/* Inititng I2C SLAVE as 0x10  and  enabling the registered I2C Slave Interrupt */
	  					bus_manager_init_i2c_slave(0x10);

	  					printf("[STATE] -> NODE_I2C_SLAVE_INIT \n[INFO] sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);
	  					slave_init = 1;
	  			}
	  			break;

	  	  case ( NODE_I2C_MASTER_INIT ):

				printf("[STATE] -> NODE_I2C_MASTER_INIT\n");

				if (DEVICE.i2c_addr) {

					//Unregister the slave interrupt and turn off slave mode
					I2CIntUnregister(I2C0_BASE);
					printf("[INFO] unregistered the slave interrupts \n");

					/* Disabling power to slave module and disabling i2c slave data interrupt */
					bus_manager_disable_i2c_slave();
					slave_init = 0;
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

					STATE = REQUIRE_SENSACT_NAME;
				}
	  			break;

	  	  case ( REQUIRE_SENSACT_NAME ):
	  			headerBuff[0] = GET_SENSOR_NAME;
	  	  	  	headerBuff[1] = currentSensor;
	  	  	  	if (++currentSensor == DEVICE.sensact_num) {
	  	  	  		STATE = REQUIRE_SENSOR_RETURN_TYPE;
	  	  	  		currentSensor = 0;
	  	  	  	}


				printf("[STATE] -> GET_SENSOR_NAME\n");

				board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
				board_i2c_write(headerBuff, TRU2AIR_HEADER_BUFF_SIZE);
				board_i2c_read_until(nameBuff,'\0');
				board_i2c_shutdown();

				break;

	  	  case ( REQUIRE_SENSOR_RETURN_TYPE ):

	  			headerBuff[0] = GET_SENSOR_TYPE;
	  	  	  	headerBuff[1] = currentSensor;
	  	  	  	if (++currentSensor == DEVICE.sensact_num) {
	  	  	  		currentSensor = 0;
	  	  	  		STATE = 5;
	  	  	  	}

				printf("[STATE] -> GET_SENSOR_TYPE");
				board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
				board_i2c_write(headerBuff, TRU2AIR_HEADER_BUFF_SIZE);
				board_i2c_read(typeBuff,1);
				board_i2c_shutdown();

				// Registering to SAM


	  			break;

	  	  default:
	  		  printf("[STATE] -> DEFAULT\n[INFO] tru2air sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);
	  		  STATE = NODE_I2C_SLAVE_INIT;
	  		  break;
	  }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


void clearBuffer() {
	unsigned char i;
	for(i=0 ; i<5; i++ ) {
		buff[i]=0;
	}
}


