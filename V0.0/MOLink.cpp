/*
// ------------------------------------------------------------------------------------ //
Title:			MOLink Application for Linux
Filename:		MOLink.cpp
Author:			Paul Vilas-Boas
Version:		0.0
Date:			26/05/2015

Description:	MOLink Application written in C/C++. 
				Used to link between MIDI devices and OSC devices.


// ------------------------------------------------------------------------------------ //
Setting up:

Location - 'cd /home/pi/MOLink'

Compiling - 'g++ -o MOLink MOLink.cpp

Make - Clean & Build: 'make', Clean Only: 'make clean' and Build Only: 'make all'

Execute - './MOLink'

Run Process in Background:
	- 'nohup /home/pi/MOLink/MOLink > MOLink.log 2>&1 & echo $!'
	
Run on boot-up:
	- Edit '/etc/rc.local'
	- Add line after first comments: nohup /home/pi/MOLink/MOLink > /home/pi/MOLink/MOLink.log 2> /home/pi/MOLink/MOLink.err < /dev/null &

WIFI Setup:
	Add to '/etc/wpa_supplicant/wpa_supplicant.conf'
	network={
		ssid="ASUS"
		scan_ssid=1
		psk="password"
	}
	network={
		ssid="ASUS_5G"
		scan_ssid=1
		psk="password"
	}

Setup Serial Port for MIDI:
	# Free serial port from Serial console:
	Modify '/boot/cmdline.txt' - remove sections containing 'ttyAMA0'.
	Modify '/etc/inittab' - comment out line 'T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100'
	
	# Hack!!! Modify UART clock to support MIDI 31500 Baud rate (3MHz x 31250 / 38400) Using UART standard Baud Rate of 38400 bps.
	* Add to '/boot/config.txt'
		init_uart_clock=2441406 
		init_uart_baud=38400
	* Add to '/boot/cmdline.txt'
		bcm2708.uart_clock=3000000
	* Command to test: 
		vcgencmd measure_clock uart

Setup I/O Interface:
	# Installing wiringPi Library - 
	Install GIT first:	'sudo apt-get install git-core'
	Update GIT:			'sudo apt-get update'
	Upgrade GIT:		'sudo apt-get upgrade'
	Go to Folder:		'cd /root/lib/wiringPi'
	Download Lib:		'git clone git://git.drogon.net/wiringPi'
	Update Lib:			'git pull origin'
	Build:				'./build'
	Test:				'gpio -v' or 'gpio readall'

	Compiling - 'g++ -c IO.cpp -L/usr/local/lib -lwiringPi'

// ------------------------------------------------------------------------------------ //
	
// ------------------------------------------------------------------------------------ //
*/

// ------------------------------------------------------------------------------------ //
// Includes
#include <stdio.h>												//
#include <stdlib.h>												// strtof()
#include <string.h>												//
#include <pthread.h>											// Threads

#include <unistd.h>												// for usleep()
#include <sys/resource.h>										// for Process ID
#include <math.h>												// for roundf()

#include "MOLink.h"												// MOLink header
#include "IO.h"													// IO Class
#include "Serial.h"												// Serial Class
#include "OSC.h"												// OSC Class
#include "GenLib.h"												// General Routines

// ------------------------------------------------------------------------------------ //
// Function Prototypes
void InitialiseFootSwitches(void);								// Initialise Foot switches
void ProcessFootSwitches(void);									// Process Foot Switches routine

// ------------------------------------------------------------------------------------ //
// Prototype Callback Functions
void *OnMIDIRead(void);											// On MIDI Read Event
void *BPMTempoThread(void);										// Tempo LED Thread

// ------------------------------------------------------------------------------------ //
// Define Classes
GenLib *GP;														// General Library
RPiIO *IO;														// IO Class Pointer
Serial *UART;													// Serial Class Pointer
RPiOSC *OSC;													// OSC Class Pointer

// ------------------------------------------------------------------------------------ //
// Define Globals
pthread_t BPMThread;											// BPM Thread
int BPM, prevBPM;												// Beats per minute
int FS1, FS2;													// Foot Switch States
bool AutoTempo, pAutoTempo;										// AutoTempo State (Foot Switch 3)

// ------------------------------------------------------------------------------------ //
// MAIN
int main(int argc, char **argv)
{
	int RetVal = 1;												// Default Error
	float CPUTmp;												//
	int MidiId;													// Midi UART ID
	char Buff[BUFF_MAX + 1];									//
	bool Reset = true;											//
	
	// Set Process Priority
	#ifdef SETPROCESS
		setpriority(PRIO_PROCESS, 0, SETPROCESS);				// Set Process Priority
	#endif
	
	// Load Libraries
	GP = NULL;													//
	IO = NULL;													// Default
	OSC = NULL;													//
	GP = new GenLib();											// Init. General Library
	IO = new RPiIO();											// Init. RPiIO Library
	UART = new Serial();										// Init. Serial Library
	OSC = new RPiOSC();											// Init. RPiOSC Library
	
	#ifdef DEBUG
		// Intro
		printf(SCREEN_CLEAR);        							//  Clear the terminal screen 
		printf(CURSOR_RESET);         							//  position cursor at top-left corner
		printf(COLOR_RED "-------------------------------------------------------------------------\r\n");
		printf("|                              " COLOR_LIGHT_GREEN "MOLink V0.0                              " COLOR_RED "|\r\n");
		printf("|                          " COLOR_YELLOW "By Paul Vilas-Boas" COLOR_RED "                           |\r\n");
		printf("-------------------------------------------------------------------------\r\n" COLOR_RESET);
	
		// Display some information
		GP->getCPUID(Buff);										// Get CPU ID
		printf("CPU ID:     %s\r\n", Buff);						// Print CPU ID
		CPUTmp = GP->getCPUTemperature();						// Get CPU Temperature
		printf("CPU Temp:   %.2f 'C\r\n", CPUTmp);				// Display Temperature
		
		printf("\r\nRPi MOLink Processing...\r\n");				//
	#endif
	
	// ---------------- Initialise MIDI / OSC ----------------- //
	while(Reset){												// Reset Loop
		Reset = false;											// Disable Reset
		
		// Set-up Foot Switches
		InitialiseFootSwitches();								// Initialise
		
		// Open OSC
		if(!OSC->Open(OSC_IP, OSC_PORT)){						// Open OSC Connection Failed?
			printf("\r\nCan't Open Socket!\r\n");				//
			RetVal = -1;										// Error code
			break;												// Exit 
		}
		
		// Initialise MIDI
		//MidiId = UART->SerialOpen(MIDI_BAUD);					// Open Serial Port for MIDI - ttyAMA0 @ 31250. (Not working with RPi)
		MidiId = UART->SerialOpen(38400);						// Open Serial Port for MIDI - ttyAMA0 @ 31250. (Hacked by changing init_uart_clock)
		if(MidiId < 0){											// Failed?
			printf("\r\nCan't Open Serial Port!\r\n");			//
			RetVal = -1;										// Error code
			break;												// Exit 
		}
		UART->SetOnReadEvent((void *(*)(void))&OnMIDIRead);		// Set up MIDI IN Read Event / Callback
		
		// Setup BPM Tempo Thread
		BPM = TEMPO_DEFAULT;									// Set default BPM
		prevBPM = BPM;											// update previous BPM
		if(pthread_create (&BPMThread, NULL, (void*(*)(void*))&BPMTempoThread, NULL) != 0){
			printf("ERROR!!! Couldn't start BPM Flasher thread!\r\n");
			RetVal = -1;										// Error code
			break;												// Exit 
		}
	
		memset(Buff, 0, BUFF_MAX);								// Init. Buffer
		
		// ---------------------------------------------------- //
		while(1){												// Loop forever
			RetVal = GP->getKey();								// Get key press
			if (RetVal == 0x1B) {								// ESC key pressed?
				RetVal = 0;										// Exit code
				break;											// Exit loop
			}
			
			// Scan foot pedal switches
			ProcessFootSwitches();								// Process the foot switches here
		}
		
		// ----------------- Close MIDI / OSC ----------------- //
		pthread_cancel(BPMThread);								// Cancel thread
		OSC->Close();											// Close OSC Connection
		UART->SerialClose();									// Close MIDI Ports
	}
	
	// ------------------ Shutdown / Clean up ----------------- //	
	if(OSC != NULL){											// OSC Resource exists?
		delete OSC;												// Clean Up
	}
	if(UART != NULL){											// UART Resource exists?
		delete UART;											// Clean Up
	}
	if(IO != NULL){												// IO Resource exists?
		delete IO;												// Clean Up
	}
	if(GP != NULL){												// IO Resource exists?
		delete GP;												// Clean Up
	}
	
	//return RetVal;												// Return
	return 0;
}

// ------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------ //
// Functions
// ------------------------------------------------------------------------------------ //
// Process the Foot Switches/Pedals here
void InitialiseFootSwitches(void)
{
	AutoTempo = true;											// Default Auto Tempo State to On
	FS1 = 0; FS2 = 0;											// Initialise Foot Switch States
	
	
	// Set-up Foot Switches
	pinMode(FTSW_CH1, INPUT);									// Set for input
	pullUpDnControl(FTSW_CH1, PUD_UP);							// Set pull-up pin
	pinMode(FTSW_CH2, INPUT);									// Set for input
	pullUpDnControl(FTSW_CH2, PUD_UP);							// Set pull-up pin
	pinMode(FTSW_CH3, INPUT);									// Set for input
	pullUpDnControl(FTSW_CH3, PUD_UP);							// Set pull-up pin
		
	// Set-up LED's	
	pinMode(LED_CH1, OUTPUT);									// Set Status LED for output pin
	digitalWrite(LED_CH1, LOW);									// Turn Off Pin
	pinMode(LED_CH2, OUTPUT);									// Set Status LED for output pin
	digitalWrite(LED_CH2, LOW);									// Turn Off Pin
	pinMode(LED_CH3, OUTPUT);									// Set Status LED for output pin
	digitalWrite(LED_CH3, LOW);									// Turn Off Pin
		
	pinMode(STATUS_LED, OUTPUT);								// Set Status LED for output pin
	digitalWrite(STATUS_LED, LOW);								// Turn Off Pin
}

// ------------------------------------------------------------------------------------ //
// Process the Foot Switches/Pedals here
void ProcessFootSwitches(void)
{
	char Buff[BUFF_MAX + 1];									//
	int Ret;													//
	long long Timeus;											// microseconds timer
	static bool Skip;											// Skip flag
	
	if(IO != NULL){												// IO Class OK?
		if((Ret = IO->InputDebounce(FTSW_CH1, 0, HOLD_TIME)) > 0){			// Foot switch, channel 1 pressed? <-- Mute FX 3 Slot Only
			FS1 ^= 1;											// Toggle State
			IO->OutputPin(LED_CH1, FS1);						// Output to LED
			strncpy(Buff, "/config/mute/1", BUFF_MAX);			// Mute Group 1
			OSC->SendInt(Buff, FS1);							// Send Int to OSC device (XR18)
			usleep(1000 * DEBOUNCE_TIME);						// 500ms Debounce
		}else if((Ret = IO->InputDebounce(FTSW_CH2, 0, HOLD_TIME)) > 0){	// Foot switch, channel 2 pressed? <-- Mute All FX Slots (1,2,3,4)
			FS2 ^= 1;											// Toggle State
			IO->OutputPin(LED_CH2, FS2);						// Output to LED
			strncpy(Buff, "/config/mute/2", BUFF_MAX);			// Mute Group 2
			OSC->SendInt(Buff, FS2);							// Send Int to OSC device (XR18)
			usleep(1000 * DEBOUNCE_TIME);						// 500ms Debounce
		}else if((Ret = IO->InputDebounce(FTSW_CH3, 0, HOLD_TIME)) > 0){	// Foot switch, channel 3 pressed? <-- Manual Tap Tempo (Tapping Foot Switch at Tempo required. Or Hold > 2 secs for automatic tempo.
			if(Ret == 2){										// Hold Foot Switch? --> Auto Mode
				AutoTempo = true;								// Set Auto Mode
				if(AutoTempo != pAutoTempo){					// On Auto Tempo Transition - On?
					pAutoTempo = AutoTempo;						// Update Previous AutoTempo
					usleep(1000 * DEBOUNCE_TIME);				// Debounce delay
					Skip = true;								// Set Skip Flag (For Auto Hold, Release transition).
					printf("Auto Mode\r\n");
				}
			}else{												// Manual Tap Tempo?
				AutoTempo = false;								// Clear Auto Mode
				if(AutoTempo != pAutoTempo){					// On Auto Tempo Transition - Off?
					if(!Skip){									// Skip Flag Clear?
						pAutoTempo = AutoTempo;					// Update Previous AutoTempo
						printf("Manual Mode\r\n");
					}else{										// Skipped?
						Skip = false;							// Clear Skip flag.
						AutoTempo = true;						// Back to Auto Mode
					}
					usleep(1000 * DEBOUNCE_TIME);				// Debounce delay
				}else{											// No transition
					GP->StopTimer();							// Stop Timer
					Timeus = GP->TimeDelta();					// Sample BPM
					GP->StartTimer();							// Start Timer
					Ret = (1000 * 1000 * 60) / Timeus;			// Calculate BPM from MIDI clock sync
					if((Ret >= TEMPO_MIN)&&(Ret < TEMPO_MAX)){	// Valid BPM range?
						BPM = Ret;								// Update BPM
					}
				}
			}
			IO->OutputPin(LED_CH3, HIGH);						// Output to LED On
		}
	}
}	

// ------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------ //
// Call Back Functions
// ------------------------------------------------------------------------------------ //
// If MIDI IN Has data, vector here.
void *OnMIDIRead(void)
{
	// If any MIDI data is present, it will vector here
	int Len, Tempo;												//
	char Buff[BUFF_MAX + 1];									//
	long long Timeus;											// microseconds timer
	static char Cnt;											//
	float TBuff;												//
	static int ExtFS1, ExtFS2;									// External Foot Switches
	
	Len = UART->SerialRead(Buff);								// Read MIDI
	
	if(AutoTempo){												// Auto MIDI Tempo Sync?
		// Handle MIDI clock synchronise
		if((Len == sizeof(MIDI_CLK_TICK))&&(memcmp(Buff, MIDI_CLK_TICK, Len) == 0)){
			if(Cnt > BPM_SAMPLE){								// Filter, Every 10 MIDI Ticks
				Cnt = 0;										//
				GP->StopTimer();								// Stop Timer
				Timeus = GP->TimeDelta();						// Sample BPM
				GP->StartTimer();								// Start Timer
				TBuff = (float)((1000 * 1000 * 300.0) / (Timeus * BPM_SAMPLE));	// Calculate BPM from MIDI clock tick (24 ticks per quarter note)
				
				Tempo = (int)roundf(TBuff);						// Round off float to nearest decimal
				if((Tempo >= TEMPO_MIN)&&(Tempo < TEMPO_MAX)){	// Valid BPM range?
					BPM = Tempo;								// Update BPM
				}
				if(BPM != prevBPM){								// BPM Changed?
					prevBPM = BPM;								//
					#ifdef DEBUG
						printf("\n\nmsPM = %llu", Timeus);		//
						printf("\nTempo = %i", Tempo);			//
						printf("\nBPM = %i", BPM);				//
					#endif
				}
			}else{												//
				Cnt++;											//
			}
		}else{
			#ifdef DEBUG
				printf("\r\n[%i==%i] -> ", Len, sizeof(MIDI_CC4_HEEL));	// Start of packet
				GP->PrintHex(Buff, Len);						//
			#endif
		}
	}else{														// Manual Tempo (Tap)
		// Debug
		if(Len > 1){
			#ifdef DEBUG
				printf("\r\n[%i==%i] -> ", Len, sizeof(MIDI_CC4_HEEL));	// Start of packet
				GP->PrintHex(Buff, Len);						//
			#endif
		}
	}
	
	// Handle Other MIDI Data (Extra Foot Switches / Pedals) - Optional???
	if(memcmp(Buff, MIDI_CC80_1, sizeof(MIDI_CC80_1)) == 0){
		// rtn/3/mix/on\00\00\00,i\00\00\00\00\00\00			// Mute Channel 1
		ExtFS1 ^= 1;											// Toggle State
		strncpy(Buff, "/ch/01/mix/on", BUFF_MAX);				// Channel 1, Mute
		OSC->SendInt(Buff, ExtFS1);								// Send Int to OSC device (XR18)
	}else if(memcmp(Buff, MIDI_CC81_1, sizeof(MIDI_CC81_1)) == 0){	//
		ExtFS2 ^= 1;											// Toggle State
		strncpy(Buff, "/ch/02/mix/on", BUFF_MAX);				// Channel 2, Mute
		OSC->SendInt(Buff, ExtFS2);								// Send Int to OSC device (XR18)
	}else if(memcmp(Buff, MIDI_CC82_1, sizeof(MIDI_CC82_1)) == 0){	//
		BPM = 120;
	}
	
}

// ------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------ //
// Threads
// ------------------------------------------------------------------------------------ //
// BPM Tempo Thread. Flash LED at BPM rate. Update Tempo via OSC.
void *BPMTempoThread(void)
{
	long msPM = 0;															//
	long msTempo, PmsTempo;													//
	float DelayTime;														//
	char OSCText[BUFF_MAX + 1];												//
	
	while(1){																// Loop Forever (Thread)
		msTempo = (60 * 1000) / BPM;										// BPM to milliseconds
		msPM = msTempo - LED_PULSE_TIME;										// Calculate LED pulse time
		if(AutoTempo){														// Auto MIDI Tempo Sync? Toggle LED
			IO->OutputPulse(LED_CH3, msPM, LED_PULSE_TIME);					// Pulse LED, 100ms Off, remaining BPM time On.
		}else{																// Manual - Pulse LED
			IO->OutputPulse(LED_CH3, LED_PULSE_TIME, msPM);					// Pulse LED, 100ms On, remaining BPM time off.
		}
		
		// Check for Tempo Change Here
		if(msTempo != PmsTempo){											// BPM Change?
			PmsTempo = msTempo;												// Update Previous value
			
			// Send OSC Tempo here...
			DelayTime = (float)(msTempo / 3000.0);							// Calculate delay in float format
			//sprintf(OSCText, "/fx/1/par/01");								// FX Slot 1, Parameter 1 (Delay)
			//OSC->SendFloat(OSCText, DelayTime);							// Send to OSC device (XR18)
			//sprintf(OSCText, "/fx/2/par/01");								// FX Slot 2, Parameter 1 (Delay)
			//OSC->SendFloat(OSCText, DelayTime);							// Send to OSC device (XR18)
			strncpy(OSCText, "/fx/3/par/01", BUFF_MAX);						// FX Slot 3, Parameter 1 (Delay)
			OSC->SendFloat(OSCText, DelayTime);								// Send to OSC device (XR18)
			//sprintf(OSCText, "/fx/4/par/01");								// FX Slot 4, Parameter 1 (Delay)
			//OSC->SendFloat(OSCText, DelayTime);							// Send to OSC device (XR18)
			
			#ifdef DEBUG
				printf("\nBPM Updated: %i [%i:%lu]\n", BPM, LED_PULSE_TIME, msTempo);	// 
			#endif
		}
	}
}

// ------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------ //
