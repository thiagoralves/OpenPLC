//-----------------------------------------------------------------------------
// Copyright 2015 Thiago Alves
//
// Based on the LDmicro software by Jonathan Westhues
// This file is part of OPLC Compiler.
//
// OPLC Compiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OPLC Compiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OPLC Compiler.  If not, see <http://www.gnu.org/licenses/>.
//------
//
// A ladder logic compiler to ANSI C code. User draws a ladder diagram
// with an appropriately constrained 'schematic editor' (e.g. LDmicro).
// This program will generate ANSI C code that performs the logical
// operations described in the ladder diagram. This file contains the
// program entry point.
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

using namespace std;

#include <iostream>
#include <string.h>
#include <stdlib.h>

#include "oplc_compiler.h"
#include "mcutable.h"

char CurrentSaveFile[MAX_PATH];
static BOOL ProgramChangedNotSaved = FALSE;
char CurrentCompileFile[MAX_PATH];

// Everything relating to the PLC's program, I/O configuration, processor
// choice, and so on--basically everything that would be saved in the
// project file.
PlcProgram Prog;


static void CompileProgram()
{
	//OpenPLC changes
	Prog.mcu=&SupportedMcus[ISA_ANSIC]; //force the MCU to ANSI C code
	Prog.mcu->whichIsa = ISA_ANSIC;
	Prog.cycleTime = 50000; //force the cycle time to 50ms

    if(!GenerateIntermediateCode()) return;

    if(Prog.mcu == NULL)
    {
        Error("Must choose a target microcontroller before compiling.");
        return;
    }

    if(UartFunctionUsed() && Prog.mcu->uartNeeds.rxPin == 0)
    {
        Error("UART function used but not supported for this micro.");
        return;
    }

    if(PwmFunctionUsed() && Prog.mcu->pwmNeedsPin == 0)
    {
        Error("PWM function used but not supported for this micro.");
        return;
    }

	CompileAnsiC(CurrentCompileFile);
}


int main(int argc, char* argv[])
{
	cout <<
"================================================================\r\n\
OPLC Compiler v0.1a\r\n\
Copyright (C) 2015  Thiago Alves\r\n\
Homepage: www.openplcproject.com\r\n\
\r\n\
This program is distributed in the hope that it will be useful,\r\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\r\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\r\n\
GNU General Public License v3 for more details.\r\n\
================================================================" << endl << endl;


	char *err = "Bad command line arguments: run 'oplc_compiler src.ld [dest.cpp]'\r\n                                                           ^\r\n                                                       Optional\r\n";
	char *source, *dest;
	if (argc < 2) { Error(err); exit(-1); }

	//Define source and destination according to the number of arguments
	source = new char[1000]; //Windows couldn't handle the size of argv[1] with drag and drop properly
	strcpy(source, argv[1]);
	dest = new char[12];
	strcpy(dest, "ladder.cpp");
	if (argc > 2)
	{
		free(dest);
		dest = new char[sizeof(argv[2])];
		strcpy(dest, argv[2]);
	}

	cout << "Reading file \"" << source << "\"...";

	if(!LoadProjectFromFile(source))
	{
		cout << endl;
		Error("Couldn't open '%s'", source);
		exit(-1);
	}
	cout << "DONE!" << endl;
	strcpy(CurrentCompileFile, dest);

	cout << "Generating IO List...";
	GenerateIoList(-1);
	cout << "DONE!" << endl;

	cout << "Compiling ladder program to ANSI C...";
	CompileProgram();
	cout << "DONE!" << endl << endl;

	cout <<
"Compile successful! Wrote C source code to \"" << dest << "\".\r\n\r\n\
This is not a complete C program. You have to provide the runtime\r\n\
and all the I/O routines. See the comments in the source code for\r\n\
information about how to do this." << endl << endl;
	exit(0);
}
