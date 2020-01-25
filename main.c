/*
 * GPIO.c
 *
 * Created: 1/17/2020 2:10:14 PM
 * Author : Az
 */ 
#include "registers.h"
#include "gpio.h"
#include "softwareDelay.h"
#include "led.h"
#include "pushButton.h"
#include "std_types.h"
#include "sevenSeg.h"
#include "timers.h"

void gpioReq7(void);
void gpioReq8(void);
void gpioReq9(void);
void test(void);


int main(void)
{ 
	/* gpioReq7();*/ 
	/* gpioReq8(); */
	/* gpioReq9(); */
	test();
	
}

void test(void)
{
	/*initialize led*/
	Led_Init(LED_1);
   /* intialize timer 0*/
   //timer0Init(T0_NORMAL_MODE,T0_OC0_CLEAR,T0_PRESCALER_NO,0,0,T0_POLLING);	/* Normal Mode & Polling*/
   //timer0Init(T0_NORMAL_MODE,T0_OC0_CLEAR,T0_PRESCALER_NO,0,0,T0_INTERRUPT_NORMAL);	/* Normal Mode & Interrupt-normal*/
   //timer0Init(T0_COMP_MODE,T0_OC0_CLEAR,T0_PRESCALER_NO,0,0,T0_POLLING);	/* Compare Mode & Polling*/
   timer0Init(T0_COMP_MODE,T0_OC0_CLEAR,T0_PRESCALER_NO,0,0,T0_INTERRUPT_CMP);	/* Compare Mode & Interrupt-compare*/
   
	while(1)
	{
		/*Go state*/
		Led_On(LED_1);
		timer0DelayMs(10); //delay 1000 ms
		Led_Off(LED_1);	
		timer0DelayMs(10); //delay 1000 ms	
	}
}


void gpioReq7(void)
{
	/*initialize segments*/
	sevenSegInit(SEG_0);
	sevenSegInit(SEG_1);
	while(1)
	{
		uint8_t counter = 0;
		for(;counter < 100;counter++)
		{
			sevenSegWrite(SEG_0,(counter % 10));
			softwareDelayMs(10);
			sevenSegWrite(SEG_1,(counter / 10));
			softwareDelayMs(10);
		}
	}	
}

void gpioReq9(void)
{
	/*initialize led*/
	Led_Init(LED_1);
	Led_Init(LED_2);
	Led_Init(LED_3);
	while(1)
	{
		/*Go state*/
		Led_On(LED_1);
		softwareDelayMs(1000);		
		Led_Off(LED_1);
		/*Stop state*/
		Led_On(LED_3);
		softwareDelayMs(1000);
		Led_Off(LED_3);
		/*Get Ready state*/
		Led_On(LED_2);
		softwareDelayMs(1000);
		Led_Off(LED_2);		
	}	
}

void gpioReq8(void)
{
	uint8_t u8_press_counter = 0;
	uint16_t u16_time_delay_acc = 0; /* time delay accumulator*/
	uint8_t u8_polling_delay = 10; /*time delay (in Milli seconds) value for polling switch button status*/
	//initialize led_1 and btn_1 (sets direction)
	Led_Init(LED_1);
	pushButtonInit(BTN_1);
	while (1)
	{
		/*checking switch state*/
		if(pushButtonGetStatus(BTN_1) == Pressed)
		{
			/*increment u8_press_counter*/
			u8_press_counter++;
			/*check u16_time_delay_acc*/
			if(u16_time_delay_acc > 1000)
			{
				/*reset u8_press_counter*/
				u8_press_counter = 0;
				/*reset u16_time_delay_acc*/
				u16_time_delay_acc = 0;
			}
			Led_On(LED_1);
			softwareDelayMs(1000);
			if(u8_press_counter == 2 && u16_time_delay_acc <= 1000) softwareDelayMs(1000);
			Led_Off(LED_1);
		}
		/*delay and add to accumulator*/
		softwareDelayMs(u8_polling_delay);
		u16_time_delay_acc += u8_polling_delay;
	}	
}

