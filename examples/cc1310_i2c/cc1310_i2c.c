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

#include "contiki.h"

#include <stdio.h> /* For printf() */
#include <stdbool.h>
#include "ti-lib.h"
#include "i2c.h"
#include "board-i2c.h"


#include <inc/hw_memmap.h>
//
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "ti-lib.h"
#include "board-i2c.h"
#include "lpm.h"

#include <string.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define NO_INTERFACE 0xFF

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Enter bootloader mode with BTN1+RESET\n");

  printf("Selecting I2C 0 \n");
  uint8_t slave_addr = 0x02;
  board_i2c_select(0,slave_addr);

  uint8_t data[24];


  board_i2c_read(&data[0],24);
  board_i2c_shutdown();


  double temp = 0;
  double pres = 0;
  double hum = 0;

  for (int i = 0; i<8; i++) {
	  *(((char*)&temp)+i)=data[i];
	  *(((char*)&pres)+i)=data[i+8];
	  *(((char*)&hum)+i)=data[i+16];
  }



  for (int i = 0; i<(24); i++) {
	  if (i==0) {
		  printf("\nT: 0x%x", (unsigned char)data[i]);
	  }
	  else if (i==8) {
		  printf("\np: 0x%x", (unsigned char)data[i]);
	  }
	  else if (i==16) {
		  printf("\nhumidity: 0x%x", (unsigned char)data[i]);
	  }
	  else {
		  printf("[%i]", i);
		  printf("%x", (unsigned char)data[i]);
	  }
  }
  printf("\n %d, %d, %d ",(int)(temp*1000),(int)(pres), (int)(hum*1000) );

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
