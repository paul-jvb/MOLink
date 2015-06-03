/*
// ------------------------------------------------------------------------------------ //
Title:			UDP Socket Library for Linux (Header)
Filename:		UDPSocket.h
Author:			Paul Vilas-Boas
Version:		0.0
Date:			19/05/2015

Description:	Simple UDP Socket Library.
	
// ------------------------------------------------------------------------------------ //
*/

#ifndef _UDPSOCKET_H
#define _UDPSOCKET_H
// ------------------------------------------------------------------------------------ //
// Includes
#include <sys/socket.h>											//
#include <netinet/in.h>											//
#include <pthread.h>


// ------------------------------------------------------------------------------------ //
// Constants
#define RX_SZ				2048

// ------------------------------------------------------------------------------------ //
//
typedef void *(*CallBack)(void);								// c style callback

// ------------------------------------------------------------------------------------ //
// UDP Socket Class
class UDPSocket 
{
private:
	int udpSocket;												//
	struct sockaddr_in serverAddr, clientAddr;					//
	struct sockaddr_storage serverStorage;						//
	
	pthread_mutex_t sktMutexes[2];								//
	pthread_t sktThread;										//
	bool RxThreadActive;										//
	bool TxThreadActive;										//
	void *(*OnReadEventPtr)(void);								//
	
	static void *ReadThread(UDPSocket *);						//
	
public:
	int BytesAvailable;											// Bytes Available
	
	UDPSocket(void);											//
	~UDPSocket();												//
	
	int SocketConnect(const char *Address, unsigned short Port);//
	void SocketClose(void);										//
	bool GetIP(const char *Device, char *IP);					//
	int GetIPS(const char *Device, char *IP);					//
	void GetGateway(const char* Device, char *Data);			//
	void SocketWrite(const char *msg, int len);					//
	int SocketRead(char *recvBuff, int recvSize);				//
	void SetUnBlocking(void);									//
	void SetBlocking(void);										//
	int GetBytesAvailable(void);								//
	
	void SetOnReadEvent(CallBack);								//
	void OnReadEvent(void);										//
};

// ------------------------------------------------------------------------------------ //
#endif
