/*
 * UARTlib.c
 *
 * Created: 03/05/2025 10:44:40
 *  Author: axell
 */ 

#include "UARTlib.h"

void initUART()
{
	// Define TX (PD0) as OUT
	DDRD |= (1 << DDD1);
	// Define RX (PD1) as IN
	DDRD &= ~(1 << DDD0);
	// Configure UCSR0A
	UCSR0A = 0x00;
	// Configure UCSR0B (enable receive interrupt, enable reception, enable transmission)
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
	// Configure UCSR0C (asynchronous normal, no parity, 1 stop bit, 8 bit rate)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	// Configure baud rate, UBRR0 = 103 --> 9600 @ 16MHz
	UBRR0 = 103;
}

void writeChar(char caracter)
{
	while((UCSR0A & (1 << UDRE0)) == 0);
	UDR0 = caracter; // Rewrite character until UDR0 is empty and ready to receive again
}

void writeString(char* texto)
{
	for (uint16_t i = 0; *(texto + i) != '\0'; i++) // until text is empty
	{
		writeChar(*(texto + i)); // text direction with pointer
	}
}

void writeInt(uint16_t number)
{
	char buffer[10];
	uint8_t i = 0;

	if (number == 0) {
		writeChar('0');
		return;
	}

	while (number > 0) {
		buffer[i++] = (number % 10) + '0';
		number /= 10;
	}

	while (i > 0) {
		writeChar(buffer[--i]);
	}
}
