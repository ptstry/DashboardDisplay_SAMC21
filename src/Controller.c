/*
 * Controller.c
 *
 * Created: 02/01/2018 16:52:48
 *  Author: Piotr
 */ 

#include "Controller.h"
#include "pinout.h"
#include <asf.h>

void configure_out(void);
void configure_tc(void);
void configure_tc2(void);
void configure_tc2_callback(void);
void toggle_HSYNC(struct tc_module *const module_inst);
void toggle_DATA(struct tc_module *const module_inst);
void write_DATA(struct tc_module *const module_inst);
void counter(struct tc_module *const module_inst);



void configure_out(void) {
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	
	pin_conf.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(DE_PIN, &pin_conf);
	port_pin_set_config(HSYNC_PIN, &pin_conf);
	port_group_set_config(&PORTA, 0x00FFFFFF, &pin_conf);
}

void configure_tc(void) {
	struct tc_config config_tc;
	
	tc_get_config_defaults(&config_tc);
	
	// [change default config]
	config_tc.clock_source = GCLK_GENERATOR_1;
	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV1;
	config_tc.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;
	config_tc.counter_8_bit.period = 2;
	config_tc.counter_8_bit.compare_capture_channel[0] = 1;
	// [change default config]
	
	// [setup PWM]
	config_tc.pwm_channel[0].enabled = true;
	config_tc.pwm_channel[0].pin_out = DCLK_OUT_PIN;
	config_tc.pwm_channel[0].pin_mux = DCLK_OUT_MUX;
	// [setup PWM]
	
	tc_init(&tc_instance_0, PWM_MODULE, &config_tc);
	//tc_enable(&tc_instance_0);
}

void configure_tc2(void) {
	struct tc_config config_tc_2;
	tc_get_config_defaults(&config_tc_2);
	
	// [change default config]
	config_tc_2.clock_source = GCLK_GENERATOR_1;
	config_tc_2.clock_prescaler = TC_CLOCK_PRESCALER_DIV2; // 512DCLK width will be 256 (8bit)
	config_tc_2.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc_2.counter_8_bit.period = 255;
	config_tc_2.counter_8_bit.compare_capture_channel[0] = 1; // Thw = 2 DCLK
	config_tc_2.counter_8_bit.compare_capture_channel[1] = 15; // Thbp = 30 DCLK
	
	tc_init(&tc_instance_1, TC1, &config_tc_2);
	//tc_enable(&tc_instance_1);
}

void configure_tc2_callback(void) {
	tc_register_callback(&tc_instance_1, toggle_HSYNC, TC_CALLBACK_CC_CHANNEL0);
	tc_register_callback(&tc_instance_1, toggle_DATA, TC_CALLBACK_CC_CHANNEL1);
	tc_register_callback(&tc_instance_1, toggle_HSYNC, TC_CALLBACK_OVERFLOW);
	tc_register_callback(&tc_instance_0, counter, TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&tc_instance_0, TC_CALLBACK_CC_CHANNEL0);
	
	tc_enable_callback(&tc_instance_1, TC_CALLBACK_CC_CHANNEL0);
	//tc_enable_callback(&tc_instance_1, TC_CALLBACK_CC_CHANNEL1);
	tc_enable_callback(&tc_instance_1, TC_CALLBACK_OVERFLOW);
}

void toggle_DATA(struct tc_module *const module_inst) {
	port_pin_toggle_output_level(DE_PIN); // Data Enable Pin
	// [enable data write interrupts on tc_instance_0]
	tc_register_callback(&tc_instance_0, write_DATA, TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&tc_instance_0, TC_CALLBACK_CC_CHANNEL0);
}
void toggle_HSYNC(struct tc_module *const module_inst) {
	tc_1_val = tc_get_count_value(&tc_instance_1);
	port_pin_toggle_output_level(HSYNC_PIN);
	
}
void write_DATA(struct tc_module *const module_inst) {
	static uint32_t data;
	static uint16_t cnt;
	
	if(cnt==480) {
		port_group_set_output_level(&PORTA, 0, ~0);
		tc_disable_callback(&tc_instance_0, TC_CALLBACK_CC_CHANNEL0);
	}
	
	data += 4;
	uint32_t msk = data | port_group_get_output_level(&PORTA, ~0);
	port_group_set_output_level(&PORTA, msk, ~0);
	
	cnt++;
}
void counter(struct tc_module *const module_inst) {
	tc_1_val = tc_get_count_value(&tc_instance_1);
}

void controller_init(void) {
	configure_out();
	configure_tc();
	configure_tc2();
	configure_tc2_callback();
	tc_enable(&tc_instance_0);
	tc_enable(&tc_instance_1);
}