/*
// -------------------------------------------------------------------------------------
Title:			Serial Class Header for RPi - Linux
Filename:		Serial.h
Author:			Paul Vilas-Boas
Version:		0.0
Date:			22/05/2015

Description:	Interfaces to the UART (RPi's On-Board UART).
	
// -------------------------------------------------------------------------------------
*/

#ifndef _SERIAL_H
#define _SERIAL_H

// -------------------------------------------------------------------------------------
// Includes
#include "config.h"												// General Configuration File
#include <pthread.h>											// Threads

// -------------------------------------------------------------------------------------
// Constants
#define MIDI_BAUD		31250									// MIDI Baud Rate
#define RX_BUFFER_SIZE	4096									// UART Receive Buffer Size
#define SERIAL_PORT		"/dev/ttyAMA0"							// RPi's Onboard Serial Port

// -------------------------------------------------------------------------------------
// Define Serial Class
class Serial
{
private:
	int Fd;														//
	pthread_mutex_t uartMutexes[2];								//
	pthread_t uartThread;										//
	bool RxThreadActive;										//
	int RxPtr;													//
	char RxData[RX_BUFFER_SIZE + 1];							//
	void *(*OnReadEventPtr)(void);								//
	
	static void *ReadThread(Serial *);							//
	
public:
	Serial();													//
	~Serial();													//
	
	int BaudRateConstant(int BaudRate);							//
	int SerialOpen(int Baud);									//
	void SerialClose(void);										//
	void SerialWrite(const char *Data);							//
	int SerialRead(char *Data);									//

	void SetOnReadEvent(void *(*FnPtr)(void));					//
	void OnReadEvent(void);										//
};

// -------------------------------------------------------------------------------------
#endif
