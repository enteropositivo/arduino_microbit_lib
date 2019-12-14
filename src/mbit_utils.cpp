
#include "mbit_utils.h"
#include "nrf.h"
#include "nrf_sdm.h"




uint16_t mbit_utils_analog_int(uint32_t ulPin ){
  
   uint32_t pin = ADC_CONFIG_PSEL_Disabled;


    ulPin = g_ADigitalPinMap[ulPin];

    switch ( ulPin ) {
      case 26:
        pin = ADC_CONFIG_PSEL_AnalogInput0;
        break;

      case 27:
        pin = ADC_CONFIG_PSEL_AnalogInput1;
        break;

      case 1:
        pin = ADC_CONFIG_PSEL_AnalogInput2;
        break;

      case 2:
        pin = ADC_CONFIG_PSEL_AnalogInput3;
        break;

      case 3:
        pin = ADC_CONFIG_PSEL_AnalogInput4;
        break;

      case 4:
        pin = ADC_CONFIG_PSEL_AnalogInput5;
        break;

      case 5:
        pin = ADC_CONFIG_PSEL_AnalogInput6;
        break;

      case 6:
        pin = ADC_CONFIG_PSEL_AnalogInput7;
        break;

    }

  

    volatile uint16_t adc_result = 0;
  
   
    NRF_ADC->INTENSET = 1;   
    
    sd_nvic_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_LOW);  
    sd_nvic_EnableIRQ(ADC_IRQn);

    // config ADC
    NRF_ADC->CONFIG = (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos)                     /* Bits 17..16 : ADC external reference pin selection. */
                    | (pin << ADC_CONFIG_PSEL_Pos)                                                /*!< Use analog input X as analog input. */
                    | (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)                            /*!< Use internal 1.2V bandgap voltage as reference for conversion. */
                    | (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)  /*!< Analog input specified by PSEL with no prescaling used as input for the conversion. */
                    | (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);                               /*!< 10bit ADC resolution. */ 
    
   
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;

    return pin;
    

}


void ADC_IRQHandler(void)
{
    /* Clear dataready event */
  NRF_ADC->EVENTS_END = 0;  

  /* Write ADC result to port 2 */
   // nrf_gpio_port_write(NRF_GPIO_PORT_SELECT_PORT2, NRF_ADC->RESULT); 
   // nrf_gpio_pin_toggle(LED_3);
    
    //Use the STOP task to save current. Workaround for PAN_028 rev1.5 anomaly 1.
  NRF_ADC->TASKS_STOP = 1;
    
    //Release the external crystal
    sd_clock_hfclk_release();

  Serial.println("XXX");
}   


uint16_t mbit_utils_analog_read(uint32_t ulPin ){
  
   uint32_t pin = ADC_CONFIG_PSEL_Disabled;


    ulPin = g_ADigitalPinMap[ulPin];

    switch ( ulPin ) {
      case 26:
        pin = ADC_CONFIG_PSEL_AnalogInput0;
        break;

      case 27:
        pin = ADC_CONFIG_PSEL_AnalogInput1;
        break;

      case 1:
        pin = ADC_CONFIG_PSEL_AnalogInput2;
        break;

      case 2:
        pin = ADC_CONFIG_PSEL_AnalogInput3;
        break;

      case 3:
        pin = ADC_CONFIG_PSEL_AnalogInput4;
        break;

      case 4:
        pin = ADC_CONFIG_PSEL_AnalogInput5;
        break;

      case 5:
        pin = ADC_CONFIG_PSEL_AnalogInput6;
        break;

      case 6:
        pin = ADC_CONFIG_PSEL_AnalogInput7;
        break;

    }

  

    volatile uint16_t adc_result = 0;
  
   
    NRF_ADC->INTENSET = 0;   // disable interrupt
    NRF_ADC->EVENTS_END = 0; 

    // config ADC
    NRF_ADC->CONFIG = (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos)                     /* Bits 17..16 : ADC external reference pin selection. */
                    | (pin << ADC_CONFIG_PSEL_Pos)                                                /*!< Use analog input X as analog input. */
                    | (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)                            /*!< Use internal 1.2V bandgap voltage as reference for conversion. */
                    | (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)  /*!< Analog input specified by PSEL with no prescaling used as input for the conversion. */
                    | (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);                               /*!< 10bit ADC resolution. */ 
    
   
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
    delay(2);
    //nrf_delay_us(40);
    // start ADC conversion
    NRF_ADC->TASKS_START = 1;
  
    // wait for conversion to end
    while (!NRF_ADC->EVENTS_END && NRF_ADC->BUSY){};
    //while(( (NRF_ADC->BUSY & ADC_BUSY_BUSY_Msk)>>ADC_BUSY_BUSY_Pos) == ADC_BUSY_BUSY_Busy  ){};
 //Serial.println(".");   
    NRF_ADC->EVENTS_END = 0;
    
    // save results
    adc_result = NRF_ADC->RESULT;
    
    // stop ADC conversion to save power
    NRF_ADC->TASKS_STOP = 1;

    
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Disabled;    // Enable ADC

    NRF_ADC->CONFIG = (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos) |
                        (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) |
                        (ADC_CONFIG_REFSEL_VBG                       << ADC_CONFIG_REFSEL_Pos) |
                        (ADC_CONFIG_PSEL_Disabled                    << ADC_CONFIG_PSEL_Pos) |
                        (ADC_CONFIG_EXTREFSEL_None                   << ADC_CONFIG_EXTREFSEL_Pos);

      
    return adc_result;


}