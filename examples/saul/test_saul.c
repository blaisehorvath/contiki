#include "test_saul.h"

extern spgbz_list_t device_list[127];

bus_comm_t testFunc () {
	return (bus_comm_t){0,8.5};
}

void runTests() {
  printf("\nTesting init: %s \n", testInit() ? "success" : "fail");
  printf("Testing add item: %s \n", testAddItem() ? "success" : "fail");
  printf("Testing deleting items: %s \n", testDeleteItem() ? "success" : "fail");
}

_Bool testInit () {
	int i;
	for (i = 0; i<127; i++) {
		if(device_list[i].sensor_id != 0) {
			return (_Bool)0;
		}
	}
	return (_Bool)1;
}

_Bool testAddItem () {
	sensor_descriptor_t sensor = (sensor_descriptor_t){testFunc, testFunc, "test",42,3};
	sensor_descriptor_t sensor2 = (sensor_descriptor_t){testFunc, testFunc, "test",42,3};
	add_list_item(sensor);
	add_list_item(sensor2);
	if (device_list[0].dev_id==42 && device_list[1].dev_id==42) {
		return (_Bool)1;
	} else {
		return (_Bool)0;
	}
}

_Bool testDeleteItem () {
	del_list_items(42);
	if (device_list[0].dev_id==0 && device_list[1].dev_id==0) {
		return (_Bool)1;
	}
	return (_Bool)0;
}

_Bool testReadWriteInvoke () {

};
