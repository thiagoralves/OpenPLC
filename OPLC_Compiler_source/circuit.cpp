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
// Routines for analysing the circuit and creating the specific structure
// for it: add a particular element at a particular point, etc.
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

using namespace std;

#include <stddef.h>
#include <stdlib.h>

#include "oplc_compiler.h"

//-----------------------------------------------------------------------------
// Convenience routines for allocating frequently-used data structures.
//-----------------------------------------------------------------------------
ElemLeaf *AllocLeaf(void)
{
    return (ElemLeaf *)CheckMalloc(sizeof(ElemLeaf));
}
ElemSubcktSeries *AllocSubcktSeries(void)
{
    return (ElemSubcktSeries *)CheckMalloc(sizeof(ElemSubcktSeries));
}
ElemSubcktParallel *AllocSubcktParallel(void)
{
    return (ElemSubcktParallel *)CheckMalloc(sizeof(ElemSubcktParallel));
}

//-----------------------------------------------------------------------------
// Allocate a new `empty' rung, with only a single relay coil at the end. All
// the UI code assumes that rungs always have a coil in them, so it would
// add a lot of nasty special cases to create rungs totally empty.
//-----------------------------------------------------------------------------
static ElemSubcktSeries *AllocEmptyRung(void)
{
    ElemSubcktSeries *s = AllocSubcktSeries();
    s->count = 1;
    s->contents[0].which = ELEM_PLACEHOLDER;
    ElemLeaf *l = AllocLeaf();
    s->contents[0].d.leaf = l;

    return s;
}

//-----------------------------------------------------------------------------
// Start a new project. Give them one rung, with a coil (that they can
// never delete) and nothing else.
//-----------------------------------------------------------------------------
void NewProgram(void)
{
    FreeEntireProgram();

    Prog.numRungs = 1;
    Prog.rungs[0] = AllocEmptyRung();
}

//-----------------------------------------------------------------------------
// Free a circuit and all of its subcircuits. Calls self recursively to do
// so.
//-----------------------------------------------------------------------------
void FreeCircuit(int which, void *any)
{
    ok();
    switch(which)
    {
        case ELEM_SERIES_SUBCKT:
        {
            ElemSubcktSeries *s = (ElemSubcktSeries *)any;
            int i;
            for(i = 0; i < s->count; i++)
            {
                FreeCircuit(s->contents[i].which, s->contents[i].d.any);
            }
            CheckFree(s);
            break;
        }

        case ELEM_PARALLEL_SUBCKT:
        {
            ElemSubcktParallel *p = (ElemSubcktParallel *)any;
            int i;
            for(i = 0; i < p->count; i++)
            {
                FreeCircuit(p->contents[i].which, p->contents[i].d.any);
            }
            CheckFree(p);
            break;
        }

        CASE_LEAF
            ForgetFromGrid(any);
            CheckFree(any);
            break;

        default:
            oops();
            break;
    }
    ok();
}

//-----------------------------------------------------------------------------
// Free the entire program.
//-----------------------------------------------------------------------------
void FreeEntireProgram(void)
{
    ForgetEverything();

    int i;
    for(i = 0; i < Prog.numRungs; i++)
    {
        FreeCircuit(ELEM_SERIES_SUBCKT, Prog.rungs[i]);
    }
    Prog.numRungs = 0;
    Prog.cycleTime = 10000;
    Prog.mcuClock = 4000000;
    Prog.baudRate = 2400;
    Prog.io.count = 0;
    Prog.mcu = NULL;
}

//-----------------------------------------------------------------------------
// Returns TRUE if the subcircuit contains any of the given instruction
// types (ELEM_....), else FALSE.
//-----------------------------------------------------------------------------
static BOOL ContainsWhich(int which, void *any, int seek1, int seek2, int seek3)
{
    switch(which)
    {
        case ELEM_PARALLEL_SUBCKT:
        {
            ElemSubcktParallel *p = (ElemSubcktParallel *)any;
            int i;
            for(i = 0; i < p->count; i++)
            {
                if(ContainsWhich(p->contents[i].which, p->contents[i].d.any, seek1, seek2, seek3))
                {
                    return TRUE;
                }
            }
            break;
        }
        case ELEM_SERIES_SUBCKT:
        {
            ElemSubcktSeries *s = (ElemSubcktSeries *)any;
            int i;
            for(i = 0; i < s->count; i++)
            {
                if(ContainsWhich(s->contents[i].which, s->contents[i].d.any, seek1, seek2, seek3))
                {
                    return TRUE;
                }
            }
            break;
        }
        default:
            if(which == seek1 || which == seek2 || which == seek3)
            {
                return TRUE;
            }
            break;
    }
    return FALSE;
}

//-----------------------------------------------------------------------------
// Are either of the UART functions (send or recv) used? Need to know this
// to know whether we must receive their pins.
//-----------------------------------------------------------------------------
BOOL UartFunctionUsed(void)
{
    int i;
    for(i = 0; i < Prog.numRungs; i++)
    {
        if(ContainsWhich(ELEM_SERIES_SUBCKT, Prog.rungs[i], ELEM_UART_RECV, ELEM_UART_SEND, ELEM_FORMATTED_STRING))
        {
            return TRUE;
        }
    }
    return FALSE;
}

//-----------------------------------------------------------------------------
// Is the PWM function used? Need to know this to know whether we must reserve
// the pin.
//-----------------------------------------------------------------------------
BOOL PwmFunctionUsed(void)
{
    int i;
    for(i = 0; i < Prog.numRungs; i++)
    {
        if(ContainsWhich(ELEM_SERIES_SUBCKT, Prog.rungs[i], ELEM_SET_PWM, -1, -1))
        {
            return TRUE;
        }
    }
    return FALSE;
}
