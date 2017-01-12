#include "cc1310_onboard_devices.h"

/*--------------------------------------------------------------
 *					Onboard sensors and actuators
 *--------------------------------------------------------------*/

sensor_descriptor_t red_led = {
		read_red_led,
		set_red_led,
		"RED LED",
		ONBOARD_DEV_ADDR,
		0x01
};

sensor_descriptor_t green_led = {
		read_green_led,
		set_green_led,
		"GREEN LED",
		ONBOARD_DEV_ADDR,
		0x02
};

/*--------------------------------------------------------------
 *						Red led
 *--------------------------------------------------------------*/

void set_red_led (double toWrite, sensact_rw_result_t* result) {
	if (toWrite == 1) {
		leds_on(LEDS_RED);
		result->data = 1;
		result->err = NO_SENSACT_ERROR;
	}
	else if (toWrite == 0) {
		leds_off(LEDS_RED);
		result->data = 0;
		result->err = NO_SENSACT_ERROR;
	}
	else {
		result->data = 0;
		result->err = WRITE_VALIE_OUT_OF_RANGE;
	}
}

void read_red_led (sensact_rw_result_t* result) {
	/*
	 * The leds get returns the result of the ti_lib's led arch get, which returns
	 * 0 if the value is 0 or if something went wrong, so there is no way to
	 * differentiate between when some error happened or the led is off...
	 * */
	if ( leds_get() && LEDS_RED ) {
		result->data = 1;
		result->err = NO_SENSACT_ERROR;
	}
	else {
		result->data = 0;
		result->err = NO_SENSACT_ERROR;
	}
}


/*--------------------------------------------------------------
 *						Green led
 *--------------------------------------------------------------*/


void set_green_led (double toWrite, sensact_rw_result_t* result) {
	if (toWrite == 1) {
		leds_on(LEDS_GREEN);
		result->data = 1;
		result->err = NO_SENSACT_ERROR;
	}
	else if (toWrite == 0) {
		leds_off(LEDS_GREEN);
		result->data = 0;
		result->err = NO_SENSACT_ERROR;
	}
	else {
		result->data = 0;
		result->err = WRITE_VALIE_OUT_OF_RANGE;
	}
}

void read_green_led (sensact_rw_result_t* result) {
	/*
	 * The leds get returns the result of the ti_lib's led arch get, which returns
	 * 0 if the value is 0 or if something went wrong, so there is no way to
	 * differentiate between when some error happened or the led is off...
	 * */
	if ( leds_get() && LEDS_GREEN ) {
		result->data = 1;
		result->err = NO_SENSACT_ERROR;
	}
	else {
		result->data = 0;
		result->err = NO_SENSACT_ERROR;
	}
}
