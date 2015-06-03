/*
// ------------------------------------------------------------------------------------ //
Title:			Serial Class for RPi - Linux
Filename:		Serial.cpp
Author:			Paul Vilas-Boas
Version:		0.0
Date:			25/05/2015

Description:	Interfaces to the UART (RPi's On-Board UART).

// ------------------------------------------------------------------------------------ //
Setting up:

Compiling - 'g++ -c Serial.cpp -lpthread'

// ------------------------------------------------------------------------------------ //
Notes:
	# Free serial port from Serial console:
	Modify '/boot/cmdline.txt' - remove sections containing 'ttyAMA0'.
	Modify '/etc/inittab' - comment out line 'T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100'
	
	# Hack!!! Modify UART clock to support MIDI 31500 Baud rate (3MHz x 31250 / 38400)
	* Add to '/boot/config.txt'
	//init_uart_clock=2441406 
	init_uart_clock=2460937 
	init_uart_baud=38400
	
	* Add to '/boot/cmdline.txt'
	bcm2708.uart_clock=3000000
	
	* Command to test: 'vcgencmd measure_clock uart'
// ------------------------------------------------------------------------------------ //
*/

// ------------------------------------------------------------------------------------ //
// Includes
#include <stdio.h>												// For printf()
#include <string.h>												// For strlen(), memset()
#include <unistd.h>												// Used for UART
#include <fcntl.h>												// Used for UART
#include <termios.h>											// Used for UART
#include <sys/ioctl.h>											//
#include <linux/serial.h>										//
#include "Serial.h"												// Include Serial Class


// ------------------------------------------------------------------------------------ //
// Constructor
Serial::Serial()
{
	Fd = -1;													// Initialise File Descriptor as error
	OnReadEventPtr = NULL;										// Clear Callback Function Pointer
	RxThreadActive = false;										// Init.
	
	#ifdef DEBUG
		printf("\r\nRPi Serial Startup...\r\n");
	#endif
	
}

// ------------------------------------------------------------------------------------ //
// De-constructor
Serial::~Serial()
{
	#ifdef DEBUG
		printf("\r\nRPi Serial Shutdown...\r\n");
	#endif
	
	SerialClose();												// Close Serial Port & Threads
}

// ------------------------------------------------------------------------------------ //
// Get Baud Rate Standards
int Serial::BaudRateConstant(int BaudRate) {
#define B(x) case x: return B##x
	switch(BaudRate) {
		B(50);     B(75);     B(110);    B(134);    B(150);
		B(200);    B(300);    B(600);    B(1200);   B(1800);
		B(2400);   B(4800);   B(9600);   B(19200);  B(38400);
		B(57600);  B(115200); B(230400); B(460800); B(500000); 
		B(576000); B(921600); B(1000000);B(1152000);B(1500000);
	default: return 0;
	}
#undef B
}    

// ------------------------------------------------------------------------------------ //
// Serial Port Open (With custom Baud Rate Support)
int Serial::SerialOpen(int Baud)
{
	struct termios Options;										//
	struct serial_struct SerInfo;								//
	int Speed = 0;												//
	
	// Open and configure serial port
	if ((Fd = open (SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1){
		printf("\r\nERROR!!! Can't Open Serial Port...\r\n");
		return -1;
	}
	
	Speed = BaudRateConstant(Baud);

	if (Speed == 0) {
		// Custom divisor
		SerInfo.reserved_char[0] = 0;
		if (ioctl(Fd, TIOCGSERIAL, &SerInfo) < 0){
			printf("\r\nERROR!!! Can't TIOCGSERIAL (1)...\r\n");
			return -1;
		}
		SerInfo.flags &= ~ASYNC_SPD_MASK;
		SerInfo.flags |= ASYNC_SPD_CUST;
		SerInfo.custom_divisor = (SerInfo.baud_base + (Baud / 2)) / Baud;
		if (SerInfo.custom_divisor < 1){ 
			SerInfo.custom_divisor = 1;
		}
		if (ioctl(Fd, TIOCSSERIAL, &SerInfo) < 0){
			printf("\r\nERROR!!! Can't TIOCSSERIAL...\r\n");
			return -1;
		}
		if (ioctl(Fd, TIOCGSERIAL, &SerInfo) < 0){
			printf("\r\nERROR!!! Can't TIOCGSERIAL (2)...\r\n");
			return -1;
		}
		if (SerInfo.custom_divisor * Baud != SerInfo.baud_base) {
			printf(	"Actual Baudrate is %d / %d = %f\r\n",
					SerInfo.baud_base, SerInfo.custom_divisor,
					(float)SerInfo.baud_base / SerInfo.custom_divisor);
		}
		#ifdef DEBUG
			printf(	"Actual Baudrate is %d / %d = %f\n",
					SerInfo.baud_base, SerInfo.custom_divisor,
					(float)SerInfo.baud_base / SerInfo.custom_divisor);
			printf("Baud Divisor: %i\r\n", SerInfo.custom_divisor);
			printf("Baud Base: %i\r\n", SerInfo.baud_base);
		#endif
	}
	
	fcntl(Fd, F_SETFL, 0);
	tcgetattr(Fd, &Options);
	cfsetispeed(&Options, Speed ?: B38400);
	cfsetospeed(&Options, Speed ?: B38400);
	cfmakeraw(&Options);
	Options.c_cflag |= (CLOCAL | CREAD);
	//Options.c_cflag &= ~CRTSCTS;
	Options.c_cflag &= ~PARENB ;
    Options.c_cflag &= ~CSTOPB ;
    Options.c_cflag &= ~CSIZE ;
    Options.c_cflag |= CS8 ;
    Options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ;
    Options.c_oflag &= ~OPOST ;
	Options.c_cc [VMIN]  =   0 ;
    Options.c_cc [VTIME] = 100 ;	// Ten seconds (100 deciseconds)
	if (tcsetattr(Fd, TCSANOW | TCSAFLUSH, &Options) != 0){
		printf("\r\nERROR!!! Can't TCSANOW | TCSAFLUSH...\r\n");
		return -1;
	}
	
	// Start Read Thread
	if(!RxThreadActive){
		if(pthread_create(&uartThread, NULL, (void* (*)(void*))&Serial::ReadThread, this) != 0){
			printf("\r\nERROR!!! Can't create UART RX Thread...\r\n");
			return -1;
		}
	}
	
	return Fd;													// Return File Descriptor
}

// ------------------------------------------------------------------------------------ //
// Serial Port Close (Wrapper)
void Serial::SerialClose(void)
{
	RxThreadActive = false;										// Clear Thread Active
	usleep(1000);												// 1000us delay
	
	if(RxThreadActive){											// Thread active?
		pthread_cancel(uartThread);								// Cancel thread
	}
	
	if(Fd >= 0){												// OK / Open?
		close(Fd) ;												//
		Fd = -1;												//
	}
}

// ------------------------------------------------------------------------------------ //
// Serial Write
void Serial::SerialWrite(const char *Data)
{
	pthread_mutex_lock(&uartMutexes[0]);						// Lock read thread
	write (Fd, Data, strlen(Data)) ;							// Write to UART
	usleep(1000);												// 1000us delay
	pthread_mutex_unlock(&uartMutexes[0]) ;						// Unlock read thread
}

// ------------------------------------------------------------------------------------ //
// Serial Read
int Serial::SerialRead(char *Data)
{
	int RxSize = RxPtr;											//
		
	if(RxSize < RX_BUFFER_SIZE){								// Buffer OK?
		memcpy(Data, RxData, RxSize);							// Copy to Buffer
	}
	RxPtr = 0;													// Reset RxPtr
	memset(RxData, 0x00, RxSize);								// Clear Buffer
	
	return RxSize;												//
}

// ------------------------------------------------------------------------------------ //
// Set On Read Event Function Pointer (OnReadEventPtr) / Set Callback Function
void Serial::SetOnReadEvent(void *(*FnPtr)(void))
{
	if(FnPtr != NULL){
		OnReadEventPtr = FnPtr;
	}
}

// ------------------------------------------------------------------------------------ //
// On Serial Read Event (Fixed)
void Serial::OnReadEvent(void)
{
	if(OnReadEventPtr != NULL){									// Callback function set?
		OnReadEventPtr();										// Call, Callback function here
	}
}

// ------------------------------------------------------------------------------------ //
// Read UART Thread
void *Serial::ReadThread(Serial *C) 
{
	char Buff[(RX_BUFFER_SIZE / 4) + 1];						// 
	int Bytes;													//
	
	C->RxThreadActive = true;									// Set Thread Active
	do{
		if ((ioctl(C->Fd, FIONREAD, &Bytes) != -1)&&(Bytes > 0)){	// Any Data present?
			pthread_mutex_lock(&C->uartMutexes[0]);				// Lock thread
			if(read(C->Fd, Buff, Bytes) == Bytes){				// Read OK?
				if(C->RxPtr < RX_BUFFER_SIZE){					// Buffer OK?
					memcpy(&C->RxData[C->RxPtr], Buff, Bytes);	// Save 
					C->RxPtr += Bytes;							// Update byte counter
					C->OnReadEvent();							// Call On Serial Read Event
				}else{											// Buffer Full?
					// Reset / Flush buffer
					tcflush (C->Fd, TCIOFLUSH);					//
					C->RxPtr = 0;								//
					memset(C->RxData, 0x00, RX_BUFFER_SIZE);	//
				}
			}
			pthread_mutex_unlock(&C->uartMutexes[0]) ;			// Unlock thread
		}
	}while(C->RxThreadActive);									// Loop while active
}
// ------------------------------------------------------------------------------------ //
