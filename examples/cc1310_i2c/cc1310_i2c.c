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

<<<<<<< HEAD

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
//static uint8_t slave_addr = 0x00;
//static uint8_t interface = NO_INTERFACE;
/*---------------------------------------------------------------------------*/

//static uint32_t i2c_base = 0x40002000;
//static uint32_t i2c_clk = 400000;
//static uint8_t bme280_addr = 0x76;
=======
#define BOARD_IOID_SDA            IOID_5 /**< Interface 0 SDA: All sensors bar MPU */
#define BOARD_IOID_SCL            IOID_6 /**< Interface 0 SCL: All sensors bar MPU */
#define BOARD_IOID_SDA_HP         IOID_8 /**< Interface 1 SDA: MPU */
#define BOARD_IOID_SCL_HP         IOID_9 /**< Interface 1 SCL: MPU */
static uint32_t i2c_base = 0x40002000;
static uint8_t bme280_addr = 0x76;
>>>>>>> 0bb7fe8bc5393fb9b34a68205fd1569ff3d11c4d



/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  PROCESS_BEGIN();
<<<<<<< HEAD

  printf("Enter bootloader mode with BTN1+RESET\n");

  printf("Selecting I2C 0 \n");
  uint8_t slave_addr = 0x02;
  board_i2c_select(0,slave_addr);

  uint8_t data[8];
  board_i2c_read(&data[0],8);

  for (int i = 0; i<8; i++) {
	  printf("byte %i is 0x%x \n", i, (unsigned char)data[i]);
  }

  double temp = *((double*)data);
  printf("temp is: %f \n", temp);
  double a;
  int size = sizeof(a);

  printf("size of double in contiki is %i", size);
//  printf("data is %i \n", (int)data);
  board_i2c_shutdown();

=======
  uint8_t res;
  uint8_t regaddr = 0xD0;
  printf("Hello, world\n");
  printf("Enabling entering bootloader mode with BTN1+Reset BTN!\n");
  board_i2c_select(0,bme280_addr);
  board_i2c_write_read(&regaddr,1, &res,1);
  printf("res:%d",res);
>>>>>>> 0bb7fe8bc5393fb9b34a68205fd1569ff3d11c4d
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
