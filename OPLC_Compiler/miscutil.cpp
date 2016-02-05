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
// Miscellaneous utility functions that don't fit anywhere else. Error and debug
// functions and memory allocations.
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

using namespace std;

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "oplc_compiler.h"

//-----------------------------------------------------------------------------
// For error messages to the user; printf-like
//-----------------------------------------------------------------------------
void Error(char *str, ...)
{
	va_list f;
    char buf[1024];
    va_start(f, str);
    vsprintf(buf, str, f);

	// Indicate that it's an error, plus the output filename
	char str_err[MAX_PATH+100];
	sprintf(str_err, "compile error ('%s'): ", CurrentCompileFile);

	cout << str_err << endl << buf << endl;
}

//-----------------------------------------------------------------------------
// printf-like debug function
//-----------------------------------------------------------------------------
void dbp(char *str, ...)
{
	va_list f;
    char buf[1024];
    va_start(f, str);
    vsprintf(buf, str, f);

    cout << buf << endl;
}

//-----------------------------------------------------------------------------
// Check the consistency of the heap on which all the PLC program stuff is
// stored.
//-----------------------------------------------------------------------------
void CheckHeap(char *file, int line)
{
	//Should periodically check the heap using the HeapValidate() Win32API.
	//For other systems, we shouldn't do anything
}


//-----------------------------------------------------------------------------
// Like malloc/free, but memsets memory allocated to all zeros. Also TODO some
// checking and something sensible if it fails.
//-----------------------------------------------------------------------------
void *CheckMalloc(size_t n)
{
	ok();
	void *p = malloc(n);
	memset(p, 0, n); //initialize the memory block
    return p;
}
void CheckFree(void *p)
{
	ok();
	free(p);
}
