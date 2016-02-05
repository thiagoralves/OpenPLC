# OPLC Compiler
Command line ladder compiler for the OpenPLC. 

This program is a ladder logic compiler to ANSI C code. The user draws 
a ladder diagram with an appropriately constrained 'schematic editor' 
(e.g. LDmicro) and it will generate ANSI C code that performs the logical
operations described in the ladder diagram. 

The idea is that the generated .cpp file can be used to compile a program
to any embedded platform in order for it to work as a real PLC. The 
OpenPLC Project aims at creating this layer from the .cpp to the working
program to diverse platforms, such as Arduino, Raspberry Pi and ESP8266.

Built using Code Blocks on a Linux machine. Just open the Code Blocks 
Project file and you will be fine. I tryed compiling it on Visual C++ 
Community 2015 on Windows and it compiled perfectly as well.

This code is intended to be multi platform, which means that it should
compile on anything that has a C++ compiler. Basically it uses only
standard ANSI C libraries and types.

USAGE: OPLC_Compiler file.ld file.cpp

For more info check www.openplcproject.com
