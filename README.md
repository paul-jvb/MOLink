# MOLink
MIDI OSC Link for the Raspberry Pi.

Tested with the following:
* OSC device:   Behringer, X-AIR, XR18.
* MIDI device:  BOSS Loop Station, RC-300

The hardware schematics are attached in JPG format (Issues section).

Overview:
* Foot Switch 1 - Toggles Mute Group 1. (LED 1 on when muted).
* Foot Switch 2 - Toggles Mute Group 2. (LED 2 on when muted).
* Foot Switch 3 - Toggles between Auto Tempo Sync & Manual Tap Tempo. (LED 3 flashes to tempo).
Updates OSC device, FX slot 3, parameter 1 (delay in ms).
Tapping foot switch 3 will manually update delay parameter.
Holding foot switch 3 for >3 seconds will activate auto mode, which reads MIDI device tempo and updates OSC delay parameter.

Notes:
* IP Address of OSC device must be manually entered in config.h and re-compiled.
* Setup I/O Interface:
  # Install wiringPi Library - 
  Install GIT first:	'sudo apt-get install git-core'
  Update GIT:			'sudo apt-get update'
  Upgrade GIT:		'sudo apt-get upgrade'
  Go to Folder:		'cd /root/lib/wiringPi'
  Download Lib:		'git clone git://git.drogon.net/wiringPi'
  Update Lib:			'git pull origin'
  Build:          './build'
  Test:           'gpio -v' or 'gpio readall'
* To Make - Clean & Build: 'make', Clean Only: 'make clean' and Build Only: 'make all'
* To Execute - './MOLink'
* To Run Process in Background:
	- 'nohup /home/pi/MOLink/MOLink > MOLink.log 2>&1 & echo $!'
* To Run on boot-up:
	- Edit '/etc/rc.local'
	- Add line after first comments: nohup /home/pi/MOLink/MOLink > /home/pi/MOLink/MOLink.log 2> /home/pi/MOLink/MOLink.err < /dev/null &
* To Setup WIFI (example):
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

* Setup Serial Port for MIDI:
	# Free serial port from Serial console:
	Modify '/boot/cmdline.txt' - remove sections containing 'ttyAMA0'.
	Modify '/etc/inittab' - comment out line 'T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100'
	# Hack!!! Modify UART clock to support MIDI 31500 Baud rate (3MHz x 31250 / 38400) Using UART standard Baud Rate of 38400 bps.
  - Add to '/boot/config.txt'
	  init_uart_clock=2441406 
	  init_uart_baud=38400
  - Add to '/boot/cmdline.txt'
	  bcm2708.uart_clock=3000000
  - Command to test: 
	  vcgencmd measure_clock uart
