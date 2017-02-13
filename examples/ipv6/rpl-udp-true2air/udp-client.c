/*
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

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "sys/ctimer.h"

#include <stdio.h>
#include <string.h>
#include "true2air_prot.h"

/* Only for TMOTE Sky? */
#include "dev/serial-line.h"
#include "dev/uart1.h"
#include "net/ipv6/uip-ds6-route.h"
#include "dev/leds.h"
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"

#ifndef PERIOD
#define PERIOD 10
#endif

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define START_INTERVAL		(15 * CLOCK_SECOND)
#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))
#define SET_IP_TIME (10* CLOCK_SECOND)
#define MAX_PAYLOAD_LEN		30

#define ROM_BOOTLOADER_ENABLE                 1
#define BOOTLOADER_ENABLE = 0xC5
#define BL_LEVEL = 0x00
#define BL_PIN_NUMBER = 0x0B
#define BL_ENABLE = 0xC5

#define I2C_WAIT_INTERVAL (CLOCK_SECOND/10)
#define I2C_CHECK_INTERVAL (CLOCK_SECOND/2)
static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;


#include "SAM.h"
#include "tru2air_i2c_protocol.h"
#include "cc1310_onboard_devices.h"
#ifndef SIMULATED
#include "bus_manager.h"
#endif

#include "project-conf.h"

extern sensact_descriptor_t cc1310_green_led, cc1310_red_led;

#ifndef SIMULATED
extern sensact_descriptor_t cc1310_relay0, cc1310_relay1, cc1310_relay2, cc1310_relay3;
/* Globals */
extern volatile enum TRU2AIR_CLIENT_NODE_I2C_HANDLER_STATE STATE;
unsigned char master_dev_id_buff[4];
unsigned char rec_bytes = 4;
extern volatile  tru2air_sensor_node_t DEVICE;
#endif

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);

#ifndef SIMULATED
/*---------------------------------------------------------------------------*/
/*								tru2air i2c isr							     */
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
			DEVICE.i2c_addr = bus_manager_register_i2c_device(DEVICE.dev_addr);
		}
	}
	// If a read byte request came from the master
	else if ( I2C_SLAVE_ACT_TREQ & ss ) {
		I2CSlaveDataPut(I2C0_BASE, DEVICE.i2c_addr);


		// switching state to DEVICE init
		STATE = NODE_I2C_MASTER_INIT;
		process_poll(&udp_client_process);
	}
	//TODO: make an else for error handling
}
/*---------------------------------------------------------------------------*/
#endif

static void tcpip_handler(void)
{
  rfnode_pkt pkt_out;
  rfnode_pkt *pkt_in;
  memset(&pkt_out,0,sizeof(rfnode_pkt));
  if(uip_newdata()) {
	pkt_in = (rfnode_pkt*)uip_appdata;
	if(node_pkt_reply(pkt_in,&pkt_out))
	  uip_udp_packet_sendto(client_conn, (void*)&pkt_out, sizeof(rfnode_pkt),
	                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
  }
}
/*---------------------------------------------------------------------------*/
static void
send_init_packet(void *ptr)
{
  rfnode_pkt pkt;int i = 0;
  PRINTF("Send SET_IPADDR to the server\n");
  pkt.cnt=1;
  memset(&pkt.data, 0x00, 32);
  pkt.data[0]=2;
  pkt.new_device = 1;
  pkt.msg = SET_IPADDR;
  pkt.pkt_cnt = 0;
#ifdef SIMULATED
  for ( i = 0; i < 22 ; i++) pkt.name[i] = 0;
#else
  for ( i = 0; i < 22 ; i++) pkt.name[i] = 0;
#endif
  sprintf(pkt.name, "SETIPADDRMSG");
  PRINTF("ZEROED\n");
  uip_udp_packet_sendto(client_conn, (void*)&pkt, sizeof(rfnode_pkt),
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

/* The choice of server address determines its 6LoPAN header compression.
 * (Our address will be compressed Mode 3 since it is derived from our link-local address)
 * Obviously the choice made here must also be selected in udp-server.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
 * e.g. set Context 0 to fd00::.  At present Wireshark copies Context/128 and then overwrites it.
 * (Setting Context 0 to fd00::1111:2222:3333:4444 will report a 16 bit compressed address of fd00::1111:22ff:fe33:xxxx)
 *
 * Note the IPCMV6 checksum verification depends on the correct uncompressed addresses.
 */

#if 0
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from server link-local (MAC) address */
  uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0x0250, 0xc2ff, 0xfea8, 0xcd1a); //redbee-econotag
#endif
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic;
static struct etimer i2cCheck;
  static struct ctimer i2cfasz;
#ifndef SIMULATED
  static struct etimer led_off;
#endif

  PROCESS_BEGIN();

  PROCESS_PAUSE();

#ifndef SIMULATED
  /* Bus manager stuff -------------------------------------------------------*/
  init_i2c_bus_manager();
  /*---------------------------------------------------------------------------*/
#endif

  /* SAM stuff --------------------------------------------------------------*/
  sam_init();


  /* Leds */
  leds_arch_init();
  leds_off(LEDS_RED && LEDS_GREEN);
  sam_add_sensact(cc1310_red_led);
  sam_add_sensact(cc1310_green_led);
  uint32_t toWrite = 1;

  sensact_rw_result_t led_result;
//  sam_write_sensact(&cc1310_red_led, &toWrite, &led_result);
//  sam_write_sensact(&cc1310_green_led, &toWrite, &led_result);

#ifndef SIMULATED
  /* Relays */
  sam_add_sensact(cc1310_relay1);
//  sam_write_sensact(&cc1310_relay1, &toWrite, &led_result);
//  printf("[RELAY] relay write error: %d\n", led_result.err);
//
//  sam_read_sensact(&cc1310_relay1, &led_result);
//  printf("[RELAY] relay read error: %d result: %d \n", led_result.err, led_result.data);
//
//  sam_read_sensact(&cc1310_relay1, &led_result);
//  printf("[RELAY] relay read error: %d result: %d \n", led_result.err, led_result.data);

  printf("[SAM] board sensact num is %d \n", sam_get_sensact_num());
#endif

  /* SAM stuff end -----------------------------------------------------------*/

  set_global_address();

  PRINTF("UDP client process started nbr:%d routes:%d\n",
         NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);

  print_local_addresses();

  /* new connection with remote host */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL);
  if(client_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

  /* initialize serial line */
  uart1_set_input(serial_line_input_byte);
  serial_line_init();

  etimer_set(&periodic, SEND_INTERVAL);
etimer_set(&i2cCheck, I2C_CHECK_INTERVAL);
  printf("sizeof(rfnode_pkt:%d) \n",sizeof(rfnode_pkt));
#ifndef SIMULATED
//  etimer_set(&led_off, 50);

  bus_manager_register_i2c_isr(i2c_slave_data_isr);
  bus_manager_init_i2c_slave(0x25);
  printf("[INFO] i2c slave listen initiated\n");
#endif

  printf("[STATUS] listening to UDP communication\n");
send_init_packet(0);
  while(1) {

    PROCESS_WAIT_EVENT();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
    if(etimer_expired(&periodic)){
    	etimer_reset(&periodic);
    if(!node_is_initialized()) send_init_packet(0); //So we can send again if we set it back.
    }
    if(ev == PROCESS_EVENT_POLL) {
        ctimer_set(&i2cfasz, I2C_WAIT_INTERVAL,init_tru2air_sensor_node, NULL);
    }
    if(etimer_expired(&i2cCheck)){
    i2c_bus_checker();
    etimer_reset(&i2cCheck);
    }
#ifndef SIMULATED
//    if(etimer_expired(&led_off)){
//    	leds_toggle(LEDS_RED);
//    	etimer_reset(&led_off);
//    }
#endif

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
