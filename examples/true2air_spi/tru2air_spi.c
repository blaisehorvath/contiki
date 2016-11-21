/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#define ROM_BOOTLOADER_ENABLE                 1
#define BOOTLOADER_ENABLE = 0xC5
#define BL_LEVEL = 0x00
#define BL_PIN_NUMBER = 0x0B
#define BL_ENABLE = 0xC5

/*----- Defining SPI stuff -----*/
#define BOARD_IOID_SPI_SCK2        IOID_12
#define BOARD_IOID_SPI_MOSI2       IOID_11
#define BOARD_IOID_SPI_MISO2       IOID_9
#define CS IOID_8


#include "contiki.h"
#include "ti-lib.h"

#include <stdio.h> /* For printf() */

void SSIIntResponse () {
	uint32_t data;
	(void)SSIDataGetNonBlocking(SSI0_BASE,&data);
	printf("SPI RECEIEVE %d\n",((char*)&data)[0]);
	uint32_t answer = 17;
	printf("SPI ANSWER: %d \n", answer);
	SSIDataPut(SSI0_BASE, answer);
}

/*---------------------------------------------------------------------------*/
PROCESS(tru2air_spi, "tru2air_spi");
AUTOSTART_PROCESSES(&tru2air_spi);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tru2air_spi, ev, data)
{
  PROCESS_BEGIN();

  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
        != PRCM_DOMAIN_POWER_ON));

  /* Enable the clock to I2C */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_SSI0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  ti_lib_ioc_pin_type_ssi_slave(SSI0_BASE, BOARD_IOID_SPI_MOSI2,
		  BOARD_IOID_SPI_MISO2, CS, BOARD_IOID_SPI_SCK2 );

  printf("Welcome to the tru2air SPI test.\n");
  SSIConfigSetExpClk(SSI0_BASE, ti_lib_sys_ctrl_clock_get(), SSI_FRF_MOTO_MODE_0
                               , SSI_MODE_SLAVE, ti_lib_sys_ctrl_clock_get()/16,
							   8);

  SSIIntRegister(SSI0_BASE, SSIIntResponse);
  SSIIntEnable(SSI0_BASE, SSI_RXTO);

  uint32_t data;
  int i;
  SSIEnable(SSI0_BASE);
  while (1) {
	  for(i = 0; i < 5000000; i++){}
//	  SSIEnable(SSI0_BASE);
	  printf("status: %d\n", (uint32_t)SSIStatus(SSI0_BASE));
//	  SSIDisable(SSI0_BASE);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
