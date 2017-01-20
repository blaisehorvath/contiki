#include "SAM.h"

#include <string.h>

#define SENSACTS_MAX_NUM 127

//TODO: theoretically this array can be greater that one byte, but not sure if that ever will be the case...
sensact_descriptor_t device_list[SENSACTS_MAX_NUM];

void sam_init() {
	int i;
	for (i = 0; i<SENSACTS_MAX_NUM; i++) {
		device_list[i] = (sensact_descriptor_t){0,0,"",0,0};
	}
}

//TODO: this deletes a device, not just a single sensor! Should it be handled?
void sam_del_device(uint32_t dev_id) {
	int i;
	for (i = 0; i < SENSACTS_MAX_NUM; i++) {
		if (device_list[i].dev_id != 0 && device_list[i].dev_id == dev_id) {
			device_list[i].dev_id = 0;
			strcpy(device_list[i].name , "");
			device_list[i].read = 0;
			device_list[i].sensor_id = 0;
			device_list[i].write = 0;
		}
	}
}

void sam_add_sensact(sensact_descriptor_t sensor) {
	int i;
	for (i = 0; i < SENSACTS_MAX_NUM; i++) {
		if (device_list[i].dev_id == 0) {
			device_list[i].dev_id = sensor.dev_id;
			strcpy(device_list[i].name, sensor.name); //TODO:possible error source, check length?
			device_list[i].sensor_id = sensor.sensor_id;
			device_list[i].read = *(sensor.read);
			device_list[i].write = *(sensor.write);
			break;
		}
	}
}

void sam_read_sensact(uint32_t device_addr, char sensact_addr, sensact_rw_result_t* result) {
	int i;
	_Bool found;
	for (i = 0; i < SENSACTS_MAX_NUM; i++) {
		if(device_list[i].dev_id == device_addr && device_list[i].sensor_id == sensact_addr) {
			device_list[i].read(result);
			found = 1;
			break;
		}
	}
	if (!found) {
		result->data = 0;
		result->err = SENSACT_MISSING;
		printf("read error \n");
	}
}

void sam_write_sensact(uint32_t device_addr, char sensact_addr, double data, sensact_rw_result_t* result) {
	int i;
	_Bool found;

	for (i=0; i < SENSACTS_MAX_NUM; i++) {
		if(device_list[i].dev_id == device_addr && device_list[i].sensor_id == sensact_addr) {
			device_list[i].write(data, result);
			found = 1;
			break;
		}
	}
	if (!found) {
		result->data = 0;
		result->err = SENSACT_MISSING;
		printf("write error \n");
	}
}

unsigned char sam_get_sensact_num() {
	unsigned char i;

	for (i=0; i<SENSACTS_MAX_NUM; i++) {
		if(device_list[i].dev_id == 0) return i;
	}

	return SENSACTS_MAX_NUM;
};

sensact_descriptor_t* sam_get_sensact_by_name(char* name) {
	unsigned char i;

	for ( i=0; i<SENSACTS_MAX_NUM; i++ ) {
		//TODO: !!!!!!!!! continue from here, get strcmp right
		if(device_list[i].dev_id != 0 && strcmp(device_list[i].name, name)==0 ) return &device_list[i];
	}
	return NULL;
}
