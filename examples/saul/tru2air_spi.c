#include "tru2air_spi.h"

void initSPISlave(void (spi_int_cb)()){

  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
        != PRCM_DOMAIN_POWER_ON));

  /* Enable the clock for SPI */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_SSI0);

  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  ti_lib_ioc_pin_type_ssi_slave(SSI0_BASE, MOSI_PIN,
		  MISO_PIN, CS_PIN, SPI_SCK_PIN );

  SSIConfigSetExpClk(
			  SSI0_BASE,
			  ti_lib_sys_ctrl_clock_get(),
			  SSI_FRF_MOTO_MODE_0,
			  SSI_MODE_SLAVE,
			  ti_lib_sys_ctrl_444clock_get()/16,
			  8
		  );

  SSIIntRegister(SSI0_BASE, spi_int_cb);
  SSIIntEnable(SSI0_BASE, SSI_RXTO);
  SSIEnable(SSI0_BASE);
}

uint8_t getByteFromSPI(){
	uint32_t data;
	(void)SSIDataGetNonBlocking(SSI0_BASE,&data);
//	printf("SPI RECEIEVE 0x%02x\n",((char*)&data)[0]);
	return (((uint8_t*)&data)[0]);
}

void sendByteviaSPI(uint8_t answer){
//	printf("SPI ANSWER: %u \n", answer);
	SSIDataPutNonBlocking(SSI0_BASE, answer);
};


//	  SSIEnable(SSI0_BASE);
//	  printf("status: %d\n", (uint32_t)SSIStatus(SSI0_BASE));
//	  SSIDisable(SSI0_BASE);
/*---------------------------------------------------------------------------*/
