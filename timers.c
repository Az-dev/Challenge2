#include "timers.h"
#include "std_types.h"
#include "registers.h"
#include "interrupts.h"

typedef struct gstr_timerInit_t{
    uint8_t en_mode;
    uint8_t en_en_OC0;
    uint8_t en_prescal;
    uint8_t u8_initialValue;
    uint8_t u8_outputCompare;
    uint8_t en_interruptMask;    
}gstr_timerInit_t; 

static gstr_timerInit_t timer0;

volatile uint16_t gu16_sw_counter = 0;

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
    }else if (timer0.en_mode = T0_COMP_MODE){        
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
    TCCR0 &= ~(T0_NO_CLOCK);
}

/**
 * Description:
 * @param delay
 */
void timer0DelayMs(uint16_t u16_delay_in_ms)
{
    uint16_t ticks_number = 0;
    // uint16_t sw_counter = 0;
    uint8_t ov_initial = 0; /* over flow initial value*/
    uint8_t oc_initial = 0; /* output compare initial value*/

    /* Intialize the timer*/
    timer0Init(T0_NORMAL_MODE,T0_OC0_DIS,T0_PRESCALER_NO,0,0,T0_POLLING);

    /* set the timer */
    /*
    if(timer0.en_mode == T0_NORMAL_MODE)
    {
        timer0Set(timer0.u8_initialValue);        
    }else if (timer0.en_mode = T0_COMP_MODE){
        timer0Set(timer0.u8_outputCompare);       
    }
    */   

    /* calculate tick number in milli seconds */    
    ticks_number = (u16_delay_in_ms * CPU_F) / (timer0.en_prescal * MILLI_SECONDS);
    /* calculate software_counter value in milli seconds */
    gu16_sw_counter   = ticks_number / T0_OV_VAL;
    gu16_sw_counter   = (ticks_number % T0_OV_VAL) == 0? gu16_sw_counter : (gu16_sw_counter + 1);
    /* set timer0*/
    if(timer0.en_mode == T0_NORMAL_MODE)
    {
        /* set TOV initial value*/        
        timer0Set((T0_OV_VAL - (uint8_t)(ticks_number % T0_OV_VAL)));        
    }else if (timer0.en_mode = T0_COMP_MODE){
        /* set OC initial value*/
        timer0Set((uint8_t)(ticks_number % T0_OV_VAL));
    }    

    /* check whether interrupt mode will be activated or polling mode*/
    if(timer0.en_interruptMask)   
    {
        /* Set Global interrupt bit*/
        SREG  |= 0x80;
        /* Initialize TIMSK */
        TIMSK |= timer0.en_interruptMask;
        /* start the timer*/
        timer0Start();

    }else
    { 
        /* polling mode */
        int i = 0;
        /* start the timer*/
        timer0Start();

        for(;i <= gu16_sw_counter;i++)
        {
            while(1)
            {
                if(TIFR & 0x01) break;
            }
        }

        /* stop the timer*/
        timer0Stop();
    }   
    
}

/*
 * user defined
 */
void timer0DelayUs(uint32_t u32_delay_in_us)
{
    //calculate tick number in micro seconds

}

void TIMER0_OVF_vect(void)
{
    /* TOV flag has been rest automatically*/
    /* reset TCNT0 */
    //TCNT0 &= 0x00;
    /* check global software counter*/
    if(gu16_sw_counter == 0x00) gu16_sw_counter--;
    else timer0Stop();    
}


void TIMER0_COMP_vect(void)
{
    /* Initialize OCF0 with 0xFF */
    OCR0 = 0xFF; 
    /* check global software counter*/
    if(gu16_sw_counter == 0x00) gu16_sw_counter--;
    else timer0Stop();
}

