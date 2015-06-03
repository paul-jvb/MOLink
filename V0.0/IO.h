/*
// -------------------------------------------------------------------------------------
Title:			IO Header for RPi - Linux
Filename:		IO.h
Author:			Paul Vilas-Boas
Version:		0.0
Date:			25/05/2015

Description:	Wrapper for the wiringPi library. Interfaces to Raspberry Pi BCM2835 I/O ports.
	
// -------------------------------------------------------------------------------------
*/

#ifndef _IO_H
#define _IO_H

// -------------------------------------------------------------------------------------
// Includes
#include "config.h"												//
#include <wiringPi.h>											// Wiring Pi Library

// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// Constants
#define	DEBOUNCE_TIME		100									// Debounce time in mS


// -------------------------------------------------------------------------------------
// Define IO Class
class RPiIO
{
private:
	int Init;													//
	
public:
	RPiIO();													//
	~RPiIO();													//
	
	void OutputPulse(int Pin, int OnDelay, int OffDelay);		//
	void OutputPin(int Pin, int State);							//
	int InputPin(int Pin);										//
	int InputDebounce(int Pin, char ActiveState, int TimeOut);	//
	
};

// -------------------------------------------------------------------------------------
#endif
