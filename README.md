# OpenPLC
This program is intended to emulate a PLC on a Linux machine. This virtual PLC uses the OpenPLC Software Stack to interpret ladder diagrams and reply to MODBUS/TCP requests. Ladder diagrams created using the OpenPLC Ladder IDE or ldmicro can be uploaded to this virtual PLC.

The OpenPLC has different hardware layers to support physical devices. More hardware layers can be added to the project. For instance, there is a hardware layer for the RaspberryPi, which makes the OpenPLC controls its IO pins. 

There is a NodeJS application that works as a http server for the user to upload ladder diagrams.

You must have NodeJS and WiringPi (in case you are using Raspberry Pi) installed to use this program. Usage:

1) ./build.sh

2) sudo node server.js [or sudo node server_rpi.js (for Raspberry Pi)]

A server will be created on port 8080. Just open your browser and navigate to localhost:8080. After the application is running, you can connect to the virtual PLC using any MODBUS/TCP HMI software.

