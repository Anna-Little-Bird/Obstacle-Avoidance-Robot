#include <xc.h>
#include "LCD.h"
#pragma config WDTEN=OFF, FOSC=INTIO67,MCLRE=EXTMCLR,IESO=OFF, LVP=OFF

// define the constants
#define _XTAL_FREQ 4000000
#define IDLE 0
#define RUN 1
#define TURNRIGHT 2
#define TURNLEFT 3
#define BACK 4
#define MOTOR_A LATC1
#define MOTOR_B LATC2
#define DIR_A LATC0
#define DIR_B LATC5
#define TRIGGER_R LATA2
#define ECHO_R PORTAbits.RA3
#define TRIGGER_L LATA0
#define ECHO_L PORTAbits.RA1

// define the variables
unsigned char state,sec=0,i;
unsigned int distance_r,distance_l;

// interrupt service routine
void interrupt timer_interrupt(void)
{
if (TMR1IF==1)		// if timer1 flag is set
	{
		sec++;
// when it is 5 sec and state is IDLE
		if ((sec==10)&&(state==IDLE))
		{
			state=RUN;	// go to state RUN
			sec=0;	    // clear the counter
			TMR1ON=0;	// turn off timer1
		}
		TMR1=3036;	// for 0.5 second delay
		TMR1IF=0;	// clear the flag
	}
}

// function for displaying data on LCD
void display_dist(unsigned int left, unsigned int right)
{
	if (left>50)
	{
		set_address_line2(3);
		write_data(95);
		write_data(95);
	}
	if (right>50)
	{
		set_address_line2(7);
		write_data(95);
		write_data(95);
	}
	else
	{
		set_address_line2(3);
		write_data((left/10)+48);
		write_data((left%10)+48);
		set_address_line2(7);
		write_data((right/10)+48);
		write_data((right%10)+48);
	}
}

// function for eliminating the errors from sensors
void error (unsigned int distance_l, unsigned int distance_r)
{
	if (distance_r<9)
	{distance_r=distance_r+1;}
	else if ((distance_r>7)&&(distance_r<26))
	{distance_r=distance_r+2;}
	if (distance_r>70)
	{distance_r=70;}
	if (distance_l<14)
	{distance_l=distance_l+1;}
	else if ((distance_l>14)&&(distance_l<30))
	{distance_l=distance_l+2;}
	if (distance_l>70)
	{distance_l=70;}
}

void main(void)
{
	OSCCON=0b01010010;	// oscillator frequency is 4MHz
	// pins' settings
	TRISA=0x0A;
	ANSELA=0x00;
	PORTA=0x00;
	TRISB=0x00;
	ANSELB=0x00;
	PORTB=0x00;
	TRISC=0x00;
	ANSELC=0x00;
	PORTC=0x00;
	// display initialisation
	init_display();
	set_address_line2(1);
	send_msg("L:");
	set_address_line2(5);
	send_msg("R:");
	// timer0 settings
	PSA=0;	    // enable prescaler
	T0PS0=1;	// prescaler is 4
	T0PS1=0;
	T0PS2=0;
	T08BIT=0;	// 16-bit timer0
	T0CS=0;	    // internal clock
	TMR0ON=0;	// disable timer0
	// timer1 settings
	T1CKPS0=1;	// prescaler is 8
	T1CKPS1=1;
	TMR1CS1=0;	// clock source is instruction clock
	TMR1CS0=0;
	T1SOSCEN=0;	// disable second oscillator
	T1RD16=0;	// 2 8-bit operation
	TMR1IE = 1;		// enable timer1 interrupts
	TMR1IF = 0;	// clear the flag
	TMR1ON=0;	// disable timer1
	TMR1=3036;	// for precisely 0.5 sec delay
	GIEH=1;	    // enable global interrupt
	GIEL=1;
	state=IDLE;	// initial state

while(1)
	{
		// measuring the distance from the right sensor
		TMR0H=0x00;	//resetting timer
		TMR0L=0x00;
		TRIGGER_R=1;
		__delay_us(14);
		TRIGGER_R=0;
		while(ECHO_R==0);
		TMR0ON=1;
		while(ECHO_R==1);
		distance_r=TMR0/15;
		// measuring the distance from the left sensor
		TMR0H=0x00;	//resetting timer
		TMR0L=0x00;
		TRIGGER_L=1;
		__delay_us(14);
		TRIGGER_L=0;
		while(ECHO_L==0);
		TMR0ON=1;
		while(ECHO_L==1);
		TMR0ON=0;
		distance_l=TMR0/15;
        // eliminating the measurement error
        error(distance_l,distance_r);
        // displaying the results
		display_dist(distance_l,distance_r);
		// conditions for states
		if ((distance_l<20)&&(distance_r<20))
		{state=BACK;}
		else if ((distance_r<=20)&&((state==RUN)||(state==BACK)))
		{state=TURNLEFT;}
		else if ((distance_l<=20)&&((state==RUN)||(state==BACK)))
		{state=TURNRIGHT;}
		else if (((distance_r>20)&&(state==TURNLEFT))||((distance_l>20)&&(state==TURNRIGHT))||((distance_l>20)&&(distance_r>20)&&(state==BACK)))
		{state=RUN;}
		switch(state)
		{
			case IDLE:
				MOTOR_A=0;	// stop the motors
				MOTOR_B=0;
				set_address_line1(3);	// display the state
				send_msg("IDLE ");
				TMR1ON=1;	// turn on timer1 for 5 sec. delay
				break;
			case RUN:
                // make the motors go in one direction
				DIR_A=1;
				DIR_B=1;
				MOTOR_A=1;	// turn on the motors
				MOTOR_B=1;
				set_address_line1(3);	// display the state
				send_msg(" RUN ");
				break;
			case TURNLEFT:
				MOTOR_A=1;	// run the motors
				MOTOR_B=1;
				__delay_ms(100);
				MOTOR_A=0;	// stop the motor A
				__delay_ms(100);
                // change the direction of motor A for turning
				DIR_A=0;
				DIR_B=1;
				MOTOR_A=1;	// run the motor A
				set_address_line1(3);	// display the state
				send_msg("LEFT ");
				break;
			case TURNRIGHT:
				MOTOR_A=1;	// run the motors
				MOTOR_B=1;
				__delay_ms(100);
				MOTOR_B=0;	// stop the motor B
				__delay_ms(100);
                // change the direction of motor B for turning
				DIR_A=1;
				DIR_B=0;
				MOTOR_B=1;	// run the motor B
				set_address_line1(3);	// display the state
				send_msg("RIGHT");
				break;
			case BACK:
				DIR_A=0;	// make the motors go backward
				DIR_B=0;
				MOTOR_A=1;	// run the motors
				MOTOR_B=1;
				set_address_line1(3);	// display the state
				send_msg("BACK ");
				break;
		}
	}
}
