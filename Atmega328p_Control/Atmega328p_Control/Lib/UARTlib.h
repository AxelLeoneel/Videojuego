/*
 * UARTlib.h
 *
 * Created: 03/05/2025 10:44:50
 *  Author: axell
 */ 


#ifndef UARTLIB_H_
#define UARTLIB_H_

#include <avr/io.h>

void initUART();
void writeChar(char caracter);
void writeString(char* texto);
void writeInt(uint16_t number);

#endif /* UARTLIB_H_ */