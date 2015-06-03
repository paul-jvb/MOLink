/*
// ------------------------------------------------------------------------------------ //
Title:			OSC for RPi - Linux
Filename:		OSC.cpp
Author:			Paul Vilas-Boas
Version:		0.0
Date:			26/05/2015

Description:	OSC Interface for the Raspberry Pi.


// ------------------------------------------------------------------------------------ //
Resources:
	

// ------------------------------------------------------------------------------------ //
*/

// ------------------------------------------------------------------------------------ //
// Includes
#include <string.h>																		// strstr()
#include <stdio.h>																		// printf()
#include <unistd.h>																		// for usleep()

#include "OSC.h"																		// OSC Class

// ------------------------------------------------------------------------------------ //
void OnReadOSC(void);																	// On Read OSC Event / Callback

UDPSocket *SKT;																			// UPD Socket Class Resource

// ------------------------------------------------------------------------------------ //
// Constructor
RPiOSC::RPiOSC()
{
	SKT = new UDPSocket();																//
	SktId = 0;																			// Initialise
}

// ------------------------------------------------------------------------------------ //
// Deconstructor
RPiOSC::~RPiOSC()
{
	if(SktId > 0){																		// Socket OK?
		SKT->SocketClose();																// Close Socket
	}
	if(SKT != NULL){																	//
		delete SKT;																		//
	}
}

// ------------------------------------------------------------------------------------ //
// Connect to OSC Device
bool RPiOSC::Open(char *Address, int Port)
{
	int Len, TimeOut;																	//
	char Buff[OSC_BUFF_MAX+1];															//
	
	// Get Gateway & IP Address and Display
	SKT->GetGateway(ETH_DEVICE, Buff);
	if(Buff[0] != 0){																	// Ethernet Cable Connected?
		printf("GATEWAY: %s", Buff);
		if(SKT->GetIP(ETH_DEVICE, Buff)){
			printf("IP:      %s\n", Buff);
		}

	}else{																				// No Ethernet?
		SKT->GetGateway(WLAN_DEVICE, Buff);												// Get WIFI Gateway
		if(Buff[0] != 0){																// WIFI Connected?
			printf("GATEWAY: %s", Buff);
			if(SKT->GetIP(WLAN_DEVICE, Buff)){
				printf("IP:      %s\n", Buff);
			}
		}else{
			printf("\nERROR!!! Not connected to network...\n");
			return false;																//
		}
	}
	
	// Auto Detect?
	if((Address == NULL)||(Address == "")){												// Search for IP?
		// Code to scan for OSC device... Unfinished
	}
	
	// Connect to Socket
	SktId = SKT->SocketConnect(Address, Port);											// Connect to socket
	
	if(SktId > 0){																		// Socket OK?
		SKT->SetOnReadEvent((CallBack)&OnReadOSC);										//
		
		// Query OSC Device
		strncpy(Buff, "/xinfo", OSC_BUFF_MAX);											// Prepare OSC command to send
		Send(Buff);																		// Send to OSC device (XR18)
		TimeOut = 2000;
		do{
			Len = GetBytesAvailable();
			usleep(1000);																// Delay for 1000us = 1ms
			TimeOut--;
		}while((Len == 0)&&(TimeOut > 0));
		if(Len > 0){
			usleep(1000 * 1000);
			Receive(Buff, OSC_BUFF_MAX);												//
			printf("Type:    %s\n", &Buff[48]);
			printf("Version: %s\n", &Buff[56]);
			printf("Serial:  %s\n", &Buff[32]);
			printf("\nOSC: ");
			fwrite(Buff, sizeof(char), Len, stdout);
			printf("\nHEX: ");
			for(int i = 0; i < Len; i++){
				printf("%.2X ", Buff[i]);
			}
			printf("\n");
		}
		
		return true;																	// Return Success.
	}
	
	return false;																		// Return Fail
}

// ------------------------------------------------------------------------------------ //
// Disconnect from OSC Device
void RPiOSC::Close(void)
{
	char Buff[OSC_BUFF_MAX+1];															//
	
	if(SktId > 0){																		// Socket OK?
		SKT->SocketClose();																//
	}
}

// ------------------------------------------------------------------------------------ //
// OSC Send
void RPiOSC::Send(const char *Data)
{
	int Len;																			//
	char *Buff;																			//
	
	if(SktId > 0){																		// Socket OK?
		Len = strlen(Data);																//
		Buff = new char[Len + 16];														// Allocate buffer space
		if(Buff != NULL){																// Buffer OK?
			memcpy(Buff, Data, Len);													// Add Data
			memcpy(&Buff[Len], "\0\0", 2);												// Add zero's
			SKT->SocketWrite(Buff, Len + 2);											// Send OSC packet
			delete []Buff;																// Clean up
		}
	}
}

// ------------------------------------------------------------------------------------ //
// OSC Send Int
void RPiOSC::SendInt(const char *Data, int Value)
{
	int Len, Ofs;																		//
	char *Buff;																			//
	IntRaw rawI;
	if(SktId > 0){																		// Socket OK?
		Len = strlen(Data);																//
		Buff = new char[Len + sizeof(OSCInt) + 16];										// Allocate buffer space
		if(Buff != NULL){																// Buffer OK?
			memset(Buff, 0, Len + sizeof(OSCInt) + 15);									//
			memcpy(Buff, Data, Len);													//
			Ofs = Len / 4;																//
			Ofs = ((Ofs + 1) * 4);															//
			memcpy(&Buff[Ofs], OSCInt, sizeof(OSCInt));									// Add OSC Text
			rawI.i = Value;																// Set Int
			Buff[Ofs + sizeof(OSCInt)] = rawI.s[3];
			Buff[Ofs + sizeof(OSCInt) + 1] = rawI.s[2];
			Buff[Ofs + sizeof(OSCInt) + 2] = rawI.s[1];
			Buff[Ofs + sizeof(OSCInt) + 3] = rawI.s[0];
			/*
			printf("\nOSC ([%i]%i): ", Ofs, Value);
			for(int i = 0; i < Len + sizeof(OSCInt) + sizeof(rawI.s); i++){
				printf("%.2X ", Buff[i]);
			}
			printf("\r\n");
			*/
			SKT->SocketWrite(Buff, Ofs + sizeof(OSCInt) + sizeof(rawI.s));				// Send OSC packet
			delete []Buff;																// Clean up
		}
	}
}

// ------------------------------------------------------------------------------------ //
// OSC Send Float
void RPiOSC::SendFloat(const char *Data, float Value)
{
	int Len;																			//
	char *Buff;																			//
	FloatRaw rawF;																		//

	if(SktId > 0){																		// Socket OK?
		Len = strlen(Data);																//
		Buff = new char[Len + sizeof(OSCFloat) + 4];									// Allocate buffer space
		if(Buff != NULL){																// Buffer OK?
			memcpy(Buff, Data, Len);													//
			memcpy(&Buff[Len], OSCFloat, sizeof(OSCFloat));								// Add OSC Text
			rawF.f = Value;																// Set float
			Buff[Len + sizeof(OSCFloat)] = rawF.s[3];									// Set float value (Big-Endian order)
			Buff[Len + sizeof(OSCFloat) + 1] = rawF.s[2];	                            //
			Buff[Len + sizeof(OSCFloat) + 2] = rawF.s[1];                               //
			Buff[Len + sizeof(OSCFloat) + 3] = rawF.s[0];                               //
			SKT->SocketWrite(Buff, Len + sizeof(OSCFloat) + sizeof(rawF.s));			// Send OSC packet
			delete []Buff;																// Clean up
		}
	}
}

// ------------------------------------------------------------------------------------ //
// OSC Receive
void RPiOSC::Receive(char *Data, int Size)
{
	if(SktId > 0){																		// Socket OK?
		(void)SKT->SocketRead(Data, Size);												//
	}
}

// ------------------------------------------------------------------------------------ //
// OSC Get Bytes Available
int RPiOSC::GetBytesAvailable(void)
{
	return SKT->GetBytesAvailable();													//
}

// ------------------------------------------------------------------------------------ //
// On OSC Read Event
void OnReadOSC(void)
{
	char Data[OSC_BUFF_MAX+1];
	int Len;
	
	if(SKT != NULL){
		#ifdef DEBUG
			Len = SKT->SocketRead(Data, OSC_BUFF_MAX);									//
			printf("RX-OSC[%i]: ", Len);
			for(int i = 0; i < Len; i++){
				printf("%.2X ", Data[i]);
			}
			printf("\r\n");
			fflush(stdout);
		#endif
	}
}

// ------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------ //
