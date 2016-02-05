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
// Load the circuit from the .ld file (written in a nice ASCII format).
// The loaded objects are stored in the PlcProgram data structure
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

using namespace std;

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "oplc_compiler.h"

static ElemSubcktSeries *LoadSeriesFromFile(FILE *f);

//-----------------------------------------------------------------------------
// Check a line of text from a saved project file to determine whether it
// contains a leaf element (coil, contacts, etc.). If so, create an element
// for and save that in *any and *which, and return TRUE, else return FALSE.
//-----------------------------------------------------------------------------
static BOOL LoadLeafFromFile(char *line, void **any, int *which)
{
    ElemLeaf *l = AllocLeaf();
    int x;

    if(memcmp(line, "COMMENT", 7)==0)
    {
        char *s = line + 8;
        int i = 0;
        while(*s && *s != '\n')
        {
            if(*s == '\\')
            {
                if(s[1] == 'n')
                {
                    l->d.comment.str[i++] = '\n';
                    s++;
                }
                else if(s[1] == 'r')
                {
                    l->d.comment.str[i++] = '\r';
                    s++;
                }
                else if(s[1] == '\\')
                {
                    l->d.comment.str[i++] = '\\';
                    s++;
                }
                else
                {
                    // that is odd
                }
            }

            else
            {
                l->d.comment.str[i++] = *s;
            }
            s++;
        }

        l->d.comment.str[i++] = '\0';
        *which = ELEM_COMMENT;
    }
    else if(sscanf(line, "CONTACTS %s %d", l->d.contacts.name, &l->d.contacts.negated)==2)
    {
        *which = ELEM_CONTACTS;
    }
    else if(sscanf(line, "COIL %s %d %d %d", l->d.coil.name, &l->d.coil.negated, &l->d.coil.setOnly, &l->d.coil.resetOnly)==4)
    {
        *which = ELEM_COIL;
    }
    else if(memcmp(line, "PLACEHOLDER", 11)==0)
    {
        *which = ELEM_PLACEHOLDER;
    }
    else if(memcmp(line, "SHORT", 5)==0)
    {
        *which = ELEM_SHORT;
    }
    else if(memcmp(line, "OPEN", 4)==0)
    {
        *which = ELEM_OPEN;
    }
    else if(memcmp(line, "MASTER_RELAY", 12)==0)
    {
        *which = ELEM_MASTER_RELAY;
    }
    else if(sscanf(line, "SHIFT_REGISTER %s %d", l->d.shiftRegister.name, &(l->d.shiftRegister.stages))==2)
    {
        *which = ELEM_SHIFT_REGISTER;
    }
    else if(memcmp(line, "OSR", 3)==0)
    {
        *which = ELEM_ONE_SHOT_RISING;
    }
    else if(memcmp(line, "OSF", 3)==0)
    {
        *which = ELEM_ONE_SHOT_FALLING;
    }
    else if((sscanf(line, "TON %s %d", l->d.timer.name, &l->d.timer.delay)==2))
    {
        *which = ELEM_TON;
    }
    else if((sscanf(line, "TOF %s %d", l->d.timer.name, &l->d.timer.delay)==2))
    {
        *which = ELEM_TOF;
    }
    else if((sscanf(line, "RTO %s %d", l->d.timer.name, &l->d.timer.delay)==2))
    {
        *which = ELEM_RTO;
    }
    else if((sscanf(line, "CTD %s %d", l->d.counter.name, &l->d.counter.max)==2))
    {
        *which = ELEM_CTD;
    }
    else if((sscanf(line, "CTU %s %d", l->d.counter.name, &l->d.counter.max)==2))
    {
        *which = ELEM_CTU;
    }
    else if((sscanf(line, "CTC %s %d", l->d.counter.name, &l->d.counter.max)==2))
    {
        *which = ELEM_CTC;
    }
    else if(sscanf(line, "RES %s", l->d.reset.name)==1)
    {
        *which = ELEM_RES;
    }
    else if(sscanf(line, "MOVE %s %s", l->d.move.dest, l->d.move.src)==2)
    {
        *which = ELEM_MOVE;
    }
    else if(sscanf(line, "ADD %s %s %s", l->d.math.dest, l->d.math.op1, l->d.math.op2)==3)
    {
        *which = ELEM_ADD;
    }
    else if(sscanf(line, "SUB %s %s %s", l->d.math.dest, l->d.math.op1, l->d.math.op2)==3)
    {
        *which = ELEM_SUB;
    }
    else if(sscanf(line, "MUL %s %s %s", l->d.math.dest, l->d.math.op1, l->d.math.op2)==3)
    {
        *which = ELEM_MUL;
    }
    else if(sscanf(line, "DIV %s %s %s", l->d.math.dest, l->d.math.op1, l->d.math.op2)==3)
    {
        *which = ELEM_DIV;
    }
    else if(sscanf(line, "EQU %s %s", l->d.cmp.op1, l->d.cmp.op2)==2)
    {
        *which = ELEM_EQU;
    }
    else if(sscanf(line, "NEQ %s %s", l->d.cmp.op1, l->d.cmp.op2)==2)
    {
        *which = ELEM_NEQ;
    }
    else if(sscanf(line, "GRT %s %s", l->d.cmp.op1, l->d.cmp.op2)==2)
    {
        *which = ELEM_GRT;
    }
    else if(sscanf(line, "GEQ %s %s", l->d.cmp.op1, l->d.cmp.op2)==2)
    {
        *which = ELEM_GEQ;
    }
    else if(sscanf(line, "LEQ %s %s", l->d.cmp.op1, l->d.cmp.op2)==2)
    {
        *which = ELEM_LEQ;
    }
    else if(sscanf(line, "LES %s %s", l->d.cmp.op1, l->d.cmp.op2)==2)
    {
        *which = ELEM_LES;
    }
    else if(sscanf(line, "READ_ADC %s", l->d.readAdc.name)==1)
    {
        *which = ELEM_READ_ADC;
    }
    else if(sscanf(line, "SET_PWM %s %d", l->d.setPwm.name, &(l->d.setPwm.targetFreq))==2)
    {
        *which = ELEM_SET_PWM;
    }
    else if(sscanf(line, "UART_RECV %s", l->d.uart.name)==1)
    {
        *which = ELEM_UART_RECV;
    }
    else if(sscanf(line, "UART_SEND %s", l->d.uart.name)==1)
    {
        *which = ELEM_UART_SEND;
    }
    else if(sscanf(line, "PERSIST %s", l->d.persist.var)==1)
    {
        *which = ELEM_PERSIST;
    }
    else if(sscanf(line, "FORMATTED_STRING %s %d", l->d.fmtdStr.var, &x)==2)
    {
        if(strcmp(l->d.fmtdStr.var, "(none)")==0)
        {
            strcpy(l->d.fmtdStr.var, "");
        }

        char *p = line;
        int i;
        for(i = 0; i < 3; i++)
        {
            while(!isspace(*p)) p++;
            while( isspace(*p)) p++;
        }
        for(i = 0; i < x; i++)
        {
            l->d.fmtdStr.string[i] = atoi(p);
            if(l->d.fmtdStr.string[i] < 32)
            {
                l->d.fmtdStr.string[i] = 'X';
            }
            while(!isspace(*p) && *p) p++;
            while( isspace(*p) && *p) p++;
        }
        l->d.fmtdStr.string[i] = '\0';

        *which = ELEM_FORMATTED_STRING;
    }
    else if(sscanf(line, "LOOK_UP_TABLE %s %s %d %d", l->d.lookUpTable.dest, l->d.lookUpTable.index, &(l->d.lookUpTable.count), &(l->d.lookUpTable.editAsString))==4)
    {
        char *p = line;
        int i;
        // First skip over the parts that we already sscanf'd.
        for(i = 0; i < 5; i++)
        {
            while((!isspace(*p)) && *p)
                p++;
            while(isspace(*p) && *p)
                p++;
        }
        // Then copy over the look-up table entries.
        for(i = 0; i < l->d.lookUpTable.count; i++)
        {
            l->d.lookUpTable.vals[i] = atoi(p);
            while((!isspace(*p)) && *p)
                p++;
            while(isspace(*p) && *p)
                p++;
        }
        *which = ELEM_LOOK_UP_TABLE;
    }
    else if(sscanf(line, "PIECEWISE_LINEAR %s %s %d", l->d.piecewiseLinear.dest, l->d.piecewiseLinear.index, &(l->d.piecewiseLinear.count))==3)
    {
        char *p = line;
        int i;
        // First skip over the parts that we already sscanf'd.
        for(i = 0; i < 4; i++)
        {
            while((!isspace(*p)) && *p)
                p++;
            while(isspace(*p) && *p)
                p++;
        }
        // Then copy over the piecewise linear points.
        for(i = 0; i < l->d.piecewiseLinear.count*2; i++)
        {
            l->d.piecewiseLinear.vals[i] = atoi(p);
            while((!isspace(*p)) && *p)
                p++;
            while(isspace(*p) && *p)
                p++;
        }
        *which = ELEM_PIECEWISE_LINEAR;
    }
    else
    {
        // that's odd; nothing matched
        CheckFree(l);
        return FALSE;
    }
    *any = l;
    return TRUE;
}

//-----------------------------------------------------------------------------
// Load a parallel subcircuit from a file. We look for leaf nodes using
// LoadLeafFromFile, which we can put directly into the parallel circuit
// that we're building up, or series subcircuits that we can pass to
// LoadSeriesFromFile. Returns the parallel subcircuit built up, or NULL if
// something goes wrong.
//-----------------------------------------------------------------------------
static ElemSubcktParallel *LoadParallelFromFile(FILE *f)
{
    char line[512];
    void *any;
    int which;

    ElemSubcktParallel *ret = AllocSubcktParallel();
    int cnt = 0;

    for(;;)
    {
        if(!fgetsNoCR(line, sizeof(line), f)) return NULL;
        char *s = line;
        while(isspace(*s)) s++;

        if(strcmp(s, "SERIES\n")==0)
        {
            which = ELEM_SERIES_SUBCKT;
            any = LoadSeriesFromFile(f);
            if(!any) return NULL;

        }
        else if(LoadLeafFromFile(s, &any, &which))
        {
            // got it
        }
        else if(strcmp(s, "END\n")==0)
        {
            ret->count = cnt;
            return ret;
        }
        else
        {
            return NULL;
        }
        ret->contents[cnt].which = which;
        ret->contents[cnt].d.any = any;
        cnt++;
        if(cnt >= MAX_ELEMENTS_IN_SUBCKT) return NULL;
    }
}


//-----------------------------------------------------------------------------
// Same as LoadParallelFromFile, but for a series subcircuit. Thus builds
// a series circuit out of parallel circuits and leaf elements.
//-----------------------------------------------------------------------------
static ElemSubcktSeries *LoadSeriesFromFile(FILE *f)
{
    char line[512];
    void *any;
    int which;

    ElemSubcktSeries *ret = AllocSubcktSeries();
    int cnt = 0;

    for(;;)
    {
        if(!fgetsNoCR(line, sizeof(line), f)) return NULL;
        char *s = line;
        while(isspace(*s)) s++;

        if(strcmp(s, "PARALLEL\n")==0)
        {
            which = ELEM_PARALLEL_SUBCKT;
            any = LoadParallelFromFile(f);
            if(!any) return NULL;

        }
        else if(LoadLeafFromFile(s, &any, &which))
        {
            // got it
        }
        else if(strcmp(s, "END\n")==0)
        {
            ret->count = cnt;
            return ret;
        }
        else
        {
            return NULL;
        }

        ret->contents[cnt].which = which;
        ret->contents[cnt].d.any = any;
        cnt++;
        if(cnt >= MAX_ELEMENTS_IN_SUBCKT) return NULL;
    }
}

//-----------------------------------------------------------------------------
// Corrects the end of line. If it reads with \r\n, it will remove the \r
// and keep the \n
//-----------------------------------------------------------------------------
char * fgetsNoCR(char *buffer, int size, FILE * stream)
{
	char *returnObject = fgets(buffer, size, stream);
	if (returnObject)
	{
		for (int i = 0; i < size; i++)
		{
			if (buffer[i] == '\r')
			{
				buffer[i] = '\n';
				buffer[i + 1] = '\0';
			}
		}
	}

	return returnObject;
}


//-----------------------------------------------------------------------------
// Load a project from a saved project description files. This describes the
// program, the target processor, plus certain configuration settings (cycle
// time, processor clock, etc.). Return TRUE for success, FALSE if anything
// went wrong.
//-----------------------------------------------------------------------------
BOOL LoadProjectFromFile(char *filename)
{
	FreeEntireProgram();
	strcpy(CurrentCompileFile, "");

    FILE *f = fopen(filename, "r");
    if(!f) return FALSE;

	char line[512];
    int crystal, cycle, baud;

    while(fgetsNoCR(line, sizeof(line), f))
    {
        if(strcmp(line, "IO LIST\n")==0)
        {
            if(!LoadIoListFromFile(f))
            {
                fclose(f);
                return FALSE;
            }
        }
        else if(sscanf(line, "CRYSTAL=%d", &crystal))
        {
            Prog.mcuClock = crystal;
        }
        else if(sscanf(line, "CYCLE=%d", &cycle))
        {
            Prog.cycleTime = cycle;
        }
        else if(sscanf(line, "BAUD=%d", &baud))
        {
            Prog.baudRate = baud;
        }
        else if(memcmp(line, "COMPILED=", 9)==0)
        {
            line[strlen(line)-1] = '\0';
            strcpy(CurrentCompileFile, line+9);
        }
        else if(memcmp(line, "MICRO=", 6)==0)
        {
            line[strlen(line)-1] = '\0';
            int i;
            for(i = 0; i < NUM_SUPPORTED_MCUS; i++)
            {
                if(strcmp(SupportedMcus[i].mcuName, line+6)==0)
                {
                    Prog.mcu = &SupportedMcus[i];
                    break;
                }
            }
            if(i == NUM_SUPPORTED_MCUS)
            {
                Error("Microcontroller '%s' not supported.\r\n\r\n"
                    "Defaulting to no selected MCU.", line+6);
            }
        }
        else if(strcmp(line, "PROGRAM\n")==0)
        {
            break;
        }
    }

    if(strcmp(line, "PROGRAM\n") != 0) goto failed;

    int rung;
    for(rung = 0;;)
    {
        if(!fgetsNoCR(line, sizeof(line), f)) break;
        if(strcmp(line, "RUNG\n")!=0) goto failed;

        Prog.rungs[rung] = LoadSeriesFromFile(f);
        if(!Prog.rungs[rung]) goto failed;
        rung++;
    }
    Prog.numRungs = rung;

    fclose(f);
    return TRUE;

	failed:
    fclose(f);
    NewProgram();
    Error("File format error! Check the .ld file and try again");
    return FALSE;
}
