/*
// -------------------------------------------------------------------------------------
Title:        I/O Access for RPi - Linux
Filename:     IO.cpp
Author:       Paul Vilas-Boas
Version:      0.0
Date:         25/05/2015

Description:  Wrapper for the wiringPi library. Interfaces to Raspberry Pi BCM2835 I/O ports.

// -------------------------------------------------------------------------------------
Setting up:

Installing wiringPi Library - 
	Install GIT first:  'sudo apt-get install git-core'
	Update GIT:         'sudo apt-get update'
	Upgrade GIT:        'sudo apt-get upgrade'
	Go to Folder:       'cd /root/lib/wiringPi'
	Download Lib:       'git clone git://git.drogon.net/wiringPi'
	Update Lib:         'git pull origin'
	Build:              './build'
	Test:               'gpio -v' or 'gpio readall'

Compiling - 'g++ -c IO.cpp -L/usr/local/lib -lwiringPi -lpthread'

// -------------------------------------------------------------------------------------
Notes:
Free serial port from Serial console:
	Modify '/boot/cmdline.txt' - remove sections containing 'ttyAMA0'.
	Modify '/etc/inittab' - comment out line 'T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100'


// -------------------------------------------------------------------------------------
Resources:
	-http://www.wiringpi.com/
	
// -------------------------------------------------------------------------------------
*/

// -------------------------------------------------------------------------------------
// Includes
#include <stdio.h>
#include <string.h>                       // For memset()
#include <pthread.h>                      // Threads
#include "IO.h"                           // IO Class


// -------------------------------------------------------------------------------------
// Constructor
RPiIO::RPiIO()
{
	int Ret;                                //
	
	// Initialise wiringPI (BCM2835)
	Ret = wiringPiSetup();                  // Initialise Wiring Pi Library
	if(Ret != 0){                           // OK?
		Init = 0;                             // 
		return;                               // Exit
	}
	Init = 1;                               // Set to OK.
	
	#ifdef DEBUG
		printf("\r\nRPi I/O Startup... %i\r\n", Ret);			//
	#endif
	
}

// -------------------------------------------------------------------------------------
// Deconstructor
RPiIO::~RPiIO()
{
	//
	#ifdef DEBUG
		printf("\r\nRPi I/O Shutdown...\r\n");  //
	#endif
	
}

// -------------------------------------------------------------------------------------
// Output Pulse On -> Off with on/off delays
void RPiIO::OutputPulse(int Pin, int OnDelay, int OffDelay)
{
	if(Init){                                 // OK?
		pinMode(Pin, OUTPUT);                   // Set for output pin
		digitalWrite(Pin, HIGH);                // Turn on pin
		delay(OnDelay);                         // On Delay
		digitalWrite(Pin, LOW);                 // Turn Off Pin
		delay(OffDelay);                        // Delay for 1 second
	}
}

// -------------------------------------------------------------------------------------
// Output to Pin (Write)
void RPiIO::OutputPin(int Pin, int State)
{
	if(Init){                                 // OK?
		pinMode(Pin, OUTPUT);                   // Set for output pin
		digitalWrite(Pin, State);               // Set State of Pin
	}
}

// -------------------------------------------------------------------------------------
// Input from Pin (Read)
int RPiIO::InputPin(int Pin)
{
	int State;                                //
	
	if(Init){                                 // OK?
		pinMode(Pin, INPUT);                    // Set for input pin
		State = digitalRead (Pin);              // Get State of Pin
		return State;                           //
	}
	
	return -1;                                // Return error
}

// -------------------------------------------------------------------------------------
// Input & Debounce from Pin (Read).
// Return active state 0 or 1. Or 2 if held above timeout, Or -1 on error.
int RPiIO::InputDebounce(int Pin, char ActiveState, int TimeOut)
{
	int State, Active = 0;                    //
	int TimeOutCnt = 0;                       //
	
	if(Init){                                 // OK?
		pinMode(Pin, INPUT);                    // Set for input pin
		do{
			State = digitalRead(Pin);             // Get State of Pin
			if((State == ActiveState)&&(Active == 0)){			// Active transition?
				delay(DEBOUNCE_TIME);               // Debounce delay
				Active = 1;                         // Set Active
			}
			delay(1);                             // 1ms delay
			TimeOutCnt++;                         // Time-out, keep count
		}while((State == ActiveState)&&(TimeOutCnt < TimeOut));	// Loop until released
		if(TimeOutCnt >= TimeOut){              // Timed Out?
			return 2;                             // Return Time-Out
		}
		
		return Active;                          // Return State
	}
	
	return -1;                               // Return error
}
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
