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
#define BOOTLOADER_ENABLE = 0xC5`
#define BL_LEVEL = 0x00`
#define BL_PIN_NUMBER = 0x0B` 
#define BL_ENABLE = 0xC5`

#include "contiki.h"

#include <stdio.h> /* For printf() */
#include <stdbool.h>
#include "ti-lib.h"
#include "i2c.h"
#include "board-i2c.h"

#define BOARD_IOID_SDA            IOID_5 /**< Interface 0 SDA: All sensors bar MPU */
#define BOARD_IOID_SCL            IOID_6 /**< Interface 0 SCL: All sensors bar MPU */
#define BOARD_IOID_SDA_HP         IOID_8 /**< Interface 1 SDA: MPU */
#define BOARD_IOID_SCL_HP         IOID_9 /**< Interface 1 SCL: MPU */
static uint32_t i2c_base = 0x40002000;
static uint8_t bme280_addr = 0x76;



/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  PROCESS_BEGIN();
  uint8_t res;
  uint8_t regaddr = 0xD0;
  printf("Hello, world\n");
  printf("Enabling entering bootloader mode with BTN1+Reset BTN!\n");
  board_i2c_select(0,bme280_addr);
  board_i2c_write_read(&regaddr,1, &res,1);
  printf("res:%d",res);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
