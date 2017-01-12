#include "SAM.h"

#include <string.h>

sensor_descriptor_t device_list[127];

void init_SAM() {
	int i;
	for (i = 0; i<127; i++) {
		device_list[i] = (sensor_descriptor_t){0,0,"",0,0};
	}
}

void del_device(uint32_t dev_id) {
	int i;
	for (i = 0; i < 127; i++) {
		if (device_list[i].dev_id != 0 && device_list[i].dev_id == dev_id) {
			device_list[i].dev_id = 0;
			strcpy(device_list[i].name , "");
			device_list[i].read = 0;
			device_list[i].sensor_id = 0;
			device_list[i].write = 0;
		}
	}
}

void add_sensact(sensor_descriptor_t sensor) {
	int i;
	for (i = 0; i < 127; i++) {
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
