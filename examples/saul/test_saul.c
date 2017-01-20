#include "test_saul.h"

extern sensact_descriptor_t device_list[127];
extern uint32_t i2c_devices[127];

sensact_rw_result_t testFunc () {
	return (sensact_rw_result_t){0,8.5};
}

void runTests() {
  printf("\nTesting SAM init: %s \n", testInit() ? "success" : "fail");
  printf("Testing SAM add item: %s \n", testAddItem() ? "success" : "fail");
  printf("Testing SAM deleting items: %s \n", testDeleteItem() ? "success" : "fail");
  printf("\nTesting board manager init: %s \n", testBoardManInit() ? "success" : "fail");
  printf("Testing board manager register new dev: %s \n", testBoardManAddItem() ? "success" : "fail");
}

/*=======================
 *  Testing  SAM
 =======================*/
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
	sensact_descriptor_t sensor = (sensact_descriptor_t){testFunc, testFunc, "test",42,3};
	sensact_descriptor_t sensor2 = (sensact_descriptor_t){testFunc, testFunc, "test",42,3};
	sam_add_sensact(sensor);
	sam_add_sensact(sensor2);
	if (device_list[0].dev_id==42 && device_list[1].dev_id==42) {
		return (_Bool)1;
	} else {
		return (_Bool)0;
	}
}

_Bool testDeleteItem () {
	sam_del_device(42);
	if (device_list[0].dev_id==0 && device_list[1].dev_id==0) {
		return (_Bool)1;
	}
	return (_Bool)0;
}

//_Bool testReadWriteInvoke () {
//
//};

/*=======================
 *  Testing  bus_manager
 =======================*/
_Bool testBoardManInit () {
	int i;
	for (i = 0; i<127; i++) {
		if(i2c_devices[i] != 0) {
			return (_Bool)0;
		}
	}
	return (_Bool)1;
}

_Bool testBoardManAddItem () {
	unsigned int a,b,c;
	a = bus_manager_register_i2c_device(123123);
	b = bus_manager_register_i2c_device(124124);
	c = bus_manager_register_i2c_device(123123);

	if (a!=1 || b!=2 || c!=1 ) return (_Bool)0;

	int i =0;
	if (i2c_devices[0]==0 && i2c_devices[3]==0) i++;
	if (i2c_devices[1]==123123) i++;
	if (i2c_devices[2]==124124) i++;

	if(i==3) return (_Bool)1;
	else return (_Bool)0;
};
