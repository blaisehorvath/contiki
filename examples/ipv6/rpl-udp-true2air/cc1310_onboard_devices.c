#include "cc1310_onboard_devices.h"

/*--------------------------------------------------------------
 *					Onboard sensors and actuators
 *--------------------------------------------------------------*/

sensact_descriptor_t cc1310_red_led = {
		read_red_led,
		set_red_led,
		"RED LED",
		TRU2AIR_CLIENT_ONBOARD_LEDS,
		0x01
};

sensact_descriptor_t cc1310_green_led = {
		read_green_led,
		set_green_led,
		"GREEN LED",
		TRU2AIR_CLIENT_ONBOARD_LEDS,
		0x02
};


#ifndef SIMULATED

unsigned char RELAY_PINS[] = {IOID_21, IOID_27, IOID_26, IOID_25};

sensact_descriptor_t cc1310_relay0 = {
		read_relay,
		write_relay,
		"RELAY0",
		TRU2AIR_CLIENT_ONBOARD_RELAYS,
		0x00
};

sensact_descriptor_t cc1310_relay1 = {
		read_relay,
		write_relay,
		"RELAY1",
		TRU2AIR_CLIENT_ONBOARD_RELAYS,
		0x01
};

sensact_descriptor_t cc1310_relay2 = {
		read_relay,
		write_relay,
		"RELAY2",
		TRU2AIR_CLIENT_ONBOARD_RELAYS,
		0x02
};


sensact_descriptor_t cc1310_relay3 = {
		read_relay,
		write_relay,
		"RELAY3",
		TRU2AIR_CLIENT_ONBOARD_RELAYS,
		0x03
};


/*--------------------------------------------------------------
 *						Relay read
 *--------------------------------------------------------------*/
void read_relay(sensact_descriptor_t* sensact, sensact_rw_result_t* result){


	if (sensact->dev_id != TRU2AIR_CLIENT_ONBOARD_RELAYS) {
		result->data = 0;
		result->err = INVALID_DEVICE_ADDR;
		return;
	}

	if (sensact->sensact_id > TRU2AIR_CLIENT_ONBOARD_RELAYS_NUM) {
		result->data = 0;
		result->err = INVALID_SENSACT_ID;
		return;
	}

//	IOCPinTypeGpioInput(RELAY_PINS[sensact->sensact_id]);
//	result->data = GPIO_readDio(RELAY_PINS[sensact->sensact_id]);
	result->data = (unsigned int) *( (unsigned int*) ( GPIO_BASE +  RELAY_PINS[sensact->sensact_id]) );
	printf("[GPIO READ] %d \n", result->data);
	result->err = NO_SENSACT_ERROR;
}


/*--------------------------------------------------------------
 *						Relay write
 *--------------------------------------------------------------*/
void write_relay(sensact_descriptor_t* sensact, uint32_t* setValue, sensact_rw_result_t* result){


	if (sensact->dev_id != TRU2AIR_CLIENT_ONBOARD_RELAYS) {
		result->data = 0;
		result->err = INVALID_DEVICE_ADDR;
		return;
	}

	if (sensact->sensact_id > TRU2AIR_CLIENT_ONBOARD_RELAYS_NUM) {
		result->data = 0;
		result->err = INVALID_SENSACT_ID;
		return;
	}

	if ((*setValue != 0 && *setValue !=1) || setValue == NULL) {
		result->data = 0;
		result->err = WRITE_VALUE_OUT_OF_RANGE;
		return;
	}

	IOCPinTypeGpioOutput(RELAY_PINS[sensact->sensact_id]);
	GPIO_writeDio(RELAY_PINS[sensact->sensact_id], *setValue);
	result->data = 0;
	result->err = NO_SENSACT_ERROR;
}

#endif

/*--------------------------------------------------------------
 *						Red led
 *--------------------------------------------------------------*/

void set_red_led (sensact_descriptor_t* sensor, uint32_t* toWrite, sensact_rw_result_t* result) {

	if (*toWrite == 1) {
		leds_on(LEDS_RED);
		result->data = 1;
		result->err = NO_SENSACT_ERROR;
	}
	else if (*toWrite == 0) {
		leds_off(LEDS_RED);
		result->data = 0;
		result->err = NO_SENSACT_ERROR;
	}
	else {
		result->data = 0;
		result->err = WRITE_VALUE_OUT_OF_RANGE;
	}
	printf("[SET RED LED] %d\n", result->data);
}

void read_red_led (sensact_descriptor_t* sensact, sensact_rw_result_t* result) {
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
	printf("[READ RED LED] %d\n", result->data);
}


/*--------------------------------------------------------------
 *						Green led
 *--------------------------------------------------------------*/


void set_green_led (sensact_descriptor_t* sensor, uint32_t* toWrite, sensact_rw_result_t* result) {
	if (*toWrite == 1) {
		leds_on(LEDS_GREEN);
		result->data = 1;
		result->err = NO_SENSACT_ERROR;
	}
	else if (*toWrite == 0) {
		leds_off(LEDS_GREEN);
		result->data = 0;
		result->err = NO_SENSACT_ERROR;
	}
	else {
		result->data = 0;
		result->err = WRITE_VALUE_OUT_OF_RANGE;
	}
	printf("[SET GREEN LED] %d\n", result->data);
}

void read_green_led (sensact_descriptor_t* sensact, sensact_rw_result_t* result) {
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
	printf("[READ GREEN LED] %d\n", result->data);
}
