#include "timers.h"

typedef struct gstr_timerInit_t{
    uint8_t en_mode;
    uint8_t en_OC0;
    uint8_t en_prescal;
    uint8_t u8_initialValue;
    uint8_t u8_outputCompare;
    uint8_t en_interruptMask;    
}gstr_timerInit_t; 

static gstr_timerInit_t timer0;

volatile uint32_t gu16_sw_counter = 0;

/**
 * Description:
 * @param control
 * @param initialValue
 * @param outputCompare
 * @param interruptMask
 */
void timer0Init(En_timer0Mode_t en_mode,En_timer0OC_t en_OC0,En_timer0perscaler_t en_prescal, uint8_t u8_initialValue, uint8_t u8_outputCompare, En_timer0Interrupt_t en_interruptMask)
{    
    /* Initialize TCCR0 */    
    timer0.en_mode = en_mode;
	/* set en_OC0*/
	timer0.en_OC0 = en_OC0;
    /* Set prescalar */    
    timer0.en_prescal = en_prescal; 
    /* set u8_initialValue */
    timer0.u8_initialValue = u8_initialValue;
    /*set u8_outputCompare */
    timer0.u8_outputCompare = u8_outputCompare; 
    /* set en_interruptMask */
    timer0.en_interruptMask = en_interruptMask;      
}

/**
 * Description:
 * @param value
 */
void timer0Set(uint8_t u8_value)
{
    if(timer0.en_mode == T0_NORMAL_MODE)
    {
        //timer is in normal mode
        TCNT0 |= u8_value;
    }else if (timer0.en_mode == T0_COMP_MODE){        
        //timer is in CTC mode
        TCNT0 = 0x00;
        OCR0 |= u8_value;
    }
}

/**
 * Description:
 * @return
 */
uint8_t timer0Read(void)
{
    return TCNT0;
}

/**
 * Description:
 */
void timer0Start(void)
{
    TCCR0 |= (timer0.en_mode | timer0.en_prescal);
}

/**
 * Description:
 */
void timer0Stop(void)
{
    TCCR0 &= T0_NO_CLOCK;
}

/**
 * Description:
 * @param delay
 */
void timer0DelayMs(uint16_t u16_delay_in_ms)
{
    uint32_t ticks_number = 0;
	 uint8_t t0_tov_initial = 0;  /* overflow initial value*/
    uint8_t t0_oc_initial = 0;   /* output compare initial value*/
    
    /* Intialize the timer*/
    /*timer0Init(T0_COMP_MODE,T0_OC0_CLEAR,T0_PRESCALER_NO,0,0,T0_INTERRUPT_CMP);*/    

    /* calculate tick number in milli seconds */    
    ticks_number = (CPU_F / (timer0.en_prescal * MILLI_SECONDS)) * u16_delay_in_ms;
    /* calculate software_counter value in milli seconds */
    gu16_sw_counter   = ticks_number / T0_OV_VAL;
    if((ticks_number % T0_OV_VAL)) /* if (ticks_number % T0_OV_VAL) != 0*/
    {
         /*increment gu16_sw_counter*/
         gu16_sw_counter++;
         /* set t0_tov_initial value */
         t0_tov_initial = (T0_OV_VAL - (uint8_t)(ticks_number % T0_OV_VAL));
         /* set t0_oc_initial*/
         t0_oc_initial = (uint8_t)(ticks_number % T0_OV_VAL); 
    }else
    {
       /* set t0_tov_initial value */
       t0_tov_initial = 0;
       /* set t0_oc_initial*/
       t0_oc_initial = T0_OCR_MAX;  
    }
    /*
    gu16_sw_counter   = (ticks_number % T0_OV_VAL) == 0? gu16_sw_counter : (gu16_sw_counter + 1);
	 t0_tov_initial   = (ticks_number % T0_OV_VAL) == 0? 0 : (T0_OV_VAL - (uint8_t)(ticks_number % T0_OV_VAL));
    */	
	switch (timer0.en_mode)
	{
		/********************************************** NORMAL MODE CONTROL **********************************************/
		case T0_NORMAL_MODE:
		   /* set timer0 : set TOV initial value*/
		   timer0Set(t0_tov_initial);
		   switch(timer0.en_interruptMask)
		   {
			   case T0_POLLING:	;/*this is an empty statment*/		   
				   /* polling mode */
				   uint16_t i = 0;
				   /* start the timer*/
				   timer0Start();
				   while(i < gu16_sw_counter)
				   {
                  i++;                  
					   while(1)
					   {
						   if((TIFR & 0x01))
						   {
							   /*reset TOV0*/
							   TIFR |= 0x01;
							   /*rest TCNT*/
							   //TCNT0 = 0;  /* useless */
							   /*break while loop*/
							   break;
						   };
					   }                  
				   }
				   /* stop the timer*/
				   timer0Stop();			   
			   break;
			   case T0_INTERRUPT_NORMAL:
			      /* set Global interrupt bit*/
			      sei();               
			      /* set TOIE0 */
			      TIMSK |= timer0.en_interruptMask;
               /* start the timer*/
			      timer0Start();
               /* wait for the timer to finish*/
               while(1)
               {
                  if(gu16_sw_counter == 0) break;
               }				  
			      /* stop the timer*/
			      timer0Stop();
               /* reset TOIE0 */
			      TIMSK &= ~(timer0.en_interruptMask);
               /* clear Global interrupt bit*/
			      cli();
			   break;		 
		   }
		break;
		/********************************************** OUTPUT COMPARE MODE CONTROL **********************************************/
		case T0_COMP_MODE:
		   /*set timer0 : set OC initial value*/
		   timer0Set(t0_oc_initial);
		   switch(timer0.en_interruptMask)
		   {
			   case T0_POLLING: ; /* this is an empty statment*/
			      /* polling mode */
				     uint16_t i = 0;  
			      /* start the timer*/
					 timer0Start();
			      while(i < gu16_sw_counter)
				  {
				      i++;
				      while(1)
					  {
					    if((TIFR & 0x02))
					    {
						    /*reset OCF0*/
						    TIFR |= 0x02;						    
						    /*break while loop*/
						    break;
					    };
				     }
                 /*reinitialize OCR0*/
                 OCR0 = T0_OCR_MAX;
				  }
			      /* stop the timer*/
			      timer0Stop();
			   break;
			   case T0_INTERRUPT_CMP:
			      /* set Global interrupt bit*/
			      sei();   
			      /* set OCIE0 */
			      TIMSK |= timer0.en_interruptMask;				  
			      /* start the timer*/
			      timer0Start();
               /* wait for the timer to finish*/
               while(1)
               {
                  if(gu16_sw_counter == 0) break;
               }
               /* stop the timer*/
               timer0Stop();
               /* reset OCIE0 */
			      TIMSK &= ~(timer0.en_interruptMask);
               /* clear Global interrupt bit*/
			      cli(); 
			   break;
		   }
         break;		
	}    
}

/*
 * user defined
 */
void timer0DelayUs(uint32_t u32_delay_in_us)
{
    //calculate tick number in micro seconds

}

/************************************************ Timers ISRs **********************************************************/
/***************  T0 ISRs *************/
ISR_TIMER0_OVF()
{      
   /* decrement global software counter*/
   gu16_sw_counter--;

}

ISR_TIMER0_COMP()
{
   /*reset OCF0 : set OCF0 to 1*/
   //TIFR |= 0x01;
   /* Initialize OCR0 with 0xFF */
   OCR0 = T0_OCR_MAX;   
   /* decrement global software counter*/
   gu16_sw_counter--;
}









