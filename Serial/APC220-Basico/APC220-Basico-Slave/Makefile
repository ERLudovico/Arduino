# Arduino NetBeans plugin (http://code.google.com/p/arduino-netbeans)
# ------------------------------------------------------------------------------
# Configure the values of COM_PORT, ARDUINO_BASE_DIR and platform (Uno or Mega). 
# Uncomment the declarations of the libraries that you want to use.
# Based on: http://playground.arduino.cc/Code/Netbeans

# Mini
COM_PORT = COM8
# NANO
COM_PORT = COM9
ARDUINO_VERSION = 105
ARDUINO_BASE_DIR = "D:\DADOS EXT\Aplicativos\Arduino\arduino-1.0.5"
ARDUINO_CORE_DIR = ${ARDUINO_BASE_DIR}/hardware/arduino/cores/arduino
ARDUINO_LIB_DIR = ${ARDUINO_BASE_DIR}/libraries

# Arduino Uno/NANO/Mini:
ARDUINO_MODEL = atmega328p
ARDUINO_PROGRAMMER = arduino
ARDUINO_PINS_DIR = ${ARDUINO_BASE_DIR}/hardware/arduino/variants/standard

# Arduino Mega 2560:
#ARDUINO_MODEL = atmega2560
#ARDUINO_PROGRAMMER = wiring
#ARDUINO_PINS_DIR = ${ARDUINO_BASE_DIR}/hardware/arduino/variants/mega

INCLUDE = -c -g \
-I${ARDUINO_CORE_DIR} \
-I${ARDUINO_PINS_DIR} \
-I${ARDUINO_LIB_DIR}/EEPROM \
-I${ARDUINO_LIB_DIR}/Esplora \
-I${ARDUINO_LIB_DIR}/Ethernet \
-I${ARDUINO_LIB_DIR}/Ethernet/utility \
-I${ARDUINO_LIB_DIR}/Firmata \
-I${ARDUINO_LIB_DIR}/GSMSHIELD \
-I${ARDUINO_LIB_DIR}/LiquidCrystal \
-I${ARDUINO_LIB_DIR}/MemoryFree \
-I${ARDUINO_LIB_DIR}/RTClib \
-I${ARDUINO_LIB_DIR}/SD \
-I${ARDUINO_LIB_DIR}/SD/utility \
-I${ARDUINO_LIB_DIR}/Servo \
-I${ARDUINO_LIB_DIR}/SoftwareSerial \
-I${ARDUINO_LIB_DIR}/SPI \
-I${ARDUINO_LIB_DIR}/Stepper \
-I${ARDUINO_LIB_DIR}/VirtualWire \
-I${ARDUINO_LIB_DIR}/WiFi \
-I${ARDUINO_LIB_DIR}/WiFi/utility \
-I${ARDUINO_LIB_DIR}/Wire \
-I${ARDUINO_LIB_DIR}/Wire/utility \


FLAGS_GCC = -c -g -Os -Wall -ffunction-sections -fdata-sections -mmcu=${ARDUINO_MODEL} -DF_CPU=16000000L -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=${ARDUINO_VERSION}
FLAGS_GPP = ${FLAGS_GCC} -fno-exceptions
FLAGS_LINKER = lib/arduino.a -Os -Wl,--gc-sections,--relax -mmcu=${ARDUINO_MODEL} -lm

CMD_AVR_GCC = avr-gcc ${FLAGS_GCC} ${INCLUDE}
CMD_AVR_GPP = avr-g++ ${FLAGS_GPP} ${INCLUDE}
CMD_AVR_AR = avr-ar rcs lib/arduino.a


# Environment 
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin


# build
build: .build-post

.build-pre:
# Core:
	${CMD_AVR_GCC} ${ARDUINO_CORE_DIR}/WInterrupts.c -o lib/WInterrupts.c.o 
	${CMD_AVR_GCC} ${ARDUINO_CORE_DIR}/wiring.c -o lib/wiring.c.o 
	${CMD_AVR_GCC} ${ARDUINO_CORE_DIR}/wiring_analog.c -o lib/wiring_analog.c.o 
	${CMD_AVR_GCC} ${ARDUINO_CORE_DIR}/wiring_digital.c -o lib/wiring_digital.c.o 
	${CMD_AVR_GCC} ${ARDUINO_CORE_DIR}/wiring_pulse.c -o lib/wiring_pulse.c.o 
	${CMD_AVR_GCC} ${ARDUINO_CORE_DIR}/wiring_shift.c -o lib/wiring_shift.c.o
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/CDC.cpp -o lib/CDC.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/HardwareSerial.cpp -o lib/HardwareSerial.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/HID.cpp -o lib/HID.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/IPAddress.cpp -o lib/IPAddress.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/main.cpp -o lib/main.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/new.cpp -o lib/new.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/Print.cpp -o lib/Print.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/Stream.cpp -o lib/Stream.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/Tone.cpp -o lib/Tone.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/USBCore.cpp -o lib/USBCore.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/WMath.cpp -o lib/WMath.cpp.o 
	${CMD_AVR_GPP} ${ARDUINO_CORE_DIR}/WString.cpp -o lib/WString.cpp.o 	
	${CMD_AVR_AR} lib/WInterrupts.c.o 
	${CMD_AVR_AR} lib/wiring.c.o 
	${CMD_AVR_AR} lib/wiring_analog.c.o 
	${CMD_AVR_AR} lib/wiring_digital.c.o 
	${CMD_AVR_AR} lib/wiring_pulse.c.o 
	${CMD_AVR_AR} lib/wiring_shift.c.o 
	${CMD_AVR_AR} lib/CDC.cpp.o 
	${CMD_AVR_AR} lib/HardwareSerial.cpp.o 
	${CMD_AVR_AR} lib/HID.cpp.o 
	${CMD_AVR_AR} lib/IPAddress.cpp.o 
	${CMD_AVR_AR} lib/main.cpp.o 
	${CMD_AVR_AR} lib/new.cpp.o 
	${CMD_AVR_AR} lib/Print.cpp.o 
	${CMD_AVR_AR} lib/Stream.cpp.o 
	${CMD_AVR_AR} lib/Tone.cpp.o 
	${CMD_AVR_AR} lib/USBCore.cpp.o 
	${CMD_AVR_AR} lib/WMath.cpp.o 
	${CMD_AVR_AR} lib/WString.cpp.o 	

# SPI:
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/SPI/SPI.cpp -o lib/SPI.cpp.o 
#	${CMD_AVR_AR} lib/SPI.cpp.o 
	
# Ethernet:
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Ethernet/Dhcp.cpp -o lib/Dhcp.cpp.o 
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Ethernet/Dns.cpp -o lib/Dns.cpp.o 
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Ethernet/Ethernet.cpp -o lib/Ethernet.cpp.o 
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Ethernet/EthernetClient.cpp -o lib/EthernetClient.cpp.o 
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Ethernet/EthernetServer.cpp -o lib/EthernetServer.cpp.o 
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Ethernet/EthernetUdp.cpp -o lib/EthernetUdp.cpp.o 
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Ethernet/utility/socket.cpp -o lib/socket.cpp.o 
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Ethernet/utility/w5100.cpp -o lib/w5100.cpp.o 
#	${CMD_AVR_AR} lib/Dhcp.cpp.o 
#	${CMD_AVR_AR} lib/Dns.cpp.o 
#	${CMD_AVR_AR} lib/Ethernet.cpp.o 
#	${CMD_AVR_AR} lib/EthernetClient.cpp.o 
#	${CMD_AVR_AR} lib/EthernetServer.cpp.o 
#	${CMD_AVR_AR} lib/EthernetUdp.cpp.o 
#	${CMD_AVR_AR} lib/socket.cpp.o 
#	${CMD_AVR_AR} lib/w5100.cpp.o 
	
# LiquidCrystal:
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/LiquidCrystal/LiquidCrystal.cpp -o lib/LiquidCrystal.cpp.o
#	${CMD_AVR_AR} lib/LiquidCrystal.cpp.o 
	
# Servo:
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Servo/Servo.cpp -o lib/Servo.cpp.o
#	${CMD_AVR_AR} lib/Servo.cpp.o 

# SoftwareSerial:
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/SoftwareSerial/SoftwareSerial.cpp -o lib/SoftwareSerial.cpp.o 
#	${CMD_AVR_AR} lib/SoftwareSerial.cpp.o
	
# Wire:
#	${CMD_AVR_GCC} ${ARDUINO_LIB_DIR}/Wire/utility/twi.c -o lib/twi.c.o
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/Wire/Wire.cpp -o lib/Wire.cpp.o
#	${CMD_AVR_AR} lib/twi.c.o 
#	${CMD_AVR_AR} lib/Wire.cpp.o 

# MemoryFree (http://playground.arduino.cc/Code/AvailableMemory):
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/MemoryFree/MemoryFree.cpp -o lib/MemoryFree.cpp.o
#	${CMD_AVR_AR} lib/MemoryFree.cpp.o 
	
# RTClib (https://github.com/adafruit/RTClib):
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/RTClib/RTClib.cpp -o lib/RTClib.cpp.o
#	${CMD_AVR_AR} lib/RTClib.cpp.o 
	
# VirtualWire: (http://www.open.com.au/mikem/arduino/VirtualWire)
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/VirtualWire/VirtualWire.cpp -o lib/VirtualWire.cpp.o
#	${CMD_AVR_AR} lib/VirtualWire.cpp.o	

# GSMSHIELD: (https://code.google.com/p/gsm-shield-arduino)
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/GSMSHIELD/call.cpp -o lib/call.cpp.o
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/GSMSHIELD/gps.cpp -o lib/gps.cpp.o
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/GSMSHIELD/GSM.cpp -o lib/GSM.cpp.o
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/GSMSHIELD/HWSerial.cpp -o lib/HWSerial.cpp.o
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/GSMSHIELD/inetGSM.cpp -o lib/inetGSM.cpp.o
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/GSMSHIELD/LOG.cpp -o lib/LOG.cpp.o
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/GSMSHIELD/SIM900.cpp -o lib/SIM900.cpp.o
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/GSMSHIELD/sms.cpp -o lib/sms.cpp.o
#	${CMD_AVR_GPP} ${ARDUINO_LIB_DIR}/GSMSHIELD/WideTextFinder.cpp -o lib/WideTextFinder.cpp.o
#	${CMD_AVR_AR} lib/call.cpp.o
#	${CMD_AVR_AR} lib/gps.cpp.o
#	${CMD_AVR_AR} lib/GSM.cpp.o
#	${CMD_AVR_AR} lib/HWSerial.cpp.o
#	${CMD_AVR_AR} lib/inetGSM.cpp.o
#	${CMD_AVR_AR} lib/LOG.cpp.o
#	${CMD_AVR_AR} lib/SIM900.cpp.o
#	${CMD_AVR_AR} lib/sms.cpp.o
#	${CMD_AVR_AR} lib/WideTextFinder.cpp.o
	
.build-post: .build-impl	
	avr-objcopy -O ihex ${CND_ARTIFACT_PATH_${CONF}} ${CND_ARTIFACT_PATH_${CONF}}.hex
	avr-size --mcu=${ARDUINO_MODEL} -C ${CND_ARTIFACT_PATH_${CONF}}

#upload: .build-post
upload: 
	# UNO
	# avrdude -C ${ARDUINO_BASE_DIR}/hardware/tools/avr/etc/avrdude.conf -v -p${ARDUINO_MODEL} -c${ARDUINO_PROGRAMMER} -P ${COM_PORT} -b115200 -D -Uflash:w:${CND_ARTIFACT_PATH_${CONF}}.hex:i
	# Arduino Mini v3.0
	avrdude -C ${ARDUINO_BASE_DIR}/hardware/tools/avr/etc/avrdude.conf -v -v -v -v -p${ARDUINO_MODEL} -c${ARDUINO_PROGRAMMER} -P ${COM_PORT} -b57600 -D -Uflash:w:${CND_ARTIFACT_PATH_${CONF}}.hex:i

# clean
clean: .clean-post
	rm -f lib/*

.clean-pre:
# Add your pre 'clean' code here...

.clean-post: .clean-impl
# Add your post 'clean' code here...


# clobber
clobber: .clobber-post

.clobber-pre:
# Add your pre 'clobber' code here...

.clobber-post: .clobber-impl
# Add your post 'clobber' code here...


# all
all: .all-post

.all-pre:
# Add your pre 'all' code here...

.all-post: .all-impl
# Add your post 'all' code here...


# build tests
build-tests: .build-tests-post

.build-tests-pre:
# Add your pre 'build-tests' code here...

.build-tests-post: .build-tests-impl
# Add your post 'build-tests' code here...


# run tests
test: .test-post

.test-pre: build-tests
# Add your pre 'test' code here...

.test-post: .test-impl
# Add your post 'test' code here...


# help
help: .help-post

.help-pre:
# Add your pre 'help' code here...

.help-post: .help-impl
# Add your post 'help' code here...


# include project implementation makefile
include nbproject/Makefile-impl.mk

# include project make variables
include nbproject/Makefile-variables.mk
