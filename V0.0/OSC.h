/*
// -------------------------------------------------------------------------------------
Title:			OSC Header for RPi - Linux
Filename:		OSC.h
Author:			Paul Vilas-Boas
Version:		0.0
Date:			22/05/2015

Description:	OSC Interface for the Raspberry Pi.
	
// -------------------------------------------------------------------------------------
*/

#ifndef _OSC_H
#define _OSC_H

//#define DEBUG

// -------------------------------------------------------------------------------------
// Includes
#include "config.h"												//
#include "UDPSocket.h"											// Simple UDP Socket Library

// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// Constants
#define OSC_BUFF_MAX		8192								// Buffer Maximum

#define TCP_TYPE			SOCK_STREAM							// TCP type
#define UDP_TYPE			SOCK_DGRAM							// UDP type

#define ETH_DEVICE			"eth0"								// Ethernet Device
#define WLAN_DEVICE			"wlan0"								// Wireless / WIFI Device
#define PORT_XR32			10023								// XR-32 Port
#define PORT_XR18			10024								// XR-18 Port

// Open Sound Control via Sysex F0 00 20 32 32 TEXT F7
const char SysexHdr[] = {0xF0, 0x00, 0x20, 0x32, 0x32};			// Sysex Header
const char SysexFtr[] = {0xF7};									// Sysex Footer


const char OSCInt[] = {',', 'i', 0x00, 0x00};
const char OSCFloat[] = {0x00, 0x00, 0x00, 0x00, ',', 'f', 0x00, 0x00};

// -------------------------------------------------------------------------------------
// Int to char array
typedef union _intRaw{
  int i;
  char  s[4];
} IntRaw;


// -------------------------------------------------------------------------------------
// Float to char array
typedef union _floatRaw{
  float f;
  char  s[4];
} FloatRaw;

// -------------------------------------------------------------------------------------
// Define OSC Class
class RPiOSC
{
private:
	int SktId;													// Socket ID
	
public:
	RPiOSC();													//
	~RPiOSC();													//
	
	bool Open(char *Address, int Port);							//
	void Close(void);											//
	void Send(const char *Data);								//
	void SendInt(const char *Data, int Value);					//
	void SendFloat(const char *Data, float Value);				//
	void Receive(char *Data, int Size);							//
	int GetBytesAvailable(void);								//
	
	void OnRead(void);											//
};

// -------------------------------------------------------------------------------------
#endif
