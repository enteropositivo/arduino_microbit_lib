/*
version: 1.0.2
*/


#ifndef mbit_h
#define mbit_h

#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"

#define PIN_P0		0
#define PIN_P1		1
#define PIN_P2		2
#define PIN_P3		3
#define PIN_P4		4
#define PIN_P5		5
#define PIN_P6		6
#define PIN_P7		7
#define PIN_P8		8
#define PIN_P9		9
#define PIN_P10		10
#define PIN_P11		11
#define PIN_P12		12
#define PIN_P13		13
#define PIN_P14		14
#define PIN_P15		15
#define PIN_P16		16
#define PIN_P17		17
#define PIN_P18		18
#define PIN_P18		19
#define PIN_P20		20

#define PIN_BTN_A	5
#define PIN_BTN_B	11

#define INTERRUPT_BTN_A 2  //- 0 OWNS to buzzer pin0
#define INTERRUPT_BTN_B 1

#define TIMERFREQ 1000000L   
#define TONE_TIMER NRF_TIMER0


#define WHENBUTTON_A_PRESSED_CALLBACK  10
#define WHENBUTTON_A_RELEASED_CALLBACK 11
#define WHENBUTTON_B_PRESSED_CALLBACK  12
#define WHENBUTTON_B_RELEASED_CALLBACK 13
extern void 	mbit_check_buttons();

#define WHENPIN_P0_PRESSED_CALLBACK  20
#define WHENPIN_P0_RELEASED_CALLBACK 21
#define WHENPIN_P1_PRESSED_CALLBACK  22
#define WHENPIN_P1_RELEASED_CALLBACK 23
#define WHENPIN_P2_PRESSED_CALLBACK  24
#define WHENPIN_P2_RELEASED_CALLBACK 25
extern void 	mbit_check_pins();

extern void 	mbit_start();
extern void     mbit_check_pins();
extern uint32_t mbit_id();
extern void 	mbit_on( uint8_t event,   void (*callback_fn)(uint8_t) ) ;
extern void 	mbit_clear();
extern void		mbit_led_enable(bool display_onoff );
extern bool		mbit_led_get(uint8_t x,   uint8_t y);
extern void 	mbit_led_set( uint8_t x,   uint8_t y, bool onoff ) ;
extern void 	mbit_led_on( uint8_t x,   uint8_t y ) ;
extern void 	mbit_led_off( uint8_t x,   uint8_t y ) ;
extern void 	mbit_led_toggle( uint8_t x,   uint8_t y ) ;
extern void 	mbit_number(int32_t x);
extern void 	mbit_number_100k(uint32_t num);
extern void 	mbit_draw( const String text ) ;
extern void 	mbit_draw_array( uint8_t* pixels ) ;
extern void 	mbit_text( const String text ) ;
extern void 	mbit_stop_animation( ) ;

extern bool     mbit_A_pressed();
extern bool     mbit_B_pressed();
extern bool     mbit_P0_pressed();
extern bool     mbit_P1_pressed();
extern bool     mbit_P2_pressed();


extern uint8_t 	mbit_temp(void);
extern uint16_t mbit_light_level(void);
extern void     tone(uint8_t  pin, int freq, unsigned long dur);
extern void     noTone(uint8_t _pin);

/*
¿?
para un if( mbit_btn("A", "pressed"))
*/


extern void 	mbit_accel_update();
extern float 	mbit_accel(const char *coor );
extern float 	mbit_accel_x();
extern float 	mbit_accel_y();
extern float 	mbit_accel_z();
extern uint8_t 	mbit_gesture();





#define WHENGESTURE_NONE 				100
#define WHENGESTURE_TILT_RIGHT_CALLBACK 101
#define WHENGESTURE_TILT_LEFT_CALLBACK 	102
#define WHENGESTURE_TILT_DOWN_CALLBACK 	103
#define WHENGESTURE_TILT_UP_CALLBACK 	104
#define WHENGESTURE_FACE_UP_CALLBACK 	105
#define WHENGESTURE_FACE_DOWN_CALLBACK 	106
#define WHENGESTURE_FREE_FALL_CALLBACK 	107
#define WHENGESTURE_3G_CALLBACK 		108
#define WHENGESTURE_6G_CALLBACK 		109
#define WHENGESTURE_8G_CALLBACK 		110


#endif