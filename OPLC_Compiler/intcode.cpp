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
// Generate intermediate code for the ladder logic. Basically generate code
// for a 'virtual machine' with operations chosen to be easy to compile to
// ANSI C or any other platform.
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "oplc_compiler.h"
#include "intcode.h"

IntOp IntCode[MAX_INT_OPS];
int IntCodeLen;

static DWORD GenSymCountParThis;
static DWORD GenSymCountParOut;
static DWORD GenSymCountOneShot;
static DWORD GenSymCountFormattedString;

static WORD EepromAddrFree;

//-----------------------------------------------------------------------------
// Generate a unique symbol (unique with each call) having the given prefix
// guaranteed not to conflict with any user symbols.
//-----------------------------------------------------------------------------
static void GenSymParThis(char *dest)
{
    sprintf(dest, "$parThis_%04x", GenSymCountParThis);
    GenSymCountParThis++;
}
static void GenSymParOut(char *dest)
{
    sprintf(dest, "$parOut_%04x", GenSymCountParOut);
    GenSymCountParOut++;
}
static void GenSymOneShot(char *dest)
{
    sprintf(dest, "$oneShot_%04x", GenSymCountOneShot);
    GenSymCountOneShot++;
}
static void GenSymFormattedString(char *dest)
{
    sprintf(dest, "$formattedString_%04x", GenSymCountFormattedString);
    GenSymCountFormattedString++;
}

//-----------------------------------------------------------------------------
// Compile an instruction to the program.
//-----------------------------------------------------------------------------
static void Op(int op, char *name1, char *name2, char *name3, SWORD lit)
{
    IntCode[IntCodeLen].op = op;
    if(name1) strcpy(IntCode[IntCodeLen].name1, name1);
    if(name2) strcpy(IntCode[IntCodeLen].name2, name2);
    if(name3) strcpy(IntCode[IntCodeLen].name3, name3);
    IntCode[IntCodeLen].literal = lit;
    IntCodeLen++;
}
static void Op(int op, char *name1, char *name2, SWORD lit)
{
    Op(op, name1, name2, NULL, lit);
}
static void Op(int op, char *name1, SWORD lit)
{
    Op(op, name1, NULL, NULL, lit);
}
static void Op(int op, char *name1, char *name2)
{
    Op(op, name1, name2, NULL, 0);
}
static void Op(int op, char *name1)
{
    Op(op, name1, NULL, NULL, 0);
}
static void Op(int op)
{
    Op(op, NULL, NULL, NULL, 0);
}

//-----------------------------------------------------------------------------
// printf-like comment function
//-----------------------------------------------------------------------------
void Comment(char *str, ...)
{
    va_list f;
    char buf[MAX_NAME_LEN];
    va_start(f, str);
    vsprintf(buf, str, f);
    Op(INT_COMMENT, buf);
}

//-----------------------------------------------------------------------------
// Compile the instruction that the simulator uses to keep track of which
// nodes are energized (so that it can display which branches of the circuit
// are energized onscreen). The MCU code generators ignore this, of course.
//-----------------------------------------------------------------------------
static void SimState(BOOL *b, char *name)
{
    IntCode[IntCodeLen].op = INT_SIMULATE_NODE_STATE;
    strcpy(IntCode[IntCodeLen].name1, name);
    IntCode[IntCodeLen].poweredAfter = b;
    IntCodeLen++;
}

//-----------------------------------------------------------------------------
// Is an expression that could be either a variable name or a number a number?
//-----------------------------------------------------------------------------
static BOOL IsNumber(char *str)
{
    if(*str == '-' || isdigit(*str))
    {
        return TRUE;
    }
    else if(*str == '\'')
    {
        // special case--literal single character
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//-----------------------------------------------------------------------------
// Report an error if a constant doesn't fit in 16 bits.
//-----------------------------------------------------------------------------
void CheckConstantInRange(int v)
{
    if(v < -32768 || v > 32767)
    {
        Error("Constant %d out of range: -32768 to 32767 inclusive.", v);
        CompileError();
    }
}

//-----------------------------------------------------------------------------
// Try to turn a string into a 16-bit constant, and raise an error if
// something bad happens when we do so (e.g. out of range).
//-----------------------------------------------------------------------------
SWORD CheckMakeNumber(char *str)
{
    int val;

    if(*str == '\'')
    {
        val = str[1];
    }
    else
    {
        val = atoi(str);
    }

    CheckConstantInRange(val);

    return (SWORD)val;
}

//-----------------------------------------------------------------------------
// Calculate the period in scan units from the period in microseconds, and
// raise an error if the given period is unachievable.
//-----------------------------------------------------------------------------
static int TimerPeriod(ElemLeaf *l)
{
    int period = (l->d.timer.delay / Prog.cycleTime) - 1;

    if(period < 1)
    {
        Error("Timer period too short (needs faster cycle time).");
        CompileError();
    }
    if(period >= (1 << 15))
    {
        Error("Timer period too long (max 32767 times cycle time); use a slower cycle time.");
        CompileError();
    }

    return period;
}

//-----------------------------------------------------------------------------
// Convert a hex digit (0-9a-fA-F) to its hex value, or return -1 if the
// character is not a hex digit.
//-----------------------------------------------------------------------------
int HexDigit(int c)
{
    if((c >= '0') && (c <= '9'))
    {
        return c - '0';
    }
    else if((c >= 'a') && (c <= 'f'))
    {
        return 10 + (c - 'a');
    }
    else if((c >= 'A') && (c <= 'F'))
    {
        return 10 + (c - 'A');
    }
    return -1;
}

//-----------------------------------------------------------------------------
// Return an integer power of ten.
//-----------------------------------------------------------------------------
static int TenToThe(int x)
{
    int i;
    int r = 1;
    for(i = 0; i < x; i++)
    {
        r *= 10;
    }
    return r;
}

//-----------------------------------------------------------------------------
// Compile code to evaluate the given bit of ladder logic. The rung input
// state is in stateInOut before calling and will be in stateInOut after
// calling.
//-----------------------------------------------------------------------------
static char *VarFromExpr(char *expr, char *tempName)
{
    if(IsNumber(expr))
    {
        Op(INT_SET_VARIABLE_TO_LITERAL, tempName, CheckMakeNumber(expr));
        return tempName;
    }
    else
    {
        return expr;
    }
}
static void IntCodeFromCircuit(int which, void *any, char *stateInOut)
{
    ElemLeaf *l = (ElemLeaf *)any;

    switch(which)
    {
        case ELEM_SERIES_SUBCKT:
        {
            int i;
            ElemSubcktSeries *s = (ElemSubcktSeries *)any;

            Comment("start series [");
            for(i = 0; i < s->count; i++)
            {
                IntCodeFromCircuit(s->contents[i].which, s->contents[i].d.any, stateInOut);
            }
            Comment("] finish series");
            break;
        }

        case ELEM_PARALLEL_SUBCKT:
        {
            char parThis[MAX_NAME_LEN];
            GenSymParThis(parThis);

            char parOut[MAX_NAME_LEN];
            GenSymParOut(parOut);

            Comment("start parallel [");

            Op(INT_CLEAR_BIT, parOut);

            ElemSubcktParallel *p = (ElemSubcktParallel *)any;
            int i;
            for(i = 0; i < p->count; i++)
            {
                Op(INT_COPY_BIT_TO_BIT, parThis, stateInOut);

                IntCodeFromCircuit(p->contents[i].which, p->contents[i].d.any, parThis);

                Op(INT_IF_BIT_SET, parThis);
                Op(INT_SET_BIT, parOut);
                Op(INT_END_IF);
            }
            Op(INT_COPY_BIT_TO_BIT, stateInOut, parOut);
            Comment("] finish parallel");

            break;
        }
        case ELEM_CONTACTS:
        {
            if(l->d.contacts.negated)
            {
                Op(INT_IF_BIT_SET, l->d.contacts.name);
            }
            else
            {
                Op(INT_IF_BIT_CLEAR, l->d.contacts.name);
            }
            Op(INT_CLEAR_BIT, stateInOut);
            Op(INT_END_IF);
            break;
        }
        case ELEM_COIL:
        {
            if(l->d.coil.negated)
            {
                Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_CLEAR_BIT, l->d.contacts.name);
                Op(INT_ELSE);
                Op(INT_SET_BIT, l->d.contacts.name);
                Op(INT_END_IF);
            }
            else if(l->d.coil.setOnly)
            {
                Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_SET_BIT, l->d.contacts.name);
                Op(INT_END_IF);
            }
            else if(l->d.coil.resetOnly)
            {
                Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_CLEAR_BIT, l->d.contacts.name);
                Op(INT_END_IF);
            }
            else
            {
                Op(INT_COPY_BIT_TO_BIT, l->d.contacts.name, stateInOut);
            }
            break;
        }
        case ELEM_RTO:
        {
            int period = TimerPeriod(l);

            Op(INT_IF_VARIABLE_LES_LITERAL, l->d.timer.name, period);

            Op(INT_IF_BIT_SET, stateInOut);
            Op(INT_INCREMENT_VARIABLE, l->d.timer.name);
            Op(INT_END_IF);
            Op(INT_CLEAR_BIT, stateInOut);

            Op(INT_ELSE);

            Op(INT_SET_BIT, stateInOut);

            Op(INT_END_IF);

            break;
        }
        case ELEM_RES:
            Op(INT_IF_BIT_SET, stateInOut);
            Op(INT_SET_VARIABLE_TO_LITERAL, l->d.reset.name);
            Op(INT_END_IF);
            break;

        case ELEM_TON:
        {
            int period = TimerPeriod(l);

            Op(INT_IF_BIT_SET, stateInOut);

            Op(INT_IF_VARIABLE_LES_LITERAL, l->d.timer.name, period);

            Op(INT_INCREMENT_VARIABLE, l->d.timer.name);
            Op(INT_CLEAR_BIT, stateInOut);
            Op(INT_END_IF);

            Op(INT_ELSE);

            Op(INT_SET_VARIABLE_TO_LITERAL, l->d.timer.name);

            Op(INT_END_IF);

            break;
        }
        case ELEM_TOF:
        {
            int period = TimerPeriod(l);

            // All variables start at zero by default, so by default the
            // TOF timer would start out with its output forced HIGH, until
            // it finishes counting up. This does not seem to be what
            // people expect, so add a special case to fix that up.
            char antiGlitchName[MAX_NAME_LEN];
            sprintf(antiGlitchName, "$%s_antiglitch", l->d.timer.name);
            Op(INT_IF_BIT_CLEAR, antiGlitchName);
                Op(INT_SET_VARIABLE_TO_LITERAL, l->d.timer.name, period);
            Op(INT_END_IF);
            Op(INT_SET_BIT, antiGlitchName);

            Op(INT_IF_BIT_CLEAR, stateInOut);

            Op(INT_IF_VARIABLE_LES_LITERAL, l->d.timer.name, period);

            Op(INT_INCREMENT_VARIABLE, l->d.timer.name);
            Op(INT_SET_BIT, stateInOut);
            Op(INT_END_IF);

            Op(INT_ELSE);

            Op(INT_SET_VARIABLE_TO_LITERAL, l->d.timer.name);

            Op(INT_END_IF);
            break;
        }
        case ELEM_CTU:
        {
            CheckConstantInRange(l->d.counter.max);
            char storeName[MAX_NAME_LEN];
            GenSymOneShot(storeName);

            Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_IF_BIT_CLEAR, storeName);
                    Op(INT_INCREMENT_VARIABLE, l->d.counter.name);
                Op(INT_END_IF);
            Op(INT_END_IF);
            Op(INT_COPY_BIT_TO_BIT, storeName, stateInOut);

            Op(INT_IF_VARIABLE_LES_LITERAL, l->d.counter.name,
                l->d.counter.max);
                Op(INT_CLEAR_BIT, stateInOut);
            Op(INT_ELSE);
                Op(INT_SET_BIT, stateInOut);
            Op(INT_END_IF);
            break;
        }
        case ELEM_CTD:
        {
            CheckConstantInRange(l->d.counter.max);
            char storeName[MAX_NAME_LEN];
            GenSymOneShot(storeName);

            Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_IF_BIT_CLEAR, storeName);
                    Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch", 1);
                    Op(INT_SET_VARIABLE_SUBTRACT, l->d.counter.name,
                        l->d.counter.name, "$scratch", 0);
                Op(INT_END_IF);
            Op(INT_END_IF);
            Op(INT_COPY_BIT_TO_BIT, storeName, stateInOut);

            Op(INT_IF_VARIABLE_LES_LITERAL, l->d.counter.name,
                l->d.counter.max);
                Op(INT_CLEAR_BIT, stateInOut);
            Op(INT_ELSE);
                Op(INT_SET_BIT, stateInOut);
            Op(INT_END_IF);
            break;
        }
        case ELEM_CTC:
        {
            char storeName[MAX_NAME_LEN];
            GenSymOneShot(storeName);

            Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_IF_BIT_CLEAR, storeName);
                    Op(INT_INCREMENT_VARIABLE, l->d.counter.name);
                    Op(INT_IF_VARIABLE_LES_LITERAL, l->d.counter.name,
                        l->d.counter.max+1);
                    Op(INT_ELSE);
                        Op(INT_SET_VARIABLE_TO_LITERAL, l->d.counter.name,
                            (SWORD)0);
                    Op(INT_END_IF);
                Op(INT_END_IF);
            Op(INT_END_IF);
            Op(INT_COPY_BIT_TO_BIT, storeName, stateInOut);
            break;
        }
        case ELEM_GRT:
        case ELEM_GEQ:
        case ELEM_LES:
        case ELEM_LEQ:
        case ELEM_NEQ:
        case ELEM_EQU:
        {
            char *op1 = VarFromExpr(l->d.cmp.op1, "$scratch");
            char *op2 = VarFromExpr(l->d.cmp.op2, "$scratch2");

            if(which == ELEM_GRT)
            {
                Op(INT_IF_VARIABLE_GRT_VARIABLE, op1, op2);
                Op(INT_ELSE);
            }
            else if(which == ELEM_GEQ)
            {
                Op(INT_IF_VARIABLE_GRT_VARIABLE, op2, op1);
            }
            else if(which == ELEM_LES)
            {
                Op(INT_IF_VARIABLE_GRT_VARIABLE, op2, op1);
                Op(INT_ELSE);
            }
            else if(which == ELEM_LEQ)
            {
                Op(INT_IF_VARIABLE_GRT_VARIABLE, op1, op2);
            }
            else if(which == ELEM_EQU)
            {
                Op(INT_IF_VARIABLE_EQUALS_VARIABLE, op1, op2);
                Op(INT_ELSE);
            }
            else if(which == ELEM_NEQ)
            {
                Op(INT_IF_VARIABLE_EQUALS_VARIABLE, op1, op2);
            }
            else oops();

            Op(INT_CLEAR_BIT, stateInOut);
            Op(INT_END_IF);
            break;
        }
        case ELEM_ONE_SHOT_RISING:
        {
            char storeName[MAX_NAME_LEN];
            GenSymOneShot(storeName);

            Op(INT_COPY_BIT_TO_BIT, "$scratch", stateInOut);
            Op(INT_IF_BIT_SET, storeName);
            Op(INT_CLEAR_BIT, stateInOut);
            Op(INT_END_IF);
            Op(INT_COPY_BIT_TO_BIT, storeName, "$scratch");
            break;
        }
        case ELEM_ONE_SHOT_FALLING:
        {
            char storeName[MAX_NAME_LEN];
            GenSymOneShot(storeName);

            Op(INT_COPY_BIT_TO_BIT, "$scratch", stateInOut);

            Op(INT_IF_BIT_CLEAR, stateInOut);
            Op(INT_IF_BIT_SET, storeName);
            Op(INT_SET_BIT, stateInOut);
            Op(INT_END_IF);
            Op(INT_ELSE);
            Op(INT_CLEAR_BIT, stateInOut);
            Op(INT_END_IF);

            Op(INT_COPY_BIT_TO_BIT, storeName, "$scratch");
            break;
        }
        case ELEM_MOVE:
        {
            if(IsNumber(l->d.move.dest))
            {
                Error("Move instruction: '%s' not a valid destination.", l->d.move.dest);
                CompileError();
            }
            Op(INT_IF_BIT_SET, stateInOut);
            if(IsNumber(l->d.move.src))
            {
                Op(INT_SET_VARIABLE_TO_LITERAL, l->d.move.dest, CheckMakeNumber(l->d.move.src));
            }
            else
            {
                Op(INT_SET_VARIABLE_TO_VARIABLE, l->d.move.dest, l->d.move.src, 0);
            }
            Op(INT_END_IF);
            break;
        }

        // These four are highly processor-dependent; the int code op does
        // most of the highly specific work
        {
            case ELEM_READ_ADC:
                Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_READ_ADC, l->d.readAdc.name);
                Op(INT_END_IF);
                break;

            case ELEM_SET_PWM:
            {
                Op(INT_IF_BIT_SET, stateInOut);
                char line[80];
                // ugh; need a >16 bit literal though, could be >64 kHz
                sprintf(line, "%d", l->d.setPwm.targetFreq);
                Op(INT_SET_PWM, l->d.readAdc.name, line);
                Op(INT_END_IF);
                break;
            }
            case ELEM_PERSIST:
            {
                Op(INT_IF_BIT_SET, stateInOut);

                // At startup, get the persistent variable from flash.
                char isInit[MAX_NAME_LEN];
                GenSymOneShot(isInit);
                Op(INT_IF_BIT_CLEAR, isInit);
                    Op(INT_CLEAR_BIT, "$scratch");
                    Op(INT_EEPROM_BUSY_CHECK, "$scratch");
                    Op(INT_IF_BIT_CLEAR, "$scratch");
                        Op(INT_SET_BIT, isInit);
                        Op(INT_EEPROM_READ, l->d.persist.var, EepromAddrFree);
                    Op(INT_END_IF);
                Op(INT_END_IF);

                // While running, continuously compare the EEPROM copy of
                // the variable against the RAM one; if they are different,
                // write the RAM one to EEPROM.
                Op(INT_CLEAR_BIT, "$scratch");
                Op(INT_EEPROM_BUSY_CHECK, "$scratch");
                Op(INT_IF_BIT_CLEAR, "$scratch");
                    Op(INT_EEPROM_READ, "$scratch", EepromAddrFree);
                    Op(INT_IF_VARIABLE_EQUALS_VARIABLE, "$scratch",
                        l->d.persist.var);
                    Op(INT_ELSE);
                        Op(INT_EEPROM_WRITE, l->d.persist.var, EepromAddrFree);
                    Op(INT_END_IF);
                Op(INT_END_IF);

                Op(INT_END_IF);

                EepromAddrFree += 2;
                break;
            }
            case ELEM_UART_SEND:
                Op(INT_UART_SEND, l->d.uart.name, stateInOut);
                break;

            case ELEM_UART_RECV:
                Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_UART_RECV, l->d.uart.name, stateInOut);
                Op(INT_END_IF);
                break;
        }

        case ELEM_ADD:
        case ELEM_SUB:
        case ELEM_MUL:
        case ELEM_DIV:
        {
            if(IsNumber(l->d.math.dest))
            {
                Error("Math instruction: '%s' not a valid destination.", l->d.math.dest);
                CompileError();
            }
            Op(INT_IF_BIT_SET, stateInOut);

            char *op1 = VarFromExpr(l->d.math.op1, "$scratch");
            char *op2 = VarFromExpr(l->d.math.op2, "$scratch2");

            int intOp;
            if(which == ELEM_ADD)
            {
                intOp = INT_SET_VARIABLE_ADD;
            }
            else if(which == ELEM_SUB)
            {
                intOp = INT_SET_VARIABLE_SUBTRACT;
            }
            else if(which == ELEM_MUL)
            {
                intOp = INT_SET_VARIABLE_MULTIPLY;
            }
            else if(which == ELEM_DIV)
            {
                intOp = INT_SET_VARIABLE_DIVIDE;
            }
            else oops();

            Op(intOp, l->d.math.dest, op1, op2, 0);

            Op(INT_END_IF);
            break;
        }
        case ELEM_MASTER_RELAY:
            // Tricky: must set the master control relay if we reach this
            // instruction while the master control relay is cleared, because
            // otherwise there is no good way for it to ever become set
            // again.
            Op(INT_IF_BIT_CLEAR, "$mcr");
            Op(INT_SET_BIT, "$mcr");
            Op(INT_ELSE);
            Op(INT_COPY_BIT_TO_BIT, "$mcr", stateInOut);
            Op(INT_END_IF);
            break;

        case ELEM_SHIFT_REGISTER:
        {
            char storeName[MAX_NAME_LEN];
            GenSymOneShot(storeName);
            Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_IF_BIT_CLEAR, storeName);
                    int i;
                    for(i = (l->d.shiftRegister.stages-2); i >= 0; i--)
                    {
                        char dest[MAX_NAME_LEN+10], src[MAX_NAME_LEN+10];
                        sprintf(src, "%s%d", l->d.shiftRegister.name, i);
                        sprintf(dest, "%s%d", l->d.shiftRegister.name, i+1);
                        Op(INT_SET_VARIABLE_TO_VARIABLE, dest, src);
                    }
                Op(INT_END_IF);
            Op(INT_END_IF);
            Op(INT_COPY_BIT_TO_BIT, storeName, stateInOut);
            break;
        }
        case ELEM_LOOK_UP_TABLE:
        {
            // God this is stupid; but it will have to do, at least until I
            // add new int code instructions for this.
            int i;
            Op(INT_IF_BIT_SET, stateInOut);
            ElemLookUpTable *t = &(l->d.lookUpTable);
            for(i = 0; i < t->count; i++)
            {
                Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch", i);
                Op(INT_IF_VARIABLE_EQUALS_VARIABLE, t->index, "$scratch");
                    Op(INT_SET_VARIABLE_TO_LITERAL, t->dest, t->vals[i]);
                Op(INT_END_IF);
            }
            Op(INT_END_IF);
            break;
        }
        case ELEM_PIECEWISE_LINEAR:
        {
            // This one is not so obvious; we have to decide how best to
            // perform the linear interpolation, using our 16-bit fixed
            // point math.
            ElemPiecewiseLinear *t = &(l->d.piecewiseLinear);
            if(t->count == 0)
            {
                Error("Piecewise linear lookup table with zero elements!");
                CompileError();
            }
            int i;
            int xThis = t->vals[0];
            for(i = 1; i < t->count; i++)
            {
                if(t->vals[i*2] <= xThis)
                {
                    Error("x values in piecewise linear table must be strictly increasing.");
                    CompileError();
                }
                xThis = t->vals[i*2];
            }
            Op(INT_IF_BIT_SET, stateInOut);
            for(i = t->count - 1; i >= 1; i--)
            {
                int thisDx = t->vals[i*2] - t->vals[(i-1)*2];
                int thisDy = t->vals[i*2 + 1] - t->vals[(i-1)*2 + 1];
                // The output point is given by
                //    yout = y[i-1] + (xin - x[i-1])*dy/dx
                // and this is the best form in which to keep it, numerically
                // speaking, because you can always fix numerical problems
                // by moving the PWL points closer together.

                // Check for numerical problems, and fail if we have them.
                if((thisDx*thisDy) >= 32767 || (thisDx*thisDy) <= -32768)
                {
                    Error("Numerical problem with piecewise linear lookup "
                        "table. Either make the table entries smaller, "
                        "or space the points together more closely.\r\n\r\n"
                        "See the help file for details.");
                    CompileError();
                }

                // Hack to avoid AVR brge issue again, since long jumps break
                Op(INT_CLEAR_BIT, "$scratch");
                Op(INT_IF_VARIABLE_LES_LITERAL, t->index, t->vals[i*2]+1);
                    Op(INT_SET_BIT, "$scratch");
                Op(INT_END_IF);

                Op(INT_IF_BIT_SET, "$scratch");
                Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch", t->vals[(i-1)*2]);
                Op(INT_SET_VARIABLE_SUBTRACT, "$scratch", t->index,
                    "$scratch", 0);
                Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch2", thisDx);
                Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch3", thisDy);
                Op(INT_SET_VARIABLE_MULTIPLY, t->dest, "$scratch", "$scratch3",
                    0);
                Op(INT_SET_VARIABLE_DIVIDE, t->dest, t->dest, "$scratch2", 0);

                Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch",
                    t->vals[(i-1)*2 + 1]);
                Op(INT_SET_VARIABLE_ADD, t->dest, t->dest, "$scratch", 0);
                Op(INT_END_IF);
            }
            Op(INT_END_IF);
            break;
        }
        case ELEM_FORMATTED_STRING:
        {
            // Okay, this one is terrible and ineffcient, and it's a huge pain
            // to implement, but people want it a lot. The hard part is that
            // we have to let the PLC keep cycling, of course, and also that
            // we must do the integer to ASCII conversion sensisbly, with
            // only one divide per PLC cycle.

            // This variable is basically our sequencer: it is a counter that
            // increments every time we send a character.
            char seq[MAX_NAME_LEN];
            GenSymFormattedString(seq);

            // The variable whose value we might interpolate.
            char *var = l->d.fmtdStr.var;

            // This is the state variable for our integer-to-string conversion.
            // It contains the absolute value of var, possibly with some
            // of the higher powers of ten missing.
            char convertState[MAX_NAME_LEN];
            GenSymFormattedString(convertState);

            // We might need to suppress some leading zeros.
            char isLeadingZero[MAX_NAME_LEN];
            GenSymFormattedString(isLeadingZero);

            // This is a table of characters to transmit, as a function of the
            // sequencer position (though we might have a hole in the middle
            // for the variable output)
            char outputChars[MAX_LOOK_UP_TABLE_LEN];

            BOOL mustDoMinus = FALSE;

            // The total number of characters that we transmit, including
            // those from the interpolated variable.
            int steps;

            // The total number of digits to convert.
            int digits = -1;

            // So count that now, and build up our table of fixed things to
            // send.
            steps = 0;
            char *p = l->d.fmtdStr.string;
            while(*p)
            {
                if(*p == '\\' && (isdigit(p[1]) || p[1] == '-'))
                {
                    if(digits >= 0)
                    {
                        Error("Multiple escapes (\\0-9) present in format string, not allowed.");
                        CompileError();
                    }
                    p++;
                    if(*p == '-')
                    {
                        mustDoMinus = TRUE;
                        outputChars[steps++] = 1;
                        p++;
                    }
                    if(!isdigit(*p) || (*p - '0') > 5 || *p == '0')
                    {
                        Error("Bad escape sequence following \\; for a literal backslash, use \\\\");
                        CompileError();
                    }
                    digits = (*p - '0');
                    int i;
                    for(i = 0; i < digits; i++)
                    {
                        outputChars[steps++] = 0;
                    }
                }
                else if(*p == '\\')
                {
                    p++;
                    switch(*p)
                    {
                        case 'r': outputChars[steps++] = '\r'; break;
                        case 'n': outputChars[steps++] = '\n'; break;
                        case 'b': outputChars[steps++] = '\b'; break;
                        case 'f': outputChars[steps++] = '\f'; break;
                        case '\\': outputChars[steps++] = '\\'; break;
                        case 'x':
                        {
                            int h, l;
                            p++;
                            h = HexDigit(*p);
                            if(h >= 0)
                            {
                                p++;
                                l = HexDigit(*p);
                                if(l >= 0)
                                {
                                    outputChars[steps++] = (h << 4) | l;
                                    break;
                                }
                            }
                            Error("Bad escape: correct form is \\xAB.");
                            CompileError();
                            break;
                        }
                        default:
                            Error("Bad escape '\\%c'", *p);
                            CompileError();
                            break;
                    }
                }
                else
                {
                    outputChars[steps++] = *p;
                }
                if(*p) p++;
            }

            if(digits >= 0 && (strlen(var) == 0))
            {
                Error("Variable is interpolated into formatted string, but none is specified.");
                CompileError();
            }
            else if(digits < 0 && (strlen(var) > 0))
            {
                Error("No variable is interpolated into formatted string, but a variable name is specified. Include a string like '\\-3', or leave variable name blank.");
                CompileError();
            }

            // We want to respond to rising edges, so yes we need a one shot.
            char oneShot[MAX_NAME_LEN];
            GenSymOneShot(oneShot);

            Op(INT_IF_BIT_SET, stateInOut);
                Op(INT_IF_BIT_CLEAR, oneShot);
                    Op(INT_SET_VARIABLE_TO_LITERAL, seq, (SWORD)0);
                Op(INT_END_IF);
            Op(INT_END_IF);
            Op(INT_COPY_BIT_TO_BIT, oneShot, stateInOut);

            // Everything that involves seqScratch is a terrible hack to
            // avoid an if statement with a big body, which is the risk
            // factor for blowing up on PIC16 page boundaries.

            char *seqScratch = "$scratch3";

            Op(INT_SET_VARIABLE_TO_VARIABLE, seqScratch, seq);

            // No point doing any math unless we'll get to transmit this
            // cycle, so check that first.

            Op(INT_IF_VARIABLE_LES_LITERAL, seq, steps);
            Op(INT_ELSE);
                Op(INT_SET_VARIABLE_TO_LITERAL, seqScratch, -1);
            Op(INT_END_IF);

            Op(INT_CLEAR_BIT, "$scratch");
            Op(INT_UART_SEND, "$scratch", "$scratch");
            Op(INT_IF_BIT_SET, "$scratch");
                Op(INT_SET_VARIABLE_TO_LITERAL, seqScratch, -1);
            Op(INT_END_IF);

            // So we transmit this cycle, so check out which character.
            int i;
            int digit = 0;
            for(i = 0; i < steps; i++)
            {
                if(outputChars[i] == 0)
                {
                    // Note gross hack to work around limit of range for
                    // AVR brne op, which is +/- 64 instructions.
                    Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch", i);
                    Op(INT_CLEAR_BIT, "$scratch");
                    Op(INT_IF_VARIABLE_EQUALS_VARIABLE, "$scratch", seqScratch);
                        Op(INT_SET_BIT, "$scratch");
                    Op(INT_END_IF);

                    Op(INT_IF_BIT_SET, "$scratch");

                    // Start the integer-to-string

                    // If there's no minus, then we have to load up
                    // convertState ourselves the first time.
                    if(digit == 0 && !mustDoMinus)
                    {
                        Op(INT_SET_VARIABLE_TO_VARIABLE, convertState, var);
                    }
                    if(digit == 0)
                    {
                        Op(INT_SET_BIT, isLeadingZero);
                    }

                    Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch",
                        TenToThe((digits-digit)-1));
                    Op(INT_SET_VARIABLE_DIVIDE, "$scratch2", convertState,
                        "$scratch", 0);
                    Op(INT_SET_VARIABLE_MULTIPLY, "$scratch", "$scratch",
                        "$scratch2", 0);
                    Op(INT_SET_VARIABLE_SUBTRACT, convertState,
                        convertState, "$scratch", 0);
                    Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch", '0');
                    Op(INT_SET_VARIABLE_ADD, "$scratch2", "$scratch2",
                        "$scratch", 0);

                    // Suppress all but the last leading zero.
                    if(digit != (digits - 1))
                    {
                        Op(INT_IF_VARIABLE_EQUALS_VARIABLE, "$scratch",
                            "$scratch2");
                            Op(INT_IF_BIT_SET, isLeadingZero);
                                Op(INT_SET_VARIABLE_TO_LITERAL,
                                    "$scratch2", ' ');
                            Op(INT_END_IF);
                        Op(INT_ELSE);
                            Op(INT_CLEAR_BIT, isLeadingZero);
                        Op(INT_END_IF);
                    }

                    Op(INT_END_IF);

                    digit++;
                }
                else if(outputChars[i] == 1)
                {
                    // do the minus; ugliness to get around the BRNE jump
                    // size limit, though
                    Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch", i);
                    Op(INT_CLEAR_BIT, "$scratch");
                    Op(INT_IF_VARIABLE_EQUALS_VARIABLE, "$scratch", seqScratch);
                        Op(INT_SET_BIT, "$scratch");
                    Op(INT_END_IF);
                    Op(INT_IF_BIT_SET, "$scratch");

                        // Also do the `absolute value' calculation while
                        // we're at it.
                        Op(INT_SET_VARIABLE_TO_VARIABLE, convertState, var);
                        Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch2", ' ');
                        Op(INT_IF_VARIABLE_LES_LITERAL, var, (SWORD)0);
                            Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch2", '-');
                            Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch",
                                (SWORD)0);
                            Op(INT_SET_VARIABLE_SUBTRACT, convertState,
                                "$scratch", var, 0);
                        Op(INT_END_IF);

                    Op(INT_END_IF);
                }
                else
                {
                    // just another character
                    Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch", i);
                    Op(INT_IF_VARIABLE_EQUALS_VARIABLE, "$scratch", seqScratch);
                        Op(INT_SET_VARIABLE_TO_LITERAL, "$scratch2",
                            outputChars[i]);
                    Op(INT_END_IF);
                }
            }

            Op(INT_IF_VARIABLE_LES_LITERAL, seqScratch, (SWORD)0);
            Op(INT_ELSE);
                Op(INT_SET_BIT, "$scratch");
                Op(INT_UART_SEND, "$scratch2", "$scratch");
                Op(INT_INCREMENT_VARIABLE, seq);
            Op(INT_END_IF);

            // Rung-out state: true if we're still running, else false
            Op(INT_CLEAR_BIT, stateInOut);
            Op(INT_IF_VARIABLE_LES_LITERAL, seq, steps);
                Op(INT_SET_BIT, stateInOut);
            Op(INT_END_IF);
            break;
        }
        case ELEM_OPEN:
            Op(INT_CLEAR_BIT, stateInOut);
            break;

        case ELEM_SHORT:
            // goes straight through
            break;

        case ELEM_PLACEHOLDER:
            Error("Empty row; delete it or add instructions before compiling.");
            CompileError();
            break;

        default:
            oops();
            break;
    }

    if(which != ELEM_SERIES_SUBCKT && which != ELEM_PARALLEL_SUBCKT) {
        // then it is a leaf; let the simulator know which leaf it
        // should be updating for display purposes
        SimState(&(l->poweredAfter), stateInOut);
    }
}

//-----------------------------------------------------------------------------
// Generate intermediate code for the entire program. Return TRUE if it worked,
// else FALSE.
//-----------------------------------------------------------------------------
BOOL GenerateIntermediateCode(void)
{
    GenSymCountParThis = 0;
    GenSymCountParOut = 0;
    GenSymCountOneShot = 0;
    GenSymCountFormattedString = 0;

    // The EEPROM addresses for the 'Make Persistent' op are assigned at
    // int code generation time.
    EepromAddrFree = 0;

    IntCodeLen = 0;
    memset(IntCode, 0, sizeof(IntCode));

    if(setjmp(CompileErrorBuf) != 0)
    {
        return FALSE;
    }

    Op(INT_SET_BIT, "$mcr");

    int i;
    for(i = 0; i < Prog.numRungs; i++)
    {
        if(Prog.rungs[i]->count == 1 && Prog.rungs[i]->contents[0].which == ELEM_COMMENT)
        {
            // nothing to do for this one
            continue;
        }
        Comment("");
        Comment("start rung %d", i+1);
        Op(INT_COPY_BIT_TO_BIT, "$rung_top", "$mcr");
        SimState(&(Prog.rungPowered[i]), "$rung_top"); //may be able to get rid of that later
        IntCodeFromCircuit(ELEM_SERIES_SUBCKT, Prog.rungs[i], "$rung_top");
    }

    return TRUE;
}
