/*
 * Controller.h
 *
 * Created: 02/01/2018 16:50:50
 *  Author: Piotr
 */ 

#include <inttypes.h>

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

void controller_init(void);
volatile uint32_t tc_1_val;

struct tc_module tc_instance_0; // DCLK instance
struct tc_module tc_instance_1; // HSYNC instance



#endif /* CONTROLLER_H_ */