#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void inc_stop_watch();

unsigned char seconds_R=0; //PA5
unsigned char seconds_L=0; //PA4
unsigned char minuits_R =0; //PA3
unsigned char minuits_L =0; //PA2
unsigned char hours_R =0; //PA1
unsigned char hours_L =0; //PA0

unsigned char glob_tick =0;

unsigned char puase_flag =0;
unsigned char reset_flag =0;
unsigned char resume_flag =0;
//====================================ISR==============================================



ISR (INT0_vect)
{
	reset_flag =1; //RESET
	GIFR |= (1<<INT0); //CLEAR INT0 FLAG
}

ISR (INT1_vect)
{
	puase_flag =1; //PAUSE
	GIFR |= (1<<INT1);  //CLEAR INT1 FLAG
}

ISR (INT2_vect)
{
	resume_flag =1; //RESUME
	GIFR |= (1<<INT2);  //CLEAR INT2 FLAG
}

ISR (TIMER1_COMPA_vect)
{
	// set the tick to indicate that the timer counted 1sec.
	glob_tick = 1;
}



//===================================FUNCTIONS & INTERRUPTS=================================



void TIMER1_CTC_INIT()
{

	TCNT1 = 0;		/* Set timer1 initial count to zero */

	OCR1A = 977;    /* Set the Compare value to 977 - T(timer) = 1.024 ms */

	TIMSK |= (1<<OCIE1A); /* Enable Timer1 Compare A Interrupt */

	/* Configure timer control register TCCR1A (Mode Number 4) (CTC)
	 * 1. Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * 3. CTC Mode WGM10=0 WGM11=0
	 */
	TCCR1A = (1<<FOC1A);

	/* Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	 * 2. Prescaler = F_CPU/1024 CS10=1 CS11=0 CS12=1
	 */
	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS12);
}

void TIMER1_DISABLE()
{
	TIMSK = 0;
	TCCR1A = 0;
	TCCR1B = 0;
}


void INT0_RESET_INIT()
{
	DDRD   &= (~(1<<PD2));   // Configure INT0/PD2 as input pin
	PORTB  |= (1<<PD2);      //enable internal pull up
	GICR   |= (1<<INT0);     // Enable external interrupt pin INT0
	MCUCR  |= (1<<ISC01);    // Trigger INT0 with the FALLING edge
	MCUCR  &= ~(1<<ISC00);
}


void INT1_PAUSE_INIT()
{
	DDRB   &= (~(1<<PB3));   // Configure INT1/PB3 as input pin
	GICR   |= (1<<INT1);     // Enable external interrupt pin INT1
	MCUCR  |= (1<<ISC11) | (1<<ISC10);    // Trigger INT0 with the RISING edge
}


void INT2_RESUME_INIT()
{
	DDRB   &= (~(1<<PB2));   // Configure INT2/PB2 as input pin
	PORTB  |= (1<<PB2);      //enable internal pull up
	GICR   |= (1<<INT2);	 // Enable external interrupt pin INT2
	MCUCSR &= ~(1<<ISC2);     // Trigger INT2 with the FALLING edge
}


void inc_stop_watch()
{
	seconds_R++;
	if (seconds_R >9)
	{
		seconds_R=0;
		seconds_L++;
		if (seconds_L > 5)
		{
			seconds_L =0;
			minuits_R++;
			if (minuits_R > 9)
			{
				minuits_R=0;
				minuits_L++;
				if(minuits_L > 5)
				{
					minuits_L =0;
					hours_R++;
					if (hours_R > 9)
					{
						hours_R=0;
						hours_L++;
						if(hours_L > 9)
						{
							seconds_R=0; //PA5
							seconds_L=0; //PA4
							minuits_R =0; //PA3
							minuits_L =0; //PA2
							hours_R =0; //PA1
							hours_L =0; //PA0
						}
					}
				}
			}
		}
	}
}

//======================================MAIN==========================================



int main (void)
{
	SREG   &= ~(1<<7);       // Disable interrupts by clearing I-bit
	INT0_RESET_INIT();
	INT1_PAUSE_INIT();
	INT2_RESUME_INIT();
	TIMER1_CTC_INIT();
	SREG  |= (1<<7);       // Enable interrupts by setting I-bit

	DDRC |= 0x0F;          //configure first 4 pins in port C to outputs
	DDRA |= 0x3F ;         //configure first 6 pins in port A to outputs


	while(1)
	{
		PORTA = (1<<0);
		PORTC = seconds_R ;
		_delay_ms(2);
		PORTA = (1<<1);
		PORTC = seconds_L;
		_delay_ms(2);
		PORTA = (1<<2);
		PORTC = minuits_R ;
		_delay_ms(2);
		PORTA = (1<<3);
		PORTC = minuits_L ;
		_delay_ms(2);
		PORTA = (1<<4);
		PORTC = hours_R ;
		_delay_ms(2);
		PORTA = (1<<5);
		PORTC = hours_L ;
		_delay_ms(2);

		if (glob_tick == 1)
		{
			inc_stop_watch();
			glob_tick =0;
		}

		if (reset_flag ==1)
		{
		    seconds_R=0; //PA5
			seconds_L=0; //PA4
			minuits_R =0; //PA3
			minuits_L =0; //PA2
			hours_R =0; //PA1
			hours_L =0; //PA0

			reset_flag =0;
		}

		else if(puase_flag == 1)
		{
			TIMER1_DISABLE();
			puase_flag = 0;
		}

		else if (resume_flag ==1)
		{
			TIMER1_CTC_INIT();
			resume_flag=0;
		}


	}

}


