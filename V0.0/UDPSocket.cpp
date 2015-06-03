/*
// ------------------------------------------------------------------------------------ //
Title:			UDP Socket Library for Linux
Filename:		UDPSocket.cpp
Author:			Paul Vilas-Boas
Version:		0.0
Date:			19/05/2015

Description:	Simple UDP Socket Library.
	
// ------------------------------------------------------------------------------------ //
*/

// ------------------------------------------------------------------------------------ //
// Includes
#include <stdio.h>												//
#include <stdlib.h>												//
#include <string.h>												//

#include <sys/ioctl.h>											// Used for ioctl()
#include <fcntl.h>												// Used for fnctl()
#include <unistd.h>												//
#include <sys/types.h>											//
#include <netdb.h> 												//
#include <arpa/inet.h>
#include <net/if.h>												// IFNAMSIZ
#include <ifaddrs.h>											// getifaddrs()

#include "UDPSocket.h"											//

// ------------------------------------------------------------------------------------ //
// Constructor
UDPSocket::UDPSocket(void)
{
	udpSocket = -1;												// Initialise File Descriptor as error
	OnReadEventPtr = NULL;										// Clear Callback Function Pointer
	RxThreadActive = false;										// Clear Thread Active
	BytesAvailable = 0;											// Init.
}

// ------------------------------------------------------------------------------------ //
// De-Constructor
UDPSocket::~UDPSocket() 
{
	SocketClose();												// Close socket
}

// ------------------------------------------------------------------------------------ //
// Open UDP Socket.
int UDPSocket::SocketConnect(const char *Address, unsigned short Port) 
{	
	struct hostent *hp;																	//
	
	// Try to create a UDP socket
    if((udpSocket = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0) {				//
		printf("ERROR!!! Could not Create Socket\r\n");									//
		return -1;																		//
    }
	
	// Configure settings in address struct                                             //
	memset(&serverAddr, '\0', sizeof(serverAddr));										//
	serverAddr.sin_family = AF_INET;                                                    //
	serverAddr.sin_port = htons(Port);                                                  //
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);                                     //
	
	// Bind socket with address struct
	if(bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1){     //
		printf("ERROR!!! Unable to Bind Socket\r\n");                                   //
		return -1;                                                                      //
	}

	// Get address or IP (Client)
	hp = gethostbyname(Address);														//
	if (hp == NULL) {                                           						//
		printf("ERROR!!! Unknown Host\r\n");                    						//
		return -1;                                              						//
	}
	
	// Setup Client
	memset(&clientAddr, '\0', sizeof(clientAddr));										//
	clientAddr.sin_family = AF_INET;                                                    //
	clientAddr.sin_port = htons(Port);                                                  //
	bcopy((char *)hp->h_addr, (char *)&clientAddr.sin_addr.s_addr, hp->h_length);		//
	
	// Start Read Thread
	if(RxThreadActive == false){
		if(pthread_create (&sktThread, NULL, (void* (*)(void*))&UDPSocket::ReadThread, this) != 0){
			printf("ERROR!!! Couldn't start socket read thread!\r\n");
		}
	}
	
	return udpSocket;
}

// ------------------------------------------------------------------------------------ //
// Close Socket
void UDPSocket::SocketClose(void) 
{
	int AddrLen = sizeof(serverAddr);

	if(udpSocket > 0) {
		getpeername(udpSocket, (struct sockaddr*)&serverAddr, (socklen_t*)&AddrLen);
		shutdown(udpSocket, SHUT_WR);
		close(udpSocket);
	}
	
	// Kill thread
	if(RxThreadActive == true){
		pthread_cancel(sktThread);
	}
}

// ------------------------------------------------------------------------------------ //
// Get IP Address
bool UDPSocket::GetIP(const char *Device, char *IP)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	if(fd >= 0){
		// I want to get an IPv4 IP address
		ifr.ifr_addr.sa_family = AF_INET;

		// I want IP address attached to "eth0"
		strncpy(ifr.ifr_name, Device, IFNAMSIZ-1);

		ioctl(fd, SIOCGIFADDR, &ifr);

		close(fd);

		// display result
		sprintf(IP, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

		return true;
	}
	
	return false;																		// Fail
}

// ------------------------------------------------------------------------------------ //
// Get IP Address
int UDPSocket::GetIPS(const char *Device, char *IP)
{
	struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
	int DevNum = 0;
	
    if (getifaddrs(&ifaddr) == -1) 
    {
        return -1;
    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL)
            continue;  

        s = getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if((strcmp(ifa->ifa_name, Device) == 0)&&(ifa->ifa_addr->sa_family == AF_INET))
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                return -1;
            }
            printf("\tInterface : <%s>\n",ifa->ifa_name );
            printf("\t  Address : <%s>\n", host); 
        }
    }

    freeifaddrs(ifaddr);
	
	return DevNum;
}

// ------------------------------------------------------------------------------------ //
// Get Gateway (Cheat)
void UDPSocket::GetGateway(const char* Device, char *Data) 
{
    char Buff[64] = {0x0};
	FILE* fp;
	
    sprintf(Buff, "route -n | grep %s  | grep 'UG[ \t]' | awk '{print $2}'", Device);
    fp = popen(Buff, "r");
    
    if(fgets(Buff, sizeof(Buff), fp) != NULL){
        strcpy(Data, Buff);
	}

    pclose(fp);
}

// ------------------------------------------------------------------------------------ //
//
void UDPSocket::SocketWrite(const char *Msg, int Len) 
{
	int Length = Len;
	socklen_t AddrSize;
	
	if(Length == 0){Length = strlen(Msg);}
	if((Length > 0) && (udpSocket > 0)) {
		// Send message to client, using serverStorage as the address
		AddrSize = sizeof serverStorage;
		if(sendto(udpSocket, Msg, Length, 0, (struct sockaddr *)&clientAddr, AddrSize) == -1){
			printf("ERROR!!! Socket write failed\n");
			SocketClose();
		}
	}
}

// ------------------------------------------------------------------------------------ //
//
int UDPSocket::SocketRead(char *recvBuff, int recvSize)
{
	socklen_t AddrSize;
	int nBytes;
	
	memset(recvBuff, '\0', recvSize);
	
	// Try to receive any incoming UDP datagram. Address and port of 
    // requesting client will be stored on serverStorage variable
	AddrSize = sizeof serverStorage;
    nBytes = recvfrom(udpSocket, recvBuff, recvSize, 0, (struct sockaddr *)&serverStorage, &AddrSize);
	
	return nBytes;
}

// -------------------------------------------------------------------------------------
void UDPSocket::SetUnBlocking(void)
{
	int flags = fcntl(udpSocket, F_GETFL, 0);
	fcntl(udpSocket, F_SETFL, flags | O_NONBLOCK);
}

// -------------------------------------------------------------------------------------
void UDPSocket::SetBlocking(void)
{
	int flags = fcntl(udpSocket, F_GETFL, 0);
	fcntl(udpSocket, F_SETFL, flags & ~O_NONBLOCK);
}

// -------------------------------------------------------------------------------------
int UDPSocket::GetBytesAvailable(void)
{
	return BytesAvailable;
}

// -------------------------------------------------------------------------------------
// Set On Read Event Function Pointer (OnReadEventPtr) / Set Callback Function
//void Socket::SetOnReadEvent(void *(*FnPtr)(void))
void UDPSocket::SetOnReadEvent(CallBack FnPtr)
{
	if(FnPtr != NULL){
		OnReadEventPtr = FnPtr;
	}
}

// -------------------------------------------------------------------------------------
// On Socket Read Event (Fixed)
void UDPSocket::OnReadEvent(void)
{
	if(OnReadEventPtr != NULL){									// Callback function set?
		OnReadEventPtr();										// Call, Callback function here
	}
}

// ------------------------------------------------------------------------------------ //
// Socket Read Thread
void *UDPSocket::ReadThread(UDPSocket *C) 
{
	socklen_t AddrSize;
	int PrevBytes = 0;
	char Len[RX_SZ];
	int Ret;
	
	C->RxThreadActive = true;									// Set Thread Active
	do{															//
		Ret = recvfrom(C->udpSocket, Len, sizeof(Len), MSG_PEEK, (struct sockaddr *)&C->serverStorage, &AddrSize);
		if(Ret != -1){											// OK?
			C->BytesAvailable = Ret;
			if((C->BytesAvailable > 0)&&(C->BytesAvailable != PrevBytes)){	// Bytes available?
				C->OnReadEvent();								// Call On Read Event
				PrevBytes = C->BytesAvailable;					// 
			}
		}else{
			C->BytesAvailable = 0;
			PrevBytes = 0;
		}
	}while(C->RxThreadActive);									// Loop while RxThreadActive is set
	C->RxThreadActive = false;									// Clear Thread Active
}

// ------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------ //
