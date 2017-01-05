#include "contiki.h"
#include "dev/leds.h"

#include <stdio.h> /* For printf() */

/* Variables the application specific event value */
static process_event_t event_data_ready;

/*---------------------------------------------------------------------------*/
/* We declare the two processes */
PROCESS(main_process, "Main process");
PROCESS(alt_process, "Alt process");

/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&main_process, &alt_process);
/*---------------------------------------------------------------------------*/
/* Implementation of the first process */
PROCESS_THREAD(main_process, ev, data) {
	// variables are declared static to ensure their values are kept
	// between kernel calls.
	static struct etimer timer;
	static int count = 0;

	// any process mustt start with this.
	PROCESS_BEGIN();

	/* allocate the required event */
	event_data_ready = process_alloc_event();

	/* Initialize the temperature sensor */
	printf("Init main process\n");


	// set the etimer module to generate an event in one second.
	etimer_set(&timer, CLOCK_CONF_SECOND / 4);

	while (1) {
		// wait here for the timer to expire
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

		leds_toggle(LEDS_RED);

		count++;
		printf("%d \n", count);

		if (count % 4 == 0) {

			// post an event to the print process
			// and pass a pointer to the last measure as data
			process_post(&alt_process, event_data_ready, &count);
		}

		// reset the timer so it will generate another event
		etimer_reset(&timer);
	}
	// any process must end with this, even if it is never reached.
PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Implementation of the second process */
PROCESS_THREAD(alt_process, ev, data) {
PROCESS_BEGIN();

while (1) {
	// wait until we get a data_ready event
	PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
	int green_led = 0;
	leds_on(LEDS_GREEN);
	while(++green_led<2000000000);
	printf("%d \n", green_led);
	leds_off(LEDS_GREEN);

	// display it
	printf("Alt process\n");
}
PROCESS_END();
}
/*---------------------------------------------------------------------------*/
