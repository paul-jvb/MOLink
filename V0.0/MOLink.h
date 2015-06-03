/*
// -------------------------------------------------------------------------------------
Title:			MOLink Header for Linux
Filename:		MOLink.h
Author:			Paul Vilas-Boas
Version:		0.0
Date:			25/05/2015

Description:	MOLink Header. Link between MIDI devices and OSC devices.
	
// -------------------------------------------------------------------------------------
*/

#ifndef _MOLINK_H
#define _MOLINK_H
// -------------------------------------------------------------------------------------
// Includes
#include "config.h"												// General Configuration File

// -------------------------------------------------------------------------------------
// Constants
#define BUFF_MAX				1024							// Buffer Maximum Size

// -------------------------------------------------------------------------------------
// MIDI Constants
#define TEMPO_DEFAULT		120									// Default Tempo on Startup
#define TEMPO_MIN			40									// Tempo minimum of 40 BPM
#define TEMPO_MAX			250									// Tempo maximum of 250 BPM

// -------------------------------------------------------------------------------------
// MIDI Codes
const char MIDI_CLK_START[] =	{0xFA};							// MIDI Clock Start
const char MIDI_CLK_STOP[] =	{0xFC};							// MIDI Clock Stop
const char MIDI_CLK_TICK[] =	{0xF8};							// MIDI Clock Tick
const char MIDI_CLK_CONT[] =	{0xFB};							// MIDI Clock Continue
const char MIDI_CC4_HEEL[] =	{0xB0, 0x04, 0x00};				// MIDI CC4 (Heel)
const char MIDI_CC4_TOE[] =		{0xB0, 0x04, 0x7F};				// MIDI CC4 (Toe)

const char MIDI_CC80_0[] = {0xB0, 0x50, 0x00};					// MIDI CC80 (OFF)
const char MIDI_CC80_1[] = {0xB0, 0x50, 0x7F};					// MIDI CC80 (ON) or Trigger
const char MIDI_CC81_0[] = {0xB0, 0x51, 0x00};					// MIDI CC81 (OFF)
const char MIDI_CC81_1[] = {0xB0, 0x51, 0x7F};					// MIDI CC81 (ON) or Trigger
const char MIDI_CC82_0[] = {0xB0, 0x52, 0x00};					// MIDI CC82 (OFF)
const char MIDI_CC82_1[] = {0xB0, 0x52, 0x7F};					// MIDI CC82 (ON) or Trigger

//*
// -------------------------------------------------------------------------------------
// Terminal Constants (PuTTY or similar)
#define SCREEN_CLEAR		"\033[2J"							// Screen Clear
#define CURSOR_RESET		"\033[H"							// Reset cursor to top left of screen

#define COLOR_RESET			"\e[m"								// SSH Console Reset Colour
#define COLOR_NC			"\e[0m"								// No Colour
#define COLOR_WHITE			"\e[1;37m"
#define COLOR_BLACK			"\e[0;30m"
#define COLOR_BLUE			"\e[0;34m"
#define COLOR_LIGHT_BLUE	"\e[1;34m"
#define COLOR_GREEN			"\e[0;32m"
#define COLOR_LIGHT_GREEN	"\e[1;32m"
#define COLOR_CYAN			"\e[0;36m"
#define COLOR_LIGHT_CYAN	"\e[1;36m"
#define COLOR_RED			"\e[0;31m"
#define COLOR_LIGHT_RED		"\e[1;31m"
#define COLOR_PURPLE		"\e[0;35m"
#define COLOR_LIGHT_PURPLE	"\e[1;35m"
#define COLOR_BROWN			"\e[0;33m"
#define COLOR_YELLOW		"\e[1;33m"
#define COLOR_GRAY			"\e[0;30m"
#define COLOR_LIGHT_GRAY	"\e[0;37m"

//*/
// -------------------------------------------------------------------------------------
#endif
