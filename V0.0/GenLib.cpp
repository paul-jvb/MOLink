/*
// ------------------------------------------------------------------------------------ //
Title:        General Library for Linux
Filename:     GenLib.cpp
Author:       Paul Vilas-Boas
Version:      0.0
Date:         20/05/2015

Description:  General Routines.
	
// ------------------------------------------------------------------------------------ //
*/

// ------------------------------------------------------------------------------------ //
// Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>                                   // atoi(), strtof()
#include <sys/ioctl.h>                                // ioctl()
#include <termios.h>                                  // 
#include <sys/time.h>                                 // for timers
#include "GenLib.h"                                   // General Library

// ------------------------------------------------------------------------------------ //
//
GenLib::GenLib(void)
{
	
}

// ------------------------------------------------------------------------------------ //
//
GenLib::~GenLib()
{
	
}

// ------------------------------------------------------------------------------------ //
// Get key press from terminal
int GenLib::getKey(void) 
{
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}

// ------------------------------------------------------------------ /
// Read CPU Serial Number. Returns Serial Number in Success.
// Returns NULL on error.
void GenLib::getCPUID(char *ID)
{
	FILE *FPtr;                                         //
	char FLine[256] = {0};                              //
	
	ID[0] = 0;                                          //
	
	FPtr = fopen("/proc/cpuinfo", "r");                 // Open cpuinfo file
	if (FPtr){                                          // OK?
		do{
			fgets(FLine, sizeof(FLine), FPtr);              // Get Line
			if(strstr(FLine, "Serial") != NULL){            // Serial found?
				strcpy(ID, &strstr(FLine, ": ")[2]);          // Extract serial number only
				ID[strlen(ID)-1] = 0;                         // Remove '\n'
				break;                                        // Exit loop
			}
		}while(!feof(FPtr));                              // Loop until end of file
	}
	
	fclose(FPtr);                                       // Close File handle
}

// ------------------------------------------------------------------ /
// Read CPU Temperature. Returns Temperature on Success.
// Returns NULL on error.
float GenLib::getCPUTemperature(void)
{
	FILE *FPtr;                                         //
	char FLine[256] = {0};                              //
	float Tmp = 0;                                      //
	
	FPtr = fopen("/sys/class/thermal/thermal_zone0/temp", "r");		// Open cpu temperature file
	if (FPtr){                                          // OK?
		do{
			fgets(FLine, sizeof(FLine), FPtr);              // Get Line
			if(FLine != NULL){                              // Data found?
				Tmp = atof(FLine);                            // Convert to Float
				Tmp /= 1000.0;                                // Convert to Celcius
				break;                                        // Exit loop
			}
		}while(!feof(FPtr));                              // Loop until end of file
	}
	
	fclose(FPtr);                                       // Close File handle
	
	return Tmp;                                         // Return Temperature
}

// ---------------------------------------------------------------- //
// StartTimer()
void GenLib::StartTimer(void)
{
	gettimeofday(&start_time, NULL);                    // Start Timer
}

// ---------------------------------------------------------------- //
// StopTimer()
void GenLib::StopTimer(void)
{
	gettimeofday(&end_time, NULL);                     // Stop Timer
}

// ---------------------------------------------------------------- //
// Get Time Difference
long long GenLib::TimeDelta(void)
{
	struct timeval difference;                          //

	difference.tv_sec = end_time.tv_sec - start_time.tv_sec ;     //
	difference.tv_usec = end_time.tv_usec - start_time.tv_usec;   //

	// Using while instead of if below makes the code slightly more robust.
	while(difference.tv_usec < 0)                       //
	{
		difference.tv_usec += 1000000;                    //
		difference.tv_sec -= 1;                           //
	}

	return 1000000LL * difference.tv_sec + difference.tv_usec;		//
}

// ------------------------------------------------------------------------------------ //
void GenLib::PrintHex(char *Data, int Len)
{
	for(int Ptr = 0; Ptr < Len; Ptr++){                 //
		printf("%.2X ", Data[Ptr]);                       //
		fflush(stdout);                                   //
	}
}	

// ------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------ //
