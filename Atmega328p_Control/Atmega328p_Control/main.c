/*
 * Atmega328p_Control.c
 *
 * Created: 14/10/2025 16:16:41
 * Author : axell
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Lib/UARTlib.h"

#define cmdPUNCH 0x30
#define cmdCHANGE 0x31
#define cmdUP 0x32
#define cmdDOWN 0x33
#define cmdLEFT 0x34
#define cmdRIGHT 0x35
#define cmdPOWER 0x36
#define cmdDEFAULT 0x37

volatile uint8_t portState0 = 0;
volatile uint8_t portState1 = 0;
volatile uint8_t trigger1 = 0;
volatile uint8_t trigger2 = 0;
volatile uint8_t trigger3 = 0;
volatile uint8_t counter = 0;
volatile uint8_t adcvalX = 0;
volatile uint8_t adcvalY = 0;
volatile uint8_t channel = 6;
volatile uint8_t counter2 = 0;
volatile uint8_t joyX_active = 0;
volatile uint8_t joyY_active = 0;

void setup(void);

int main(void)
{
	setup();
	while (1)
	{
		// Wait for buttons
		if (trigger1)
		{
			// Toggle led
			trigger1 = 0;
			writeChar(cmdPUNCH);
			writeChar('\n');
			// Wait some time and return to steady position
			TIMSK2 |= (1 << TOIE2);
		}
		if (trigger2)
		{
			// Toggle led
			trigger2 = 0;
			writeChar(cmdCHANGE);
			writeChar('\n');
			// Wait some time and return to steady position
			TIMSK2 |= (1 << TOIE2);
		}
		if (trigger3)
		{
			// Toggle led
			trigger3 = 0;
			writeChar(cmdPOWER);
			writeChar('\n');
			// Wait some time and return to steady position
			TIMSK2 |= (1 << TOIE2);
		}
		
		// Process ADC values and send via UART0
		// JOYSTICK - X axis
		if (adcvalX <= 80 && !joyX_active) {
			writeChar(cmdUP);
			writeChar('\n');
			joyX_active = 1;
		}
		else if (adcvalX >= 180 && !joyX_active) {
			writeChar(cmdDOWN);
			writeChar('\n');
			joyX_active = 1;
		}
		else if (adcvalX > 80 && adcvalX < 180 && joyX_active) {
			writeChar(cmdDEFAULT);
			writeChar('\n');
			joyX_active = 0;
		}

		// JOYSTICK - Y axis
		if (adcvalY <= 80 && !joyY_active) {
			writeChar(cmdLEFT);
			writeChar('\n');
			joyY_active = 1;
		}
		else if (adcvalY >= 180 && !joyY_active) {
			writeChar(cmdRIGHT);
			writeChar('\n');
			joyY_active = 1;
		}
		else if (adcvalY > 80 && adcvalY < 180 && joyY_active) {
			writeChar(cmdDEFAULT);
			writeChar('\n');
			joyY_active = 0;
		}
		// END
	}
}

void setup(void)
{
	cli();
	// Define F_CPU PreScaler = 1
	CLKPR |= (1 << CLKPCE);
	CLKPR &= ~((1 << CLKPS3) | (1 << CLKPS2) | (1 << CLKPS1) | (1 << CLKPS0));
	
	// GPIO
	DDRB &= ~((1 << DDB2) | (1 << DDB1) | (1 << DDB0)); // Input
	PORTB |= (1 << PORTB2) | (1 << PORTB1) | (1 << PORTB0); // Pull-up
	
	// Initialize UART
	initUART();
	
	// ADC configuration
	ADMUX = 0x00;
	ADMUX |= (1	<< REFS0); // AVcc
	ADMUX |= (1 << ADLAR); // Activate justification Left
	ADMUX |= (1 << MUX2) | (1 << MUX1); // ADC6 Channel
	// Begin ADC Control & Status Register
	ADCSRA = 0x00;
	ADCSRA |= (1 << ADIE); // Enable interruptions
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // PreScaler = 128 --> f = 125kHz
	ADCSRA |= (1 << ADEN); // ADC enable
	ADCSRA |= (1 << ADSC); // Start first conversion
	
	// Timer0 in normal mode (0C1A and OC1B disconnected)
	TCCR0A = 0x00;
	// Timer0 PreScaler = 1024 (Overflow = 16ms) and off
	TCCR0B |= (1 << CS02) | (1 << CS00);
	TIMSK0 &= ~(1 << TOIE0);
	
	// Timer2 in normal mode (0C2A and OC2B disconnected)
	TCCR2A = 0x00;
	// Timer2 PreScaler = 1024 (Overflow = 16ms) and off
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
	TIMSK2 &= ~(1 << TOIE2);
	
	// Enable Pin Change Interruptions for PCINT0 and PCINT1
	PCICR = (1 << PCIE0);
	PCMSK0 |= (1 << PCINT2) | (1 << PCINT1) | (1 << PCINT0);
	sei();
}

ISR(PCINT0_vect)
{
	// Enable Timer0 Overflow Interruption
	TIMSK0 |= (1 << TOIE0);
	// Disable Pin Change Interruptions for buttons
	PCMSK0 &= ~( (1 << PCINT2) | (1 << PCINT1) | (1 << PCINT0));
	// Save fist position
	portState0 = PINB & ((1 << PINB0) | (1 << PINB1) | (1 << PINB2));
}

ISR(TIMER0_OVF_vect)
{
	// Counter for 16 ms x 2 = 32 ms
	counter++;
	if (counter == 2)
	{
		counter = 0;
		// Save new value and compare initial and final states
		portState1 = PINB & ((1 << PINB0) | (1 << PINB1) | (1 << PINB2));
		// This works for low flank
		if (portState0 == portState1)
		{
			// Detect PB0 down flank
			if ( (portState0 & (1 << PINB0)) == 0 )
			trigger1 = 1;
			// Detect PB1 down flank
			if ( (portState0 & (1 << PINB1)) == 0 )
			trigger2 = 1;
			// Detect PB2 down flank
			if ( (portState0 & (1 << PINB2)) == 0 )
			trigger3 = 1;
		}
		// Enable Pin Change Interruptions
		PCMSK0 |= (1 << PCINT2) | (1 << PCINT1) | (1 << PCINT0);
		// Disable Timer0 Overflow Interruption
		TIMSK0 &= ~(1 << TOIE0);
	}
}

ISR(TIMER2_OVF_vect)
{
	// Set flag only after 150ms approx
	counter2++;
	if (counter2 == 10)
	{
		counter2 = 0;
		writeChar(cmdDEFAULT);
		writeChar('\n');
		TIMSK2 &= ~(1 << TOIE2);
	}
}


ISR(ADC_vect)
{
	if (channel == 6)
	{
		// Store ADC value
		adcvalX = ADCH;
		// Change channel
		ADMUX = (ADMUX & 0xF0) | 7; // ADC7 Channel
		channel = 7;
	}
	else
	{
		// Store ADC value
		adcvalY = ADCH;
		// Change channel
		ADMUX = (ADMUX & 0xF0) | 6; // ADC6 Channel
		channel = 6;
	}
	// Start conversion
	ADCSRA |= (1 << ADSC);
}