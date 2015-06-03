/*
// -------------------------------------------------------------------------------------
Title:			MOLink Configuration Header for RPi Linux
Filename:		config.h
Author:			Paul Vilas-Boas
Version:		0.0
Date:			  26/05/2015

Description:	
	
// -------------------------------------------------------------------------------------
*/

#ifndef _CONFIG_H
#define _CONFIG_H 
// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
#define DEBUG                                     // Debug Mode
#undef DEBUG                                      // Disable Debug Mode
#define SETPROCESS    -20                         // Set Process Priority (-20-High, +20-Low)

// -------------------------------------------------------------------------------------
// Fixed Settings
#define OSC_IP        (char *)"192.168.1.46"      // XR18 IP Address
#define OSC_PORT      10024                       // XR18 Port

// -------------------------------------------------------------------------------------
// I/O Pin Settings
#define FTSW_CH1      0                             // Foot Switch Channel 1 (GPIO_GEN0)
#define FTSW_CH2      1                             // Foot Switch Channel 2 (GPIO_GEN1)
#define FTSW_CH3      2                             // Foot Switch Channel 3 (GPIO_GEN2)
#define LED_CH1       3                             // LED Channel 1 (GPIO_GEN3)
#define LED_CH2       4                             // LED Channel 2 (GPIO_GEN4)
#define LED_CH3       5                             // LED Channel 3 (GPIO_GEN5)
#define STATUS_LED    7                             // Status Blue LED (GPIO_GCLK)

// -------------------------------------------------------------------------------------
// Constants
#define LED_PULSE_TIME    100                       // 100ms Pulse On BPM LED
#define HOLD_TIME         3000                      // Debounce hold time in mS
#define BPM_SAMPLE        10                        // BPM Sample 

// -------------------------------------------------------------------------------------
#endif
