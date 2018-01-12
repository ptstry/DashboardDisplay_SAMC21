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
void configure_tc0(void);
void configure_tc0_callback(void);
void configure_tc1(void);
void configure_tc1_callback(void);
void configure_tc2(void);
void configure_tc2_callback(void);
void enable_LINE(struct tcc_module *const module_inst);
void disable_LINE(struct tcc_module *const module_inst);
void toggle_HSYNC(struct tc_module *const module_inst);
void toggle_DATA(struct tc_module *const module_inst);
void write_DATA(struct tc_module *const module_inst);
void write_DATA_single(void);
void counter(struct tc_module *const module_inst);
void clearer(struct tc_module *const module_inst);


void configure_out(void) {
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	
	pin_conf.direction = PORT_PIN_DIR_OUTPUT;
	pin_conf.input_pull = PORT_PIN_PULL_DOWN;
	port_pin_set_config(DE_PIN, &pin_conf);
	port_pin_set_config(HSYNC_PIN, &pin_conf);
	port_group_set_config(&PORTB, 0x00FFFFFF, &pin_conf);
	port_pin_set_output_level(PIN_PA02, 0);
	port_pin_set_output_level(PIN_PB02, 0);
}

void configure_tc0(void) {
	struct tc_config config_tc_0;
	
	tc_get_config_defaults(&config_tc_0);
	
	// [change default config]
	config_tc_0.clock_source = GCLK_GENERATOR_2;
	config_tc_0.clock_prescaler = TC_CLOCK_PRESCALER_DIV1;
	config_tc_0.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc_0.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;
	//config_tc_0.waveform_invert_output = TC_WAVEFORM_INVERT_OUTPUT_CHANNEL_0;
	config_tc_0.counter_8_bit.period = 1;
	config_tc_0.counter_8_bit.compare_capture_channel[0] = 1;
	//config_tc.counter_8_bit.compare_capture_channel[1] = 2;
	// [change default config]
	
	// [setup PWM]
	config_tc_0.pwm_channel[0].enabled = true;
	config_tc_0.pwm_channel[0].pin_out = DCLK_OUT_PIN;
	config_tc_0.pwm_channel[0].pin_mux = DCLK_OUT_MUX;
	// [setup PWM]
	
	tc_init(&tc_instance_0, PWM_MODULE, &config_tc_0);
	//tc_enable(&tc_instance_0);
}

void configure_tc1(void) {
	struct tc_config config_tc_1;
	tc_get_config_defaults(&config_tc_1);
	
	// [change default config]
	config_tc_1.clock_source = GCLK_GENERATOR_2;
	config_tc_1.clock_prescaler = TC_CLOCK_PRESCALER_DIV4; // 512DCLK width will be 256 (8bit)
	config_tc_1.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc_1.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;
	config_tc_1.waveform_invert_output = TC_WAVEFORM_INVERT_OUTPUT_CHANNEL_0;
	config_tc_1.counter_8_bit.value = 1;
	config_tc_1.counter_8_bit.period = 255;
	config_tc_1.counter_8_bit.compare_capture_channel[0] = 1; // Thw = 2 DCLK
	config_tc_1.counter_8_bit.compare_capture_channel[1] = 10; // Thbp = 30 DCLK
	
	// [setup PWM]
	config_tc_1.pwm_channel[0].enabled = true;
	config_tc_1.pwm_channel[0].pin_out = HSYNC_OUT_PIN;
	config_tc_1.pwm_channel[0].pin_mux = HSYNC_OUT_MUX;
	// [setup PWM]
	
	tc_init(&tc_instance_1, PWM_MODULE_1, &config_tc_1);
	//tc_enable(&tc_instance_1);
}

void configure_tc2(void) {
	
	struct tcc_config config_tc_2;
	tcc_get_config_defaults(&config_tc_2, TCC0);
	
	config_tc_2.counter.clock_source = GCLK_GENERATOR_2;
	config_tc_2.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1024;
	config_tc_2.counter.period = 285; // 286 * 2
	config_tc_2.wave.wave_generation = TCC_WAVE_WAVEGEN_NPWM_Val;
	config_tc_2.compare.match[0] = 285;
	config_tc_2.compare.match[1] = 273; // start passing lines
	config_tc_2.compare.match[2] = 1;
	config_tc_2.wave_ext.invert[0] = false;
	config_tc_2.counter.count = 285;
	config_tc_2.double_buffering_enabled = false;
	config_tc_2.counter.direction = TCC_COUNT_DIRECTION_DOWN;
	
	// [setup PWM]
	config_tc_2.pins.enable_wave_out_pin[0] = true;
	config_tc_2.pins.wave_out_pin[0] = VSYNC_OUT_PIN;
	config_tc_2.pins.wave_out_pin_mux[0] = VSYNC_OUT_MUX;
	// [setup PWM]
	
	tcc_init(&tc_instance_2, PWM_MODULE_2, &config_tc_2);
	
	/*
	struct tc_config config_tc_2;
	tc_get_config_defaults(&config_tc_2);
	
	// [change default config]
	config_tc_2.clock_source = GCLK_GENERATOR_1;
	config_tc_2.clock_prescaler = TC_CLOCK_PRESCALER_DIV1024; // 512DCLK width will be 256 (8bit)
	config_tc_2.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc_2.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;
	config_tc_2.waveform_invert_output = TC_WAVEFORM_INVERT_OUTPUT_CHANNEL_1;
	config_tc_2.counter_8_bit.value = 1; // fix for the initial period
	config_tc_2.counter_8_bit.period = 255;
	//config_tc_2.counter_8_bit.compare_capture_channel[0] = 4; // VSYNC ON
	config_tc_2.counter_8_bit.compare_capture_channel[1] = 1; // start passing lines
	
	// [setup PWM]
	config_tc_2.pwm_channel[0].enabled = true;
	config_tc_2.pwm_channel[0].pin_out = VSYNC_OUT_PIN;
	config_tc_2.pwm_channel[0].pin_mux = VSYNC_OUT_MUX;
	// [setup PWM]
	
	tc_init(&tc_instance_2, PWM_MODULE_2, &config_tc_2);
	*/
}

void configure_tc0_callback() {
	tc_register_callback(&tc_instance_0, counter, TC_CALLBACK_CC_CHANNEL1);
	tc_enable_callback(&tc_instance_0, TC_CALLBACK_CC_CHANNEL1);
	tc_register_callback(&tc_instance_0, clearer, TC_CALLBACK_OVERFLOW);
	tc_enable_callback(&tc_instance_0, TC_CALLBACK_OVERFLOW);
}

void configure_tc1_callback(void) {
	tc_register_callback(&tc_instance_1, toggle_HSYNC, TC_CALLBACK_CC_CHANNEL0);
	tc_register_callback(&tc_instance_1, toggle_DATA, TC_CALLBACK_CC_CHANNEL1);
	tc_register_callback(&tc_instance_1, toggle_HSYNC, TC_CALLBACK_OVERFLOW);
	
	tc_register_callback(&tc_instance_0, write_DATA, TC_CALLBACK_CC_CHANNEL0);
	
	//tc_enable_callback(&tc_instance_1, TC_CALLBACK_CC_CHANNEL1);
}
void configure_tc2_callback() {
	tcc_register_callback(&tc_instance_2, enable_LINE, TCC_CALLBACK_CHANNEL_1);
	tcc_register_callback(&tc_instance_2, disable_LINE, TCC_CALLBACK_CHANNEL_2);
	tcc_enable_callback(&tc_instance_2, TCC_CALLBACK_CHANNEL_1);
	tcc_enable_callback(&tc_instance_2, TCC_CALLBACK_CHANNEL_2);
}

void enable_LINE(struct tcc_module *const module_inst) {
	// TODO: enable toggle_DATA interrupt
	tc_enable_callback(&tc_instance_1, TC_CALLBACK_CC_CHANNEL1);
}
void disable_LINE(struct tcc_module *const module_inst) {
	tc_disable_callback(&tc_instance_1, TC_CALLBACK_CC_CHANNEL1);
}
void toggle_DATA(struct tc_module *const module_inst) {
	port_pin_toggle_output_level(DE_PIN); // Data Enable Pin
	//tc_1_val = tc_get_count_value(&tc_instance_1);
	// [enable data write interrupts on tc_instance_0]
	//tc_enable_callback(&tc_instance_0, TC_CALLBACK_CC_CHANNEL0);
	write_DATA_single();
	port_pin_toggle_output_level(DE_PIN);
}
void toggle_HSYNC(struct tc_module *const module_inst) {
	//tc_1_val = tc_get_count_value(&tc_instance_1);
	port_pin_toggle_output_level(HSYNC_PIN);
	
}
void write_DATA(struct tc_module *const module_inst) {
	static uint32_t data;
	static uint16_t cnt;
	//tc_1_val = tc_get_count_value(&tc_instance_1);
	
	if(cnt==480) {
		port_group_set_output_level(&PORTA, ~0, 0); // all pins in PORTA set to 0
		tc_disable_callback(&tc_instance_0, TC_CALLBACK_CC_CHANNEL0);
		cnt = 0;
		//data = 0;
		data = data << 1;
		return;
	}
	
	data += 4;
	uint32_t msk = data | port_group_get_output_level(&PORTA, ~0);
	port_group_set_output_level(&PORTA, msk, ~0);
	
	cnt++;
}
void write_DATA_single() {
	uint32_t data = 0;
	uint32_t msk = 0;
	
	for(int i=0; i<480; i++) {
		data += 1; // ~edit fixed data value
		msk = data;;// | port_group_get_output_level(&PORTB, ~0);
		port_group_set_output_level(&PORTB, msk, ~0);
		//delay_cycles(1); // 2 cycles out of 24MHz = 12MHz
	}
	port_group_set_output_level(&PORTA, ~0, 0); // all pins in PORTA set to 0
}
void counter(struct tc_module *const module_inst) {
	tc_1_val = tc_get_count_value(&tc_instance_0);
	port_pin_set_output_level(PIN_PA02, 1);
}
void clearer(struct tc_module *const module_inst) {
	port_pin_set_output_level(PIN_PA02, 0);
}

void controller_init(void) {
	configure_out();
	configure_tc0();
	configure_tc1();
	configure_tc1_callback();
	configure_tc2();
	configure_tc2_callback();
	
	tc_enable2(&tc_instance_0, &tc_instance_1);
	tcc_enable(&tc_instance_2);
}