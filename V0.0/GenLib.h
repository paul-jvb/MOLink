/*
// ------------------------------------------------------------------------------------ //
Title:        General Library for Linux (Header)
Filename:     GenLib.h
Author:       Paul Vilas-Boas
Version:      0.0
Date:         20/05/2015

Description:  General Routines.
	
// ------------------------------------------------------------------------------------ //
*/

#ifndef _GENLIB_H
#define _GENLIB_H
// ------------------------------------------------------------------------------------ //

// -------------------------------------------------------------------------------------
// Function definitions

class GenLib 
{
private:
	struct timeval start_time, end_time;              // Timers
	
public:
	GenLib(void);
	~GenLib();
	
	int getKey(void);
	void getCPUID(char *ID);
	float getCPUTemperature(void);
	void StartTimer(void);
	void StopTimer(void);
	long long TimeDelta(void);
	
	void PrintHex(char *Data, int Len);
};
// ------------------------------------------------------------------------------------ //
#endif
