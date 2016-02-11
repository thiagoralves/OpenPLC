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
// Description of the intermediate code that we generate. The routines in
// intcode.cpp generate this intermediate code from the program source. Then
// either we simulate the intermediate code with the routines in simulate.cpp
// or we convert it to PIC or AVR instructions so that it can run on a
// real device.
// Jonathan Westhues, Nov 2004
//-----------------------------------------------------------------------------

#ifndef __INTCODE_H
#define __INTCODE_H

#define INT_SET_BIT                              1
#define INT_CLEAR_BIT                            2
#define INT_COPY_BIT_TO_BIT                      3
#define INT_SET_VARIABLE_TO_LITERAL              4
#define INT_SET_VARIABLE_TO_VARIABLE             5
#define INT_INCREMENT_VARIABLE                   6
#define INT_SET_VARIABLE_ADD                     7
#define INT_SET_VARIABLE_SUBTRACT                8
#define INT_SET_VARIABLE_MULTIPLY                9
#define INT_SET_VARIABLE_DIVIDE                 10

#define INT_READ_ADC                            11
#define INT_SET_PWM                             12
#define INT_UART_SEND                           13
#define INT_UART_RECV                           14
#define INT_EEPROM_BUSY_CHECK                   15
#define INT_EEPROM_READ                         16
#define INT_EEPROM_WRITE                        17

#define INT_IF_GROUP(x) (((x) >= 50) && ((x) < 60))
#define INT_IF_BIT_SET                          50
#define INT_IF_BIT_CLEAR                        51
#define INT_IF_VARIABLE_LES_LITERAL             52
#define INT_IF_VARIABLE_EQUALS_VARIABLE         53
#define INT_IF_VARIABLE_GRT_VARIABLE            54

#define INT_ELSE                                60
#define INT_END_IF                              61

#define INT_SIMULATE_NODE_STATE                 80

#define INT_COMMENT                            100

// Only used for the interpretable code.
#define INT_END_OF_PROGRAM                     255

#if !defined(INTCODE_H_CONSTANTS_ONLY)
    typedef struct IntOpTag {
        int         op;
        char        name1[MAX_NAME_LEN];
        char        name2[MAX_NAME_LEN];
        char        name3[MAX_NAME_LEN];
        SWORD       literal;
        BOOL       *poweredAfter;
    } IntOp;

    #define MAX_INT_OPS     (1024*16)
    extern IntOp IntCode[MAX_INT_OPS];
    extern int IntCodeLen;
#endif


#endif
