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
// Routines common to the code generators for all processor architectures.
// Jonathan Westhues, Nov 2004
//-----------------------------------------------------------------------------
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "oplc_compiler.h"


// If we encounter an error while compiling then it's convenient to break
// out of the possibly-deeply-recursed function we're in.
jmp_buf CompileErrorBuf;

// Assignment of the internal relays to memory, efficient, one bit per
// relay.
static struct {
    char    name[MAX_NAME_LEN];
    DWORD   addr;
    int     bit;
    BOOL    assignedTo;
} InternalRelays[MAX_IO];
static int InternalRelayCount;

// Assignment of the `variables,' used for timers, counters, arithmetic, and
// other more general things. Allocate 2 octets (16 bits) per.
static struct {
    char    name[MAX_NAME_LEN];
    DWORD   addrl;
    DWORD   addrh;
} Variables[MAX_IO];
static int VariableCount;

#define NO_MEMORY   0xffffffff
static DWORD    NextBitwiseAllocAddr;
static int      NextBitwiseAllocBit;
static int      MemOffset;

//-----------------------------------------------------------------------------
// Forget what memory has been allocated on the target, so we start from
// everything free.
//-----------------------------------------------------------------------------
void AllocStart(void)
{
    NextBitwiseAllocAddr = NO_MEMORY;
    MemOffset = 0;
    InternalRelayCount = 0;
    VariableCount = 0;
}

//-----------------------------------------------------------------------------
// Return the address of a previously unused octet of RAM on the target, or
// signal an error if there is no more available.
//-----------------------------------------------------------------------------
DWORD AllocOctetRam(void)
{
    if(MemOffset >= Prog.mcu->ram[0].len) {
        Error("Out of memory; simplify program or choose "
            "microcontroller with more memory.");
            CompileError();
    }

    MemOffset++;
    return Prog.mcu->ram[0].start + MemOffset - 1;
}

//-----------------------------------------------------------------------------
// Return the address (octet address) and bit of a previously unused bit of
// RAM on the target.
//-----------------------------------------------------------------------------
void AllocBitRam(DWORD *addr, int *bit)
{
    if(NextBitwiseAllocAddr != NO_MEMORY) {
        *addr = NextBitwiseAllocAddr;
        *bit = NextBitwiseAllocBit;
        NextBitwiseAllocBit++;
        if(NextBitwiseAllocBit > 7) {
            NextBitwiseAllocAddr = NO_MEMORY;
        }
        return;
    }

    *addr = AllocOctetRam();
    *bit = 0;
    NextBitwiseAllocAddr = *addr;
    NextBitwiseAllocBit = 1;
}

//-----------------------------------------------------------------------------
// Return the address (octet) and bit of the bit in memory that represents the
// given input or output pin. Raises an internal error if the specified name
// is not present in the I/O list or a user error if a pin has not been
// assigned to that I/O name. Will allocate if it no memory allocated for it
// yet, else will return the previously allocated bit.
//-----------------------------------------------------------------------------
static void MemForPin(char *name, DWORD *addr, int *bit, BOOL asInput)
{
    int i;
    for(i = 0; i < Prog.io.count; i++) {
        if(strcmp(Prog.io.assignment[i].name, name)==0)
            break;
    }
    if(i >= Prog.io.count) oops();

    if(asInput && Prog.io.assignment[i].type == IO_TYPE_DIG_OUTPUT) oops();
    if(!asInput && Prog.io.assignment[i].type == IO_TYPE_DIG_INPUT) oops();

    int pin = Prog.io.assignment[i].pin;
    for(i = 0; i < Prog.mcu->pinCount; i++) {
        if(Prog.mcu->pinInfo[i].pin == pin)
            break;
    }
    if(i >= Prog.mcu->pinCount) {
        Error("Must assign pins for all I/O.\r\n\r\n"
            "'%s' is not assigned.", name);
        CompileError();
    }
    McuIoPinInfo *iop = &Prog.mcu->pinInfo[i];

    if(asInput) {
        *addr = Prog.mcu->inputRegs[iop->port - 'A'];
    } else {
        *addr = Prog.mcu->outputRegs[iop->port - 'A'];
    }
    *bit = iop->bit;
}

//-----------------------------------------------------------------------------
// Determine the mux register settings to read a particular ADC channel.
//-----------------------------------------------------------------------------
BYTE MuxForAdcVariable(char *name)
{
    int i;
    for(i = 0; i < Prog.io.count; i++) {
        if(strcmp(Prog.io.assignment[i].name, name)==0)
            break;
    }
    if(i >= Prog.io.count) oops();

    int j;
    for(j = 0; j < Prog.mcu->adcCount; j++) {
        if(Prog.mcu->adcInfo[j].pin == Prog.io.assignment[i].pin) {
            break;
        }
    }
    if(j == Prog.mcu->adcCount) {
        Error("Must assign pins for all ADC inputs (name '%s').", name);
        CompileError();
    }

    return Prog.mcu->adcInfo[j].muxRegValue;
}

//-----------------------------------------------------------------------------
// Allocate the two octets (16-bit count) for a variable, used for a variety
// of purposes.
//-----------------------------------------------------------------------------
void MemForVariable(char *name, DWORD *addrl, DWORD *addrh)
{
    int i;
    for(i = 0; i < VariableCount; i++) {
        if(strcmp(name, Variables[i].name)==0) break;
    }
    if(i >= MAX_IO) {
        Error("Internal limit exceeded (number of vars)");
        CompileError();
    }
    if(i == VariableCount) {
        VariableCount++;
        strcpy(Variables[i].name, name);
        Variables[i].addrl = AllocOctetRam();
        Variables[i].addrh = AllocOctetRam();
    }
    *addrl = Variables[i].addrl;
    *addrh = Variables[i].addrh;
}

//-----------------------------------------------------------------------------
// Allocate or retrieve the bit of memory assigned to an internal relay or
// other thing that requires a single bit of storage.
//-----------------------------------------------------------------------------
static void MemForBitInternal(char *name, DWORD *addr, int *bit, BOOL writeTo)
{
    int i;
    for(i = 0; i < InternalRelayCount; i++) {
        if(strcmp(name, InternalRelays[i].name)==0)
            break;
    }
    if(i >= MAX_IO) {
        Error("Internal limit exceeded (number of vars)");
        CompileError();
    }
    if(i == InternalRelayCount) {
        InternalRelayCount++;
        strcpy(InternalRelays[i].name, name);
        AllocBitRam(&InternalRelays[i].addr, &InternalRelays[i].bit);
        InternalRelays[i].assignedTo = FALSE;
    }

    *addr = InternalRelays[i].addr;
    *bit = InternalRelays[i].bit;
    if(writeTo) {
        InternalRelays[i].assignedTo = TRUE;
    }
}

//-----------------------------------------------------------------------------
// Retrieve the bit to read to determine whether a set of contacts is open
// or closed. Contacts could be internal relay, output pin, or input pin,
// or one of the internal state variables ($xxx) from the int code generator.
//-----------------------------------------------------------------------------
void MemForSingleBit(char *name, BOOL forRead, DWORD *addr, int *bit)
{
    switch(name[0]) {
        case 'X':
            if(!forRead) oops();
            MemForPin(name, addr, bit, TRUE);
            break;

        case 'Y':
            MemForPin(name, addr, bit, FALSE);
            break;

        case 'R':
        case '$':
            MemForBitInternal(name, addr, bit, !forRead);
            break;

        default:
            oops();
            break;
    }
}

//-----------------------------------------------------------------------------
// Retrieve the bit to write to set the state of an output.
//-----------------------------------------------------------------------------
void MemForCoil(char *name, DWORD *addr, int *bit)
{
    switch(name[0]) {
        case 'Y':
            MemForPin(name, addr, bit, FALSE);
            break;

        case 'R':
            MemForBitInternal(name, addr, bit, TRUE);
            break;

        default:
            oops();
            break;
    }
}

//-----------------------------------------------------------------------------
// Do any post-compilation sanity checks necessary.
//-----------------------------------------------------------------------------
void MemCheckForErrorsPostCompile(void)
{
    int i;
    for(i = 0; i < InternalRelayCount; i++) {
        if(!InternalRelays[i].assignedTo) {
            Error(
               "Internal relay '%s' never assigned; add its coil somewhere.",
                InternalRelays[i].name);
            CompileError();
        }
    }
}

//-----------------------------------------------------------------------------
// From the I/O list, determine which pins are inputs and which pins are
// outputs, and pack that in 8-bit format as we will need to write to the
// TRIS or DDR registers. ADC pins are neither inputs nor outputs.
//-----------------------------------------------------------------------------
void BuildDirectionRegisters(BYTE *isInput, BYTE *isOutput)
{
    memset(isOutput, 0x00, MAX_IO_PORTS);
    memset(isInput, 0x00, MAX_IO_PORTS);

    BOOL usedUart = UartFunctionUsed();
    BOOL usedPwm  = PwmFunctionUsed();

    int i;
    for(i = 0; i < Prog.io.count; i++) {
        int pin = Prog.io.assignment[i].pin;

        if(Prog.io.assignment[i].type == IO_TYPE_DIG_OUTPUT ||
           Prog.io.assignment[i].type == IO_TYPE_DIG_INPUT)
        {
            int j;
            for(j = 0; j < Prog.mcu->pinCount; j++) {
                McuIoPinInfo *iop = &Prog.mcu->pinInfo[j];
                if(iop->pin == pin) {
                    if(Prog.io.assignment[i].type == IO_TYPE_DIG_INPUT) {
                        isInput[iop->port - 'A'] |= (1 << iop->bit);
                    } else {
                        isOutput[iop->port - 'A'] |= (1 << iop->bit);
                    }
                    break;
                }
            }
            if(j >= Prog.mcu->pinCount) {
                Error("Must assign pins for all I/O.\r\n\r\n"
                    "'%s' is not assigned.",
                    Prog.io.assignment[i].name);
                CompileError();
            }

            if(usedUart &&
                (pin == Prog.mcu->uartNeeds.rxPin ||
                 pin == Prog.mcu->uartNeeds.txPin))
            {
                Error("UART in use; pins %d and %d reserved for that.",
                    Prog.mcu->uartNeeds.rxPin, Prog.mcu->uartNeeds.txPin);
                CompileError();
            }

            if(usedPwm && pin == Prog.mcu->pwmNeedsPin) {
                Error("PWM in use; pin %d reserved for that.",
                    Prog.mcu->pwmNeedsPin);
                CompileError();
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Display our boilerplate warning that the baud rate error is too high.
//-----------------------------------------------------------------------------
void ComplainAboutBaudRateError(int divisor, double actual, double err)
{
    Error("UART baud rate generator: divisor=%d actual=%.4f for %.2f%% "
          "error.\r\n"
          "\r\n"
          "This is too large; try a different baud rate (slower "
          "probably), or a crystal frequency chosen to be divisible "
          "by many common baud rates (e.g. 3.6864 MHz, 14.7456 MHz).\r\n"
          "\r\n"
          "Code will be generated anyways but serial may be "
          "unreliable or completely broken.", divisor, actual, err);
}

//-----------------------------------------------------------------------------
// Display our boilerplate warning that the baud rate is too slow (making
// for an overflowed divisor).
//-----------------------------------------------------------------------------
void ComplainAboutBaudRateOverflow(void)
{
    Error("UART baud rate generator: too slow, divisor overflows. "
        "Use a slower crystal or a faster baud rate.\r\n"
        "\r\n"
        "Code will be generated anyways but serial will likely be "
        "completely broken.");
}
