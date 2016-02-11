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
// Routines to maintain the processor I/O list, to build them from file and
// append them on the PlcProgram data structure.
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

using namespace std;

#include <stdlib.h>
#include <string.h>

#include "oplc_compiler.h"

// I/O that we have seen recently, so that we don't forget pin assignments
// when we re-extract the list
#define MAX_IO_SEEN_PREVIOUSLY 512
static struct {
    char    name[MAX_NAME_LEN];
    int     type;
    int     pin;
} IoSeenPreviously[MAX_IO_SEEN_PREVIOUSLY];
static int IoSeenPreviouslyCount;


//-----------------------------------------------------------------------------
// Move an I/O pin into the 'seen previously' list. This means that if the
// user creates input Xasd, assigns it a pin, deletes, and then recreates it,
// then it will come back with the correct pin assigned.
//-----------------------------------------------------------------------------
static void AppendIoSeenPreviously(char *name, int type, int pin)
{
    if(strcmp(name+1, "new")==0) return;

    int i;
    for(i = 0; i < IoSeenPreviouslyCount; i++)
    {
        if(strcmp(name, IoSeenPreviously[i].name)==0 && type == IoSeenPreviously[i].type)
        {
            if(pin != NO_PIN_ASSIGNED)
            {
                IoSeenPreviously[i].pin = pin;
            }
            return;
        }
    }
    if(IoSeenPreviouslyCount >= MAX_IO_SEEN_PREVIOUSLY)
    {
        // maybe improve later; just throw away all our old information, and
        // the user might have to reenter the pin if they delete and recreate
        // things
        IoSeenPreviouslyCount = 0;
    }

    i = IoSeenPreviouslyCount;
    IoSeenPreviously[i].type = type;
    IoSeenPreviously[i].pin = pin;
    strcpy(IoSeenPreviously[i].name, name);
    IoSeenPreviouslyCount++;
}

//-----------------------------------------------------------------------------
// Append an I/O to the I/O list if it is not in there already.
//-----------------------------------------------------------------------------
static void AppendIo(char *name, int type)
{
    int i;
    for(i = 0; i < Prog.io.count; i++)
    {
        if(strcmp(Prog.io.assignment[i].name, name)==0)
        {
            if(type != IO_TYPE_GENERAL && Prog.io.assignment[i].type == IO_TYPE_GENERAL)
            {
                Prog.io.assignment[i].type = type;
            }
            // already in there
            return;
        }
    }

    if(i < MAX_IO)
    {
        Prog.io.assignment[i].type = type;
        Prog.io.assignment[i].pin = NO_PIN_ASSIGNED;
        strcpy(Prog.io.assignment[i].name, name);
        (Prog.io.count)++;
    }
}

//-----------------------------------------------------------------------------
// Walk a subcircuit, calling ourselves recursively and extracting all the
// I/O names out of it.
//-----------------------------------------------------------------------------
static void ExtractNamesFromCircuit(int which, void *any)
{
    ElemLeaf *l = (ElemLeaf *)any;

    switch(which)
    {
        case ELEM_PARALLEL_SUBCKT:
        {
            ElemSubcktParallel *p = (ElemSubcktParallel *)any;
            int i;
            for(i = 0; i < p->count; i++)
            {
                ExtractNamesFromCircuit(p->contents[i].which, p->contents[i].d.any);
            }
            break;
        }
        case ELEM_SERIES_SUBCKT:
        {
            ElemSubcktSeries *s = (ElemSubcktSeries *)any;
            int i;
            for(i = 0; i < s->count; i++)
            {
                ExtractNamesFromCircuit(s->contents[i].which, s->contents[i].d.any);
            }
            break;
        }
        case ELEM_CONTACTS:
            switch(l->d.contacts.name[0])
            {
                case 'R':
                    AppendIo(l->d.contacts.name, IO_TYPE_INTERNAL_RELAY);
                    break;

                case 'Y':
                    AppendIo(l->d.contacts.name, IO_TYPE_DIG_OUTPUT);
                    break;

                case 'X':
                    AppendIo(l->d.contacts.name, IO_TYPE_DIG_INPUT);
                    break;

                default:
                    oops();
                    break;
            }
            break;

        case ELEM_COIL:
            AppendIo(l->d.coil.name, l->d.coil.name[0] == 'R' ? IO_TYPE_INTERNAL_RELAY : IO_TYPE_DIG_OUTPUT);
            break;

        case ELEM_TON:
        case ELEM_TOF:
            AppendIo(l->d.timer.name, which == ELEM_TON ?  IO_TYPE_TON : IO_TYPE_TOF);
            break;

        case ELEM_RTO:
            AppendIo(l->d.timer.name, IO_TYPE_RTO);
            break;

        case ELEM_MOVE:
            AppendIo(l->d.move.dest, IO_TYPE_GENERAL);
            break;

        case ELEM_ADD:
        case ELEM_SUB:
        case ELEM_MUL:
        case ELEM_DIV:
            AppendIo(l->d.math.dest, IO_TYPE_GENERAL);
            break;

        case ELEM_FORMATTED_STRING:
            if(strlen(l->d.fmtdStr.var) > 0)
            {
                AppendIo(l->d.fmtdStr.var, IO_TYPE_UART_TX);
            }
            break;

        case ELEM_UART_SEND:
            AppendIo(l->d.uart.name, IO_TYPE_UART_TX);
            break;

        case ELEM_UART_RECV:
            AppendIo(l->d.uart.name, IO_TYPE_UART_RX);
            break;

        case ELEM_SET_PWM:
            AppendIo(l->d.setPwm.name, IO_TYPE_PWM_OUTPUT);
            break;

        case ELEM_CTU:
        case ELEM_CTD:
        case ELEM_CTC:
            AppendIo(l->d.counter.name, IO_TYPE_COUNTER);
            break;

        case ELEM_READ_ADC:
            AppendIo(l->d.readAdc.name, IO_TYPE_READ_ADC);
            break;

        case ELEM_SHIFT_REGISTER:
        {
            int i;
            for(i = 0; i < l->d.shiftRegister.stages; i++)
            {
                char str[MAX_NAME_LEN+10];
                sprintf(str, "%s%d", l->d.shiftRegister.name, i);
                AppendIo(str, IO_TYPE_GENERAL);
            }
            break;
        }

        case ELEM_LOOK_UP_TABLE:
            AppendIo(l->d.lookUpTable.dest, IO_TYPE_GENERAL);
            break;

        case ELEM_PIECEWISE_LINEAR:
            AppendIo(l->d.piecewiseLinear.dest, IO_TYPE_GENERAL);
            break;

        case ELEM_PLACEHOLDER:
        case ELEM_COMMENT:
        case ELEM_SHORT:
        case ELEM_OPEN:
        case ELEM_MASTER_RELAY:
        case ELEM_ONE_SHOT_RISING:
        case ELEM_ONE_SHOT_FALLING:
        case ELEM_EQU:
        case ELEM_NEQ:
        case ELEM_GRT:
        case ELEM_GEQ:
        case ELEM_LES:
        case ELEM_LEQ:
        case ELEM_RES:
        case ELEM_PERSIST:
            break;

        default:
            oops();
    }
}

//-----------------------------------------------------------------------------
// Compare function to qsort() the I/O list. Group by type, then
// alphabetically within each section.
//-----------------------------------------------------------------------------
static int CompareIo(const void *av, const void *bv)
{
    PlcProgramSingleIo *a = (PlcProgramSingleIo *)av;
    PlcProgramSingleIo *b = (PlcProgramSingleIo *)bv;

    if(a->type != b->type)
    {
        return a->type - b->type;
    }

    if(a->pin == NO_PIN_ASSIGNED && b->pin != NO_PIN_ASSIGNED) return  1;
    if(b->pin == NO_PIN_ASSIGNED && a->pin != NO_PIN_ASSIGNED) return -1;

    return strcmp(a->name, b->name);
}

//-----------------------------------------------------------------------------
// Wipe the I/O list and then re-extract it from the PLC program, taking
// care not to forget the pin assignments. Gets passed the selected item
// as an index into the list; modifies the list, so returns the new selected
// item as an index into the new list.
//-----------------------------------------------------------------------------
int GenerateIoList(int prevSel)
{
    int i, j;

    char selName[MAX_NAME_LEN];
    if(prevSel >= 0)
    {
        strcpy(selName, Prog.io.assignment[prevSel].name);
    }

    if(IoSeenPreviouslyCount > MAX_IO_SEEN_PREVIOUSLY/2)
    {
        // flush it so there's lots of room, and we don't run out and
        // forget important things
        IoSeenPreviouslyCount = 0;
    }

    // remember the pin assignments
    for(i = 0; i < Prog.io.count; i++)
    {
        AppendIoSeenPreviously(Prog.io.assignment[i].name, Prog.io.assignment[i].type, Prog.io.assignment[i].pin);
    }
    // wipe the list
    Prog.io.count = 0;
    // extract the new list so that it must be up to date
    for(i = 0; i < Prog.numRungs; i++)
    {
        ExtractNamesFromCircuit(ELEM_SERIES_SUBCKT, Prog.rungs[i]);
    }

    for(i = 0; i < Prog.io.count; i++)
    {
        if(Prog.io.assignment[i].type == IO_TYPE_DIG_INPUT ||
           Prog.io.assignment[i].type == IO_TYPE_DIG_OUTPUT ||
           Prog.io.assignment[i].type == IO_TYPE_READ_ADC)
        {
            for(j = 0; j < IoSeenPreviouslyCount; j++)
            {
                if(strcmp(Prog.io.assignment[i].name, IoSeenPreviously[j].name)==0)
                {
                    Prog.io.assignment[i].pin = IoSeenPreviously[j].pin;
                    break;
                }
            }
        }
    }

    qsort(Prog.io.assignment, Prog.io.count, sizeof(PlcProgramSingleIo), CompareIo);

    if(prevSel >= 0)
    {
        for(i = 0; i < Prog.io.count; i++)
        {
            if(strcmp(Prog.io.assignment[i].name, selName)==0)
                break;
        }
        if(i < Prog.io.count)
            return i;
    }
    // no previous, or selected was deleted
    return -1;
}


//-----------------------------------------------------------------------------
// Load the I/O list from a file. Since we are just loading pin assignments,
// put it into IoSeenPreviously so that it will get used on the next
// extraction.
//-----------------------------------------------------------------------------
BOOL LoadIoListFromFile(FILE *f)
{
    char line[80];
    char name[MAX_NAME_LEN];
    int pin;
    while(fgetsNoCR(line, sizeof(line), f))
    {
        if(strcmp(line, "END\n")==0)
        {
            return TRUE;
        }
        // Don't internationalize this! It's the file format, not UI.
        if(sscanf(line, "    %s at %d", name, &pin)==2)
        {
            int type;
            switch(name[0])
            {
                case 'X': type = IO_TYPE_DIG_INPUT; break;
                case 'Y': type = IO_TYPE_DIG_OUTPUT; break;
                case 'A': type = IO_TYPE_READ_ADC; break;
                default: oops();
            }
            AppendIoSeenPreviously(name, type, pin);
        }
    }
    return FALSE;
}
