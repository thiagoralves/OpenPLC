//-----------------------------------------------------------------------------
// ORIGINAL LDMICRO CODE WRITTEN BY JONATHAN WESTHUES
//-----------------------------------------------------------------------------
// Copyright 2007 Jonathan Westhues
//
// This file is part of LDmicro.
//
// LDmicro is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LDmicro is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LDmicro.  If not, see <http://www.gnu.org/licenses/>.
//------
//
// Write the program as ANSI C source. This is very simple, because the
// intermediate code structure is really a lot like C. Someone else will be
// responsible for calling us with appropriate timing.
// Jonathan Westhues, Oct 2004
//-----------------------------------------------------------------------------
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>

#include "oplc_compiler.h"
#include "intcode.h"

static char SeenVariables[MAX_IO][MAX_NAME_LEN];
int SeenVariablesCount;

//-----------------------------------------------------------------------------
// Variables targetting OpenPLC buffers must have the name format as
// {X,Y,I,O,M}0_0, where the first number indicates the buffer number and the
// second indicates the position on the buffer. This function parses the
// variable name and extracts the buffer number and position
//-----------------------------------------------------------------------------
static void getBufferNumberAndPosition(char *var, int *b_number, int *b_pos)
{
	char temp[10];
	temp[0] = var[6];
	temp[1] = '\0';
	*b_number = atoi(temp);

	int i = 8, j = 0;
	while (var[i] != '\0')
	{
		temp[j] = var[i];
		j++;
		temp[j] = '\0';
		i++;
	}
	*b_pos = atoi(temp)%100;
}

//-----------------------------------------------------------------------------
// Have we seen a variable before? If not then no need to generate code for
// it, otherwise we will have to make a declaration, and mark it as seen.
//-----------------------------------------------------------------------------
static BOOL SeenVariable(char *name)
{
    int i;

    for(i = 0; i < SeenVariablesCount; i++)
    {
        if(strcmp(SeenVariables[i], name)==0)
        {
            return TRUE;
        }
    }
    if(i >= MAX_IO) oops();
    strcpy(SeenVariables[i], name);
    SeenVariablesCount++;
    return FALSE;
}

//-----------------------------------------------------------------------------
// Turn an internal symbol into a C name; only trick is that internal symbols
// use $ for symbols that the int code generator needed for itself, so map
// that into something okay for C.
//-----------------------------------------------------------------------------
#define ASBIT 1
#define ASINT 2
static char *MapSym(char *str, int how)
{
    if(!str) return NULL;

    static char AllRets[16][MAX_NAME_LEN+30];
    static int RetCnt;

    RetCnt = (RetCnt + 1) & 15;

    char *ret = AllRets[RetCnt];

    // The namespace for bit and integer variables is distinct.
    char bit_int;
    if(how == ASBIT) {
        bit_int = 'b';
    } else if(how == ASINT) {
        bit_int = 'i';
    } else {
        oops();
    }

    // User and internal symbols are distinguished.
    if(*str == '$')
    {
		if (*str+1 == 'M' || *str+1 == 'X' || *str+1 == 'Y' || *str+1 == 'I' || *str+1 == 'O')
		{
			//If the variable starts with M, X, Y, I or O it is related to an internal buffer,
			//therefore a pointer must be created instead
			sprintf(ret, "*I_%c_%s", bit_int, str+1);
		}
		else
		{
			sprintf(ret, "I_%c_%s", bit_int, str+1);
		}
    }
    else
    {
		if (*str == 'M' || *str == 'X' || *str == 'Y' || *str == 'I' || *str == 'O')
		{
			//If the variable starts with M, X, Y, I or O it is related to an internal buffer,
			//therefore a pointer must be created instead
			sprintf(ret, "*U_%c_%s", bit_int, str);
		}
		else
		{
			sprintf(ret, "U_%c_%s", bit_int, str);
		}
    }
    return ret;
}

//-----------------------------------------------------------------------------
// Generate a declaration for an integer var; If the variable is mapped to an
// internal buffer, a pointer must be created to the buffer instead. In case it
// is just a normal variable, just create a static 16-bit qty.
//-----------------------------------------------------------------------------
static void DeclareInt(FILE *f, char *str)
{
	// The mapped symbol has the form *U_i_{name}, or U_i_{name}. The star
    // determines if it's a reference to an internal buffer or not.
	if (str[0] == '*')
	{
		//This is a pointer to a buffer area
		int bufferNumber=0, position=0;
		getBufferNumberAndPosition(str, &bufferNumber, &position);

		// The mapped symbol has the form *U_i_{X,Y,R}name, so look at character
		// five to determine if it's an analog input, analog output or internal
		// register (memory).
		if (str[5] == 'I')
		{
			fprintf(f, "STATIC SWORD %s = &AnalogInputBuffer%d[%d];\n", str, bufferNumber, position);
		}
		if (str[5] == 'O')
		{
			fprintf(f, "STATIC SWORD %s = &AnalogOutputBuffer%d[%d];\n", str, bufferNumber, position);
		}
		if (str[5] == 'M')
		{
			fprintf(f, "STATIC SWORD %s = &MemBuffer%d[%d];\n", str, bufferNumber, position);
		}
	}

	else
	{
		fprintf(f, "STATIC SWORD %s = 0;\n", str);
	}
}

//-----------------------------------------------------------------------------
// Generate a declaration for a bit var; three cases, input, output, and
// internal relay. An internal relay is just a BOOL variable, but for an
// input or an output, pointers are created for the respective internal buffers.
// Someone else must update these buffers to reflect the actual I/O state.
//-----------------------------------------------------------------------------
static void DeclareBit(FILE *f, char *str)
{
    // The mapped symbol has the form *U_b_{name}, or U_b_{name}. The star
    // determines if it's a reference to an internal register or not.
    if (str[0] == '*')
    {
		//This is a pointer to a buffer area
		int bufferNumber = 0, position = 0;
		getBufferNumberAndPosition(str, &bufferNumber, &position);

		// The mapped symbol has the form *U_b_{X,Y,R}name, so look at character
		// five to determine if it's an input, output, internal relay.
		if (str[5] == 'X')
		{
			fprintf(f, "STATIC BOOL %s = &DiscreteInputBuffer%d[%d];\n", str, bufferNumber, position);
		}
		if (str[5] == 'Y')
		{
			fprintf(f, "STATIC BOOL %s = &CoilsBuffer%d[%d];\n", str, bufferNumber, position);
		}
	}

	else
	{
		fprintf(f, "STATIC BOOL %s = 0;\n", str);
	}
}

//-----------------------------------------------------------------------------
// Generate declarations for all the 16-bit/single bit variables in the ladder
// program.
//-----------------------------------------------------------------------------
static void GenerateDeclarations(FILE *f)
{
    int i;
    for(i = 0; i < IntCodeLen; i++) {
        char *bitVar1 = NULL, *bitVar2 = NULL;
        char *intVar1 = NULL, *intVar2 = NULL, *intVar3 = NULL;

        switch(IntCode[i].op) {
            case INT_SET_BIT:
            case INT_CLEAR_BIT:
                bitVar1 = IntCode[i].name1;
                break;

            case INT_COPY_BIT_TO_BIT:
                bitVar1 = IntCode[i].name1;
                bitVar2 = IntCode[i].name2;
                break;

            case INT_SET_VARIABLE_TO_LITERAL:
                intVar1 = IntCode[i].name1;
                break;

            case INT_SET_VARIABLE_TO_VARIABLE:
                intVar1 = IntCode[i].name1;
                intVar2 = IntCode[i].name2;
                break;

            case INT_SET_VARIABLE_DIVIDE:
            case INT_SET_VARIABLE_MULTIPLY:
            case INT_SET_VARIABLE_SUBTRACT:
            case INT_SET_VARIABLE_ADD:
                intVar1 = IntCode[i].name1;
                intVar2 = IntCode[i].name2;
                intVar3 = IntCode[i].name3;
                break;

            case INT_INCREMENT_VARIABLE:
            case INT_READ_ADC:
            case INT_SET_PWM:
                intVar1 = IntCode[i].name1;
                break;

            case INT_UART_RECV:
            case INT_UART_SEND:
                intVar1 = IntCode[i].name1;
                bitVar1 = IntCode[i].name2;
                break;

            case INT_IF_BIT_SET:
            case INT_IF_BIT_CLEAR:
                bitVar1 = IntCode[i].name1;
                break;

            case INT_IF_VARIABLE_LES_LITERAL:
                intVar1 = IntCode[i].name1;
                break;

            case INT_IF_VARIABLE_EQUALS_VARIABLE:
            case INT_IF_VARIABLE_GRT_VARIABLE:
                intVar1 = IntCode[i].name1;
                intVar2 = IntCode[i].name2;
                break;

            case INT_END_IF:
            case INT_ELSE:
            case INT_COMMENT:
            case INT_SIMULATE_NODE_STATE:
            case INT_EEPROM_BUSY_CHECK:
            case INT_EEPROM_READ:
            case INT_EEPROM_WRITE:
                break;

            default:
                oops();
        }
        bitVar1 = MapSym(bitVar1, ASBIT);
        bitVar2 = MapSym(bitVar2, ASBIT);

        intVar1 = MapSym(intVar1, ASINT);
        intVar2 = MapSym(intVar2, ASINT);
        intVar3 = MapSym(intVar3, ASINT);

        if(bitVar1 && !SeenVariable(bitVar1)) DeclareBit(f, bitVar1);
        if(bitVar2 && !SeenVariable(bitVar2)) DeclareBit(f, bitVar2);

        if(intVar1 && !SeenVariable(intVar1)) DeclareInt(f, intVar1);
        if(intVar2 && !SeenVariable(intVar2)) DeclareInt(f, intVar2);
        if(intVar3 && !SeenVariable(intVar3)) DeclareInt(f, intVar3);
    }
}

//-----------------------------------------------------------------------------
// Actually generate the C source for the program.
//-----------------------------------------------------------------------------
static void GenerateAnsiC(FILE *f)
{
    int i;
    int indent = 1;
    for(i = 0; i < IntCodeLen; i++) {

        if(IntCode[i].op == INT_END_IF) indent--;
        if(IntCode[i].op == INT_ELSE) indent--;

        int j;
        for(j = 0; j < indent; j++) fprintf(f, "    ");

        switch(IntCode[i].op) {
            case INT_SET_BIT:
                fprintf(f, "%s = 1;\n", MapSym(IntCode[i].name1, ASBIT));
                break;

            case INT_CLEAR_BIT:
                fprintf(f, "%s = 0;\n", MapSym(IntCode[i].name1, ASBIT));
                break;

            case INT_COPY_BIT_TO_BIT:
                fprintf(f, "%s = %s;\n",
                    MapSym(IntCode[i].name1, ASBIT),
                    MapSym(IntCode[i].name2, ASBIT));
                break;

            case INT_SET_VARIABLE_TO_LITERAL:
                fprintf(f, "%s = %d;\n", MapSym(IntCode[i].name1, ASINT),
                    IntCode[i].literal);
                break;

            case INT_SET_VARIABLE_TO_VARIABLE:
                fprintf(f, "%s = %s;\n", MapSym(IntCode[i].name1, ASINT),
                                         MapSym(IntCode[i].name2, ASINT));
                break;

            {
                char op;
                case INT_SET_VARIABLE_ADD: op = '+'; goto arith;
                case INT_SET_VARIABLE_SUBTRACT: op = '-'; goto arith;
                case INT_SET_VARIABLE_MULTIPLY: op = '*'; goto arith;
                case INT_SET_VARIABLE_DIVIDE: op = '/'; goto arith;
                arith:
                    fprintf(f, "%s = %s %c %s;\n",
                        MapSym(IntCode[i].name1, ASINT),
                        MapSym(IntCode[i].name2, ASINT),
                        op,
                        MapSym(IntCode[i].name3, ASINT) );
                    break;
            }

            case INT_INCREMENT_VARIABLE:
                fprintf(f, "%s++;\n", MapSym(IntCode[i].name1, ASINT));
                break;

            case INT_IF_BIT_SET:
                fprintf(f, "if(%s) {\n",
                    MapSym(IntCode[i].name1, ASBIT));
                indent++;
                break;

            case INT_IF_BIT_CLEAR:
                fprintf(f, "if(!%s) {\n",
                    MapSym(IntCode[i].name1, ASBIT));
                indent++;
                break;

            case INT_IF_VARIABLE_LES_LITERAL:
                fprintf(f, "if(%s < %d) {\n", MapSym(IntCode[i].name1, ASINT),
                    IntCode[i].literal);
                indent++;
                break;

            case INT_IF_VARIABLE_EQUALS_VARIABLE:
                fprintf(f, "if(%s == %s) {\n", MapSym(IntCode[i].name1, ASINT),
                                               MapSym(IntCode[i].name2, ASINT));
                indent++;
                break;

            case INT_IF_VARIABLE_GRT_VARIABLE:
                fprintf(f, "if(%s > %s) {\n", MapSym(IntCode[i].name1, ASINT),
                                              MapSym(IntCode[i].name2, ASINT));
                indent++;
                break;

            case INT_END_IF:
                fprintf(f, "}\n");
                break;

            case INT_ELSE:
                fprintf(f, "} else {\n"); indent++;
                break;

            case INT_SIMULATE_NODE_STATE:
                // simulation-only
                fprintf(f, "\n");
                break;

            case INT_COMMENT:
                if(IntCode[i].name1[0]) {
                    fprintf(f, "/* %s */\n", IntCode[i].name1);
                } else {
                    fprintf(f, "\n");
                }
                break;

            case INT_EEPROM_BUSY_CHECK:
            case INT_EEPROM_READ:
            case INT_EEPROM_WRITE:
            case INT_READ_ADC:
            case INT_SET_PWM:
            case INT_UART_RECV:
            case INT_UART_SEND:
                Error("ANSI C target does not support peripherals "
                    "(UART, PWM, ADC, EEPROM). Skipping that instruction.");
                break;

            default:
                oops();
        }
    }
}

void CompileAnsiC(char *dest)
{
    SeenVariablesCount = 0;

    FILE *f = fopen(dest, "w");
    if(!f) {
        Error("Couldn't open file '%s'", dest);
        return;
    }

    fprintf(f,
"/* This is auto-generated code from OPLC_Compiler. Do not edit this file!\n"
"   Go back to the ladder diagram source for changes in the logic, and make\n"
"   any C additions either in ladder.h or in additional .c files linked\n"
"   against this one. */\n"
"\n"
"/* You must provide ladder.h; there you must provide:\n"
"      * a typedef for SWORD and BOOL, signed 16 bit and boolean types\n"
"        (probably typedef signed short SWORD; typedef unsigned char BOOL;)\n"
"\n"
"   You must also provide implementations of all the I/O read/write\n"
"   either as inlines in the header file or in another source file. (The\n"
"   I/O buffers are all declared here, but you must update them somewhere\n"
"   else to reflect the actual IO state)\n"
"\n"
"   See the generated source code (below) for function names. */\n"
"#include \"ladder.h\"\n"
"\n"
"/* Define EXTERN_EVERYTHING in ladder.h if you want all symbols extern.\n"
"   This could be useful to implement 'magic variables', so that for\n"
"   example when you write to the ladder variable duty_cycle, your PLC\n"
"   runtime can look at the C variable U_duty_cycle and use that to set\n"
"   the PWM duty cycle on the micro. That way you can add support for\n"
"   peripherals that OPLC_Compiler doesn't know about. */\n"
"#ifdef EXTERN_EVERYTHING\n"
"#define STATIC \n"
"#else\n"
"#define STATIC static\n"
"#endif\n"
"\n"
"/* Define NO_PROTOTYPES if you don't want OPLC_Compiler to provide prototypes\n"
"   for all the I/O functions (Read_U_xxx, Write_U_xxx) that you must provide.\n"
"   If you define this then you must provide your own prototypes for these\n"
"   functions in ladder.h, or provide definitions (e.g. as inlines or macros)\n"
"   for them in ladder.h. */\n"
"#ifdef NO_PROTOTYPES\n"
"#define PROTO(x)\n"
"#else\n"
"#define PROTO(x) x\n"
"#endif\n"
"\n"
"/* Below is the definition of the internal buffers for I/O and memory.*/\n"
"SWORD AnalogInputBuffer0[BUFFER_SIZE];\n"
"SWORD AnalogOutputBuffer0[BUFFER_SIZE];\n"
"BOOL DiscreteInputBuffer0[BUFFER_SIZE];\n"
"BOOL CoilsBuffer0[BUFFER_SIZE];\n"
"\n"
"SWORD MemBuffer0[BUFFER_SIZE];\n"
"SWORD MemBuffer1[BUFFER_SIZE];\n"
"SWORD MemBuffer2[BUFFER_SIZE];\n"
"\n"
"/* U_xxx symbols correspond to user-defined names. There is such a symbol\n"
"   for every internal relay, variable, timer, and so on in the ladder\n"
"   program. I_xxx symbols are internally generated. */\n"
        );

    // now generate declarations for all variables
    GenerateDeclarations(f);

    fprintf(f,
"\n"
"\n"
"/* Call this function once per PLC cycle. You are responsible for calling\n"
"   it at the interval that you specified in the MCU configuration when you\n"
"   generated this code. */\n"
"void PlcCycle(void)\n"
"{\n"
        );

    GenerateAnsiC(f);

    fprintf(f, "}\n");
    fclose(f);
}
