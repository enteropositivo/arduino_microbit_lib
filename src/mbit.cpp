#include "arduino.h"
#include "mbit.h"
#include "MMA8653.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"

#include "mbit_utils.h"
#include "mbit_display.h"

//void mbit_timer1_tick();
void mbit_timer2_tick();
void mbit_btns_int_handler();
/*extern "C"  { void TIMER1_IRQHandler(void) { mbit_timer1_tick();     }}*/
extern "C"  { void TIMER2_IRQHandler(void) { mbit_timer2_tick();     }}
extern "C"  { void GPIOTE_IRQHandler(void) { mbit_btns_int_handler(); }}


uint32_t tick_1s = millis();
uint32_t tick_200 = millis();
uint32_t tick_accel = millis();
uint8_t p_temperature = 0;

uint32_t tone_start=0;
uint16_t tone_duration=0;


void mbit_light_level_check();
 

typedef struct {
        uint8_t id;
        void (*event)();
} mbit_event;

mbit_event* events_list = NULL;
uint8_t events_count =0; 

bool mbit_event_exists( uint8_t event_id ){
    for(uint8_t e =0; e< events_count; e++){
        if( events_list[e].id == event_id ) return true;
    }
    return false;    
} 

void mbit_event_fire( uint8_t event_id ){
    for(uint8_t e =0; e< events_count; e++){
        if( events_list[e].id == event_id ){
               events_list[e].event();  
               return;
        } 
    }
    
} 



volatile uint8_t _btn_A = 0;
volatile uint8_t _btn_B = 0;
volatile uint8_t _pinmode_btn_A = 0;
volatile uint8_t _pinmode_btn_B = 0;

volatile uint8_t _pin_P0 = 0;
volatile uint8_t _pin_P1 = 0;
volatile uint8_t _pin_P2 = 0;
volatile uint8_t _pinmode_pin_P0 = 0;
volatile uint8_t _pinmode_pin_P1 = 0;
volatile uint8_t _pinmode_pin_P2 = 0;


void mbit_check_btns(bool fire_event=false){

   if( _pinmode_btn_A && !digitalRead(PIN_BTN_A) && _btn_A==0 ){
      _btn_A=1; 
      if(fire_event) mbit_event_fire( WHENBUTTON_A_PRESSED_CALLBACK ); 
   }else if( _pinmode_btn_A && digitalRead(PIN_BTN_A) && _btn_A==1 ){
      _btn_A=0; 
      if(fire_event) mbit_event_fire( WHENBUTTON_A_RELEASED_CALLBACK ); 
   }

   if( _pinmode_btn_B &&!digitalRead(PIN_BTN_B) && _btn_B==0 ){ 
     _btn_B=1; 
     if(fire_event) mbit_event_fire( WHENBUTTON_B_PRESSED_CALLBACK ); 
   }else if( _pinmode_btn_B &&digitalRead(PIN_BTN_B) && _btn_B==1 ){ 
     _btn_B=0; 
     if(fire_event) mbit_event_fire( WHENBUTTON_B_RELEASED_CALLBACK ); 
   }

}
void mbit_btns_int_handler(){


    if ((NRF_GPIOTE->EVENTS_IN[ INTERRUPT_BTN_A ] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN2_Msk)) {
         NRF_GPIOTE->EVENTS_IN[ INTERRUPT_BTN_A ] = 0;
        // NVIC_ClearPendingIRQ(GPIOTE_IRQn);
        // if(!digitalRead(PIN_BTN_A) && _btn_A==0 ){ _btn_A=1; mbit_event_fire( WHENBUTTON_A_PRESSED_CALLBACK ); }
        // if( digitalRead(PIN_BTN_A) && _btn_A==1 ){ _btn_A=0; mbit_event_fire( WHENBUTTON_A_RELEASED_CALLBACK ); }
        mbit_check_btns(true);
    }

    if ((NRF_GPIOTE->EVENTS_IN[ INTERRUPT_BTN_B ] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN1_Msk)) {
         NRF_GPIOTE->EVENTS_IN[ INTERRUPT_BTN_B ] = 0;
       //  NVIC_ClearPendingIRQ(GPIOTE_IRQn);
        
        // if(!digitalRead(PIN_BTN_B) && _btn_B==0 ){ _btn_B=1; mbit_event_fire( WHENBUTTON_B_PRESSED_CALLBACK ); }
        // if( digitalRead(PIN_BTN_B) && _btn_B==1 ){ _btn_B=0; mbit_event_fire( WHENBUTTON_B_RELEASED_CALLBACK ); }

         mbit_check_btns(true);
           
    }

   
}

void mbit_set_pin_interrupt( uint8_t pin){


    uint32_t btn = g_ADigitalPinMap[pin];

    uint8_t idx =0;
    uint32_t int_config =0;

    if( pin == PIN_BTN_A ){
        idx =INTERRUPT_BTN_A; //2
        int_config = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN2_Pos; 
        _pinmode_btn_A = 1;       
    }
    if( pin == PIN_BTN_B ){
        idx =INTERRUPT_BTN_B; //1
        int_config = GPIOTE_INTENSET_IN1_Set << GPIOTE_INTENSET_IN1_Pos; 
        _pinmode_btn_B = 1;       
    }
       
    
    if(NRF_GPIOTE->CONFIG[idx]!=0) return; //- Interrupt already setted

    NVIC_DisableIRQ(GPIOTE_IRQn);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);

   
     
 //pinMode( pin, INPUT_PULLUP);
    NRF_GPIOTE->CONFIG[idx] =  ( GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos)
                          | (btn << GPIOTE_CONFIG_PSEL_Pos) 
                          | (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);

    NRF_GPIOTE->INTENSET = int_config; //-
     NVIC_SetPriority(GPIOTE_IRQn, 3); //optional: set priority of interrupt
    NVIC_EnableIRQ(GPIOTE_IRQn);
  
}


void mbit_check_pins(){

    if( _pinmode_pin_P0 && !digitalRead(PIN_P0)==0 && _pin_P0==0 ){ _pin_P0 = 1; mbit_event_fire( WHENPIN_P0_PRESSED_CALLBACK );  }
    if( _pinmode_pin_P0 && digitalRead(PIN_P0)==1 && _pin_P0==1 ){ _pin_P0=0;   mbit_event_fire( WHENPIN_P0_RELEASED_CALLBACK ); }

    if( _pinmode_pin_P1 && !digitalRead(PIN_P1) && _pin_P1==0 ){ _pin_P1 = 1; mbit_event_fire( WHENPIN_P1_PRESSED_CALLBACK ); }
    if( _pinmode_pin_P1 && digitalRead(PIN_P1) && _pin_P1==1 ){ _pin_P1=0;   mbit_event_fire( WHENPIN_P1_RELEASED_CALLBACK ); }

    if( _pinmode_pin_P2 && !digitalRead(PIN_P2) && _pin_P2==0 ){ _pin_P2 = 1; mbit_event_fire( WHENPIN_P2_PRESSED_CALLBACK ); }
    if( _pinmode_pin_P2 && digitalRead(PIN_P2) && _pin_P2==1 ){ _pin_P2=0;   mbit_event_fire( WHENPIN_P2_RELEASED_CALLBACK ); }

}


bool mbit_A_pressed(){
   if( _pinmode_btn_A == 0){
     pinMode(PIN_BTN_A, INPUT_PULLUP);
     _pinmode_btn_A = 1; 
     mbit_check_btns();
    
  }
   return  _btn_A==1; 
}
bool mbit_B_pressed(){
  if( _pinmode_btn_B == 0){
     pinMode(PIN_BTN_B, INPUT_PULLUP);
     _pinmode_btn_B = 1; 
     mbit_check_btns();
    
  }
   return  _btn_B==1; 
}
bool mbit_P0_pressed(){
  if( _pinmode_pin_P0 == 0){
     pinMode(PIN_P0, INPUT);
     _pinmode_pin_P0 = 1; 
     mbit_check_pins();
  }
  return  _pin_P0==1; 
}
bool mbit_P1_pressed(){
   if( _pinmode_pin_P1 == 0){
     pinMode(PIN_P1, INPUT);
     _pinmode_pin_P1 = 1; 
     mbit_check_pins();
  }
   return  _pin_P1==1; 
}
bool mbit_P2_pressed(){
   if( _pinmode_pin_P2 == 0){
     pinMode(PIN_P2, INPUT);
     _pinmode_pin_P2 = 1; 
     mbit_check_pins();
  }
   return  _pin_P2==1; 
}

void mbit_on( uint8_t event,   void (*in_main_func)() ) {

    if(mbit_event_exists(event)) return;

    //- AB Buttons
    if( event == WHENBUTTON_A_PRESSED_CALLBACK || event == WHENBUTTON_A_RELEASED_CALLBACK )   mbit_set_pin_interrupt(PIN_BTN_A);
    if( event == WHENBUTTON_B_PRESSED_CALLBACK || event == WHENBUTTON_B_RELEASED_CALLBACK )   mbit_set_pin_interrupt(PIN_BTN_B);
    //- Pins
    if( event == WHENPIN_P0_PRESSED_CALLBACK || event == WHENPIN_P0_RELEASED_CALLBACK )  {
          pinMode(PIN_P0, INPUT);
          _pinmode_pin_P0 = 1;
     }    
    if( event == WHENPIN_P1_PRESSED_CALLBACK || event == WHENPIN_P1_RELEASED_CALLBACK ) {
           pinMode(PIN_P1, INPUT);
           _pinmode_pin_P1 = 1;
    }       
    if( event == WHENPIN_P2_PRESSED_CALLBACK || event == WHENPIN_P2_RELEASED_CALLBACK ) {
           pinMode(PIN_P2, INPUT);
           _pinmode_pin_P2 = 1;
    }
    

    //- Add callback to the event list
    if(events_list == NULL ){
      events_list = ( mbit_event* ) calloc( ++events_count , sizeof(mbit_event));
    }else{
       events_list = ( mbit_event* ) realloc( events_list , ++events_count*sizeof(mbit_event)  );
    }
     
    events_list[events_count-1].id = event;
    events_list[events_count-1].event = in_main_func;
}

void mbit_timer2_tick(){
   if ((NRF_TIMER2->EVENTS_COMPARE[0] != 0) && ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0))
  {
    
    NRF_TIMER2->EVENTS_COMPARE[0] = 0;   
    //----------------------
    display.renderRow();

    // TONE
    if( tone_start && ((millis() - tone_start ) > tone_duration) ){
      tone_start=0; 
      TONE_TIMER->TASKS_STOP = 1;
    }

    //----------------------
    NRF_TIMER2->CC[0] += 1000;      
  }
  

}

/*void mbit_timer1_tick(){
  if (NRF_TIMER1->EVENTS_COMPARE[0] != 0)
  {
         

     NRF_TIMER1->CC[0] += 100 * 1000;; 
    //----------------------
    //Serial.print("o ");
    mbit_check_pins();
    //----------------------
    NRF_TIMER1->EVENTS_COMPARE[0] = 0;    
    
  }
 
  

}*/


void mbit_start_timer2(void)
{    
  NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;              // Set the timer in Counter Mode
  NRF_TIMER2->TASKS_CLEAR = 1;                           // clear the task first to be usable for later
  NRF_TIMER2->PRESCALER   = 4;  
  NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_16Bit;     //Set counter to 16 bit resolution
  NRF_TIMER2->CC[0] = 1000;                               //Set value for TIMER2 compare register 0
  NRF_TIMER2->CC[1] = 0;                                  //Set value for TIMER2 compare register 1
    
  // Enable interrupt on Timer 2, both for CC[0] and CC[1] compare match events
  NRF_TIMER2->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos) ;
  NVIC_SetPriority(TIMER2_IRQn, 1);
  NVIC_EnableIRQ(TIMER2_IRQn);
    
  NRF_TIMER2->TASKS_START = 1;               // Start TIMER2
}

void mbit_start_timer1(void)
{   

    NRF_TIMER1->TASKS_STOP = 1;
    NRF_TIMER1->MODE        = TIMER_MODE_MODE_Timer;
    NRF_TIMER1->BITMODE     =  (TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos);
    NRF_TIMER1->PRESCALER   = 4;  

    NRF_TIMER1->TASKS_CLEAR = 1;               // clear the task first to be usable for later
    NRF_TIMER1->CC[0] = 100 * 1000; //first timeout
    NRF_TIMER1->CC[1] = 0;   //second timeout

    NRF_TIMER1->INTENSET    = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
   // NRF_TIMER1->INTENSET    = TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENSET_COMPARE1_Pos;
    /* Create an Event-Task shortcut to clear TIMER1 on COMPARE[0] event. */
    //NRF_TIMER1->SHORTS      = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos)     ;
    NRF_TIMER1->TASKS_START = 1;
  

  //NRF_TIMER1->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos) ;
  //NVIC_SetPriority(TIMER1_IRQn, 1);
  NVIC_EnableIRQ(TIMER1_IRQn);
    
  NRF_TIMER1->TASKS_START = 1;               // Start TIMER2
}










void mbit_start(){
  display.begin();
 
  mbit_start_timer2();
 // mbit_start_timer1();


}

uint32_t mbit_id()
{
    return NRF_FICR->DEVICEID[0]/2+ NRF_FICR->DEVICEID[1]/2;
}

void mbit_number(int32_t x){
  display.showNumber(x);
}

void mbit_clear(){
  display.clearScreen();
}

void mbit_draw(const String text) {
 display.showLeds(text);
}


void mbit_draw_array( uint8_t *pixels){

  display.clearScreen();
  for( int r=0; r<5;r++){
    for( int c=0; c<5;c++){
      if(  bitRead(pixels[r],5-c-1 ) )  display.plot(c,r); 
    }
  }

} 

void mbit_text(const String text) {
 display.showString(text);
}

void mbit_stop_animation() {
 display.stopAnimation();
}

void mbit_number_100k(uint32_t num){

    if(num>99999) return;
    display.clearScreen();
    uint8_t digit_pos=1;
    uint8_t digit=0;

    while(num>0){
      digit =num%10;
      num = num/10;
      if(digit>5){
        mbit_led_on(5-digit_pos,0);
        digit-=5;
      }
      if(digit>0)mbit_led_on(5-digit_pos,5-digit);
      digit_pos++;
    }  
}


bool mbit_led_get(uint8_t x,   uint8_t y){
    return display.point(x, y);
}

void mbit_led_set( uint8_t x,   uint8_t y , bool onoff) {
    if(onoff==true){
        display.plot(x, y);
    }else{
        display.unplot(x, y);
    }
}

void mbit_led_on( uint8_t x,   uint8_t y ) {
	display.plot(x, y);
}
void mbit_led_off( uint8_t x,   uint8_t y ) {
	display.unplot(x, y);
}

void mbit_led_toggle( uint8_t x,   uint8_t y ) {
   
   mbit_led_set( x, y, !mbit_led_get(x, y) );
}


void  mbit_led_enable(bool onoff ){
    display.enable(onoff);
}

MMA8653 accel;
int8_t accel_x=0, accel_y=0, accel_z=0;
uint8_t accel_started = 0;
uint8_t accel_gesture=WHENGESTURE_NONE;
uint8_t accel_gesture_current=WHENGESTURE_NONE; // candidate to become accel_gesture
uint8_t accel_gesture_count=0;
uint8_t accel_3Ging=0, accel_6Ging=0, accel_8Ging=0;



void mbit_accel_start(){

	//Wire.begin();
  accel.begin(false, 2); 

  accel_started = 1;
}

uint8_t mbit_gesture(){
	return accel_gesture;

}


void mbit_raise_event_gesture(){

    if( accel_gesture == WHENGESTURE_TILT_RIGHT_CALLBACK )  mbit_event_fire( WHENGESTURE_TILT_RIGHT_CALLBACK );  
    if( accel_gesture == WHENGESTURE_FACE_DOWN_CALLBACK  )  mbit_event_fire( WHENGESTURE_FACE_DOWN_CALLBACK  );  
    if( accel_gesture == WHENGESTURE_FREE_FALL_CALLBACK  )  mbit_event_fire( WHENGESTURE_FREE_FALL_CALLBACK  );  
    if( accel_gesture == WHENGESTURE_TILT_LEFT_CALLBACK  )  mbit_event_fire( WHENGESTURE_TILT_LEFT_CALLBACK  );  
    if( accel_gesture == WHENGESTURE_TILT_DOWN_CALLBACK  )  mbit_event_fire( WHENGESTURE_TILT_DOWN_CALLBACK  );  
    if( accel_gesture == WHENGESTURE_TILT_UP_CALLBACK    )  mbit_event_fire( WHENGESTURE_TILT_UP_CALLBACK    );  
    if( accel_gesture == WHENGESTURE_FACE_UP_CALLBACK    )  mbit_event_fire( WHENGESTURE_FACE_UP_CALLBACK    );  
    if( accel_gesture == WHENGESTURE_3G_CALLBACK         )  mbit_event_fire( WHENGESTURE_3G_CALLBACK         );  
    if( accel_gesture == WHENGESTURE_6G_CALLBACK         )  mbit_event_fire( WHENGESTURE_6G_CALLBACK         );  
    if( accel_gesture == WHENGESTURE_8G_CALLBACK         )  mbit_event_fire( WHENGESTURE_8G_CALLBACK         );  
}




//uint32_t maxf=0;
void mbit_accel_update(){

    if( accel_started==0 ) mbit_accel_start();
   
   if( (millis() - tick_accel) < 50 ) return;
   tick_accel = millis(); 

   
	accel.update();
	accel_x = accel.getX() * 0.0156 * 100;
	accel_y = accel.getY() * 0.0156 * 100;
	accel_z = accel.getZ() * 0.0156 * 100;    


    uint32_t force = (uint32_t)accel_x*(uint32_t)accel_x + (uint32_t)accel_y*(uint32_t)accel_y + (uint32_t)accel_z*(uint32_t)accel_z;

//Serial.println(force);

  //  if(force>maxf) maxf = force;

//Serial.print(maxf);Serial.println("-");
//Serial.print(maxf);Serial.print("-");
/*Serial.print(accel_x); Serial.print("-"); 
Serial.print(accel_y); Serial.print("-"); 
Serial.println(accel_z); */


	uint8_t instagest = WHENGESTURE_NONE;

    //-increment xG counters and reset it for new checking
    if(accel_3Ging && ++accel_3Ging>5) accel_3Ging=accel_6Ging=accel_8Ging=0;
    if(accel_6Ging && ++accel_6Ging>5) accel_3Ging=accel_6Ging=accel_8Ging=0;
    if(accel_8Ging && ++accel_8Ging>5) accel_3Ging=accel_6Ging=accel_8Ging=0;
    
    if(accel_3Ging==0 && force> 135*135 ){
        accel_3Ging=1; // start xG counter
        accel_gesture = WHENGESTURE_3G_CALLBACK;
       // Serial.println("3G");
    }
    if(accel_6Ging==0 && force> 180*180 ){
        accel_6Ging=1; // start xG counter
        accel_gesture = WHENGESTURE_6G_CALLBACK;
    }
    if(accel_8Ging==0 && force> 200*200 ){
        accel_8Ging=1; // start xG counter
        accel_gesture = WHENGESTURE_8G_CALLBACK;
    }

    //- raise xG events
    if(accel_3Ging==1 || accel_6Ging==1 || accel_8Ging==1 ) return mbit_raise_event_gesture();
        
   //   Serial.println("x3"); 

	
    if(force< 40*40){
        instagest = WHENGESTURE_FREE_FALL_CALLBACK;
    }else if( accel_x < -80 ){ 
		instagest = WHENGESTURE_TILT_RIGHT_CALLBACK;
	}else if (accel_x > 80 ){
		instagest = WHENGESTURE_TILT_LEFT_CALLBACK;
	}else if( accel_y < -80 ){ 
		instagest = WHENGESTURE_TILT_DOWN_CALLBACK;
	}else if (accel_y > 80 ){
		instagest = WHENGESTURE_TILT_UP_CALLBACK;
	}else if( accel_z < -80 ){ 
		instagest = WHENGESTURE_FACE_UP_CALLBACK;
	}else if (accel_z > 80 ){
		instagest = WHENGESTURE_FACE_DOWN_CALLBACK;
	}else{
		instagest = WHENGESTURE_NONE;
	}	




	//- gesture sampling count
	if (instagest == accel_gesture_current)
    {
         if(accel_gesture_count<2) accel_gesture_count++;
    }
    else
    {
        accel_gesture_current = instagest;
        accel_gesture_count = 0;
    }



    if (accel_gesture_current != accel_gesture && accel_gesture_count >= 2)
    {
        accel_gesture = accel_gesture_current;
        //- AQUI puedo lanzar el evento
        mbit_raise_event_gesture();
    }

  /*  Serial.print("IG:");Serial.println(instagest);
Serial.print("count:");Serial.println(accel_gesture_count);
Serial.print("G-curr:");Serial.println(accel_gesture_current);
Serial.print("Gest:");Serial.println(accel_gesture);*/

}

float mbit_accel_x(){
	return (float)accel_x;
}

float mbit_accel_y(){
	return (float)accel_y;
}

float mbit_accel_z(){
	return (float)accel_z;
}

float mbit_accel(const char *coor ){
	if(coor=="x") return mbit_accel_x();
	if(coor=="y") return mbit_accel_x();
	if(coor=="z") return mbit_accel_x();
	return (float)0;
}



void mbit_tone( int freq, int dur){
 
  uint32_t ulPin;
  uint32_t f =TIMERFREQ/freq;
    
  ulPin = g_ADigitalPinMap[ 0 ];   
  NRF_GPIOTE->CONFIG[0] =          
    (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |          
    (ulPin << GPIOTE_CONFIG_PSEL_Pos) |                            
    (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
    (GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos);      
  NRF_GPIOTE->POWER = 1; 
  //pinMode( 0, OUTPUT);                                          
    
  TONE_TIMER->TASKS_STOP = 1;                          
  TONE_TIMER->TASKS_CLEAR = 1;                         
  TONE_TIMER->MODE = TIMER_MODE_MODE_Timer;            
  TONE_TIMER->PRESCALER   = 4;                         
  TONE_TIMER->BITMODE = TIMER_BITMODE_BITMODE_16Bit;   
  TONE_TIMER->CC[0] = f/2;                             

  TONE_TIMER->SHORTS =                                 
      (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);

  
  NRF_PPI->CH[0].TEP  = (uint32_t)&NRF_GPIOTE->TASKS_OUT[0];       
  NRF_PPI->CH[0].EEP  = (uint32_t)&TONE_TIMER->EVENTS_COMPARE[0];  
  NRF_PPI->CHENSET   |= PPI_CHENSET_CH0_Enabled;                   

  TONE_TIMER->TASKS_START = 1;
  
  tone_start=millis();
  tone_duration=dur;
  
}



uint8_t mbit_temp(void) {
 //  obtener la temperatura cada segundo

   //- Timer each 1s
	if( (millis() - tick_1s) > 1000 ){
	 	tick_1s = millis();
	 	 
	   int32_t processorTemperature;
       uint8_t sd_enabled;

        sd_softdevice_is_enabled(&sd_enabled);

        if (sd_enabled)
        {
            // If Bluetooth is enabled, we need to go through the Nordic software to safely do this
            sd_temp_get(&processorTemperature);
        }
        else
        {
            // Othwerwise, we access the information directly...
            uint32_t *TEMP = (uint32_t *)0x4000C508;

            NRF_TEMP->TASKS_START = 1;

            while (NRF_TEMP->EVENTS_DATARDY == 0);

            NRF_TEMP->EVENTS_DATARDY = 0;

            processorTemperature = *TEMP;

            NRF_TEMP->TASKS_STOP = 1;
        }


        // Record our reading...
        p_temperature = processorTemperature / 4;
	}


    return p_temperature;
}



volatile uint32_t light_level = 0;

volatile uint16_t light_smooth[3] = {0,0,0};
volatile uint16_t light_smooth_idx = 0;

//- Self calibrated light-level


void mbit_light_level_check() {

  if( (millis() - tick_200) < 200 ) return;
   tick_200 = millis();

 display.enable(0);
 //------- DISABLE SCREEN --------------
  for (uint8_t r=0; r<3; r++) {
    digitalWrite(rows[r], LOW); 
  }
  for (uint8_t c=0; c<3; c++) {
    digitalWrite(cols[c], HIGH); 
  }


 int v = 0;
  for (int x = 0; x < 3; x++) {
    digitalWrite(cols[x], LOW);
    pinMode(cols[x], INPUT);

    v += mbit_utils_analog_read(cols[x]);
    pinMode(cols[x], OUTPUT);
    digitalWrite(cols[x], HIGH);
  }
 display.enable(1);
 light_smooth[ ++light_smooth_idx%3 ] = v/3 ;
return;





  

  display.enable(0);
 //------- DISABLE SCREEN --------------
  for (uint8_t r=0; r<3; r++) {
    digitalWrite(rows[r], LOW); 
  }
  for (uint8_t c=0; c<3; c++) {
    digitalWrite(cols[c], HIGH); 
  }
  //-------------------------------------



  for (uint8_t c=0; c<3; c++) {
       pinMode( cols[c], INPUT);
  }


  //-- ugly patch but it works --
  for(uint8_t c=0; c<3 ; c++ ){
      uint16_t unused_val= mbit_utils_analog_read(cols[c]) ;
   }
  delay(4);
  //-----------------------------


   volatile uint16_t light_avg=0;
   for(uint8_t c=0; c<3 ; c++ ){
      light_avg +=  mbit_utils_analog_read(cols[c]) ;delay(1);
   }

   for (uint8_t c=0; c<3; c++) {
       pinMode( cols[c], OUTPUT);
   }
   display.enable(1);

   light_avg = light_avg/3;

   light_smooth[ ++light_smooth_idx%5 ] = light_avg>80?  light_avg -80: light_avg ;

  
}


uint16_t mbit_light_level() {
// volatile uint32_t light_avg=0;

    mbit_light_level_check();

   

    light_level = (light_smooth[0]+light_smooth[1]+light_smooth[2] )/3;
 

    light_level = light_level<0? 0:light_level;
    light_level = light_level>170? 170:light_level;

   return map( light_level, 0 ,170, 255,0 ) ;

}