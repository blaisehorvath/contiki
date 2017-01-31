#include "SAM.h"



void sam_init() {
	int i;
	for (i = 0; i<SAM_SENSACTS_MAX_NUM; i++) {
		device_list[i] = (sensact_descriptor_t){0,0,"",0,0,0};
	}
}

//TODO: this deletes a device, not just a single sensor! Should it be handled?
void sam_del_device(uint32_t dev_id) {
	int i;
	for (i = 0; i < SAM_SENSACTS_MAX_NUM; i++) {
		if (device_list[i].dev_id != 0 && device_list[i].dev_id == dev_id) {
			device_list[i].dev_id = 0;
			strcpy(device_list[i].name , "");
			device_list[i].read = 0;
			device_list[i].sensact_id = 0;
			device_list[i].write = 0;
			device_list[i].sensact_return_type =  0;
		}
	}
}

void sam_add_sensact(sensact_descriptor_t sensor) {
	int i;
	for (i = 0; i < SAM_SENSACTS_MAX_NUM; i++) {
		if (device_list[i].dev_id == 0) {
			device_list[i].dev_id = sensor.dev_id;
			strcpy(device_list[i].name, sensor.name); //TODO:possible error source, check length?
			device_list[i].sensact_id = sensor.sensact_id;
			device_list[i].read = *(sensor.read);
			device_list[i].write = *(sensor.write);
			device_list[i].sensact_return_type = sensor.sensact_return_type;
			break;
		}
	}
}

void sam_read_sensact(sensact_descriptor_t* sensact, sensact_rw_result_t* result) {

	memset(&result->data, 0x00, SENSACT_DATA_SIZE); //TODO: remove this when the type system has finalized

	if(sensact != NULL) {
		sensact->read(sensact, result);
	}
	else {
		result->data[0] = 0;
		result->err = SENSACT_MISSING;
	}
}

void sam_write_sensact(sensact_descriptor_t* sensact, uint32_t* data, sensact_rw_result_t* result) {

	memset(&result->data, 0x00, SENSACT_DATA_SIZE); //TODO: remove this when the type system has finalized

	if(sensact != NULL) {
		sensact->write(sensact, data, result);
	}
	else {
		result->data[0] = 0;
		result->err = SENSACT_MISSING;
	}
}

unsigned char sam_get_sensact_num() {
	unsigned char i;

	for (i=0; i<SAM_SENSACTS_MAX_NUM; i++) {
		if(device_list[i].dev_id == 0) return i;
	}

	return SAM_SENSACTS_MAX_NUM;
};

sensact_descriptor_t* sam_get_sensact_by_name(char* name) {
	unsigned char i;

	for ( i=0; i<SAM_SENSACTS_MAX_NUM; i++ ) {
		if(device_list[i].dev_id != 0 && strcmp(device_list[i].name, name)==0 ) return &device_list[i];
	}
	return NULL;
}
