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
// The table of supported MCUs, used to determine where the IOs are, what
// instruction set, what init code, etc.
// Jonathan Westhues, Oct 2004
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ATmega128 or ATmega64

McuIoPinInfo AvrAtmega128_64TQFPIoPinInfo[] = {
    { 'E',  0,  2 },
    { 'E',  1,  3 },
    { 'E',  2,  4 },
    { 'E',  3,  5 },
    { 'E',  4,  6 },
    { 'E',  5,  7 },
    { 'E',  6,  8 },
    { 'E',  7,  9 },

    { 'B',  0, 10 },
    { 'B',  1, 11 },
    { 'B',  2, 12 },
    { 'B',  3, 13 },
    { 'B',  4, 14 },
    { 'B',  5, 15 },
    { 'B',  6, 16 },
    { 'B',  7, 17 },

    { 'G',  3, 18 },
    { 'G',  4, 19 },

    { 'D',  0, 25 },
    { 'D',  1, 26 },
    { 'D',  2, 27 },
    { 'D',  3, 28 },
    { 'D',  4, 29 },
    { 'D',  5, 30 },
    { 'D',  6, 31 },
    { 'D',  7, 32 },

    { 'G',  0, 33 },
    { 'G',  1, 34 },

    { 'C',  0, 35 },
    { 'C',  1, 36 },
    { 'C',  2, 37 },
    { 'C',  3, 38 },
    { 'C',  4, 39 },
    { 'C',  5, 40 },
    { 'C',  6, 41 },
    { 'C',  7, 42 },

    { 'G',  2, 43 },

    { 'A',  7, 44 },
    { 'A',  6, 45 },
    { 'A',  5, 46 },
    { 'A',  4, 47 },
    { 'A',  3, 48 },
    { 'A',  2, 49 },
    { 'A',  1, 50 },
    { 'A',  0, 51 },

    { 'F',  7, 54 },
    { 'F',  6, 55 },
    { 'F',  5, 56 },
    { 'F',  4, 57 },
    { 'F',  3, 58 },
    { 'F',  2, 59 },
    { 'F',  1, 60 },
    { 'F',  0, 61 },
};

McuAdcPinInfo AvrAtmega128_64TQFPAdcPinInfo[] = {
    { 61, 0x00 },
    { 60, 0x01 },
    { 59, 0x02 },
    { 58, 0x03 },
    { 57, 0x04 },
    { 56, 0x05 },
    { 55, 0x06 },
    { 54, 0x07 },
};


//-----------------------------------------------------------------------------
// ATmega162

McuIoPinInfo AvrAtmega162IoPinInfo[] = {
    { 'B',  0,  1 },
    { 'B',  1,  2 },
    { 'B',  2,  3 },
    { 'B',  3,  4 },
    { 'B',  4,  5 },
    { 'B',  5,  6 },
    { 'B',  6,  7 },
    { 'B',  7,  8 },
    { 'D',  0, 10 },
    { 'D',  1, 11 },
    { 'D',  2, 12 },
    { 'D',  3, 13 },
    { 'D',  4, 14 },
    { 'D',  5, 15 },
    { 'D',  6, 16 },
    { 'D',  7, 17 },
    { 'C',  0, 21 },
    { 'C',  1, 22 },
    { 'C',  2, 23 },
    { 'C',  3, 24 },
    { 'C',  4, 25 },
    { 'C',  5, 26 },
    { 'C',  6, 27 },
    { 'C',  7, 28 },
    { 'E',  2, 29 },
    { 'E',  1, 30 },
    { 'E',  0, 31 },
    { 'A',  7, 32 },
    { 'A',  6, 33 },
    { 'A',  5, 34 },
    { 'A',  4, 35 },
    { 'A',  3, 36 },
    { 'A',  2, 37 },
    { 'A',  1, 38 },
    { 'A',  0, 39 },
};


//-----------------------------------------------------------------------------
// ATmega16 or ATmega32

McuIoPinInfo AvrAtmega16or32IoPinInfo[] = {
    { 'B',  0,  1 },
    { 'B',  1,  2 },
    { 'B',  2,  3 },
    { 'B',  3,  4 },
    { 'B',  4,  5 },
    { 'B',  5,  6 },
    { 'B',  6,  7 },
    { 'B',  7,  8 },
    { 'D',  0, 14 },
    { 'D',  1, 15 },
    { 'D',  2, 16 },
    { 'D',  3, 17 },
    { 'D',  4, 18 },
    { 'D',  5, 19 },
    { 'D',  6, 20 },
    { 'D',  7, 21 },
    { 'C',  0, 22 },
    { 'C',  1, 23 },
    { 'C',  2, 24 },
    { 'C',  3, 25 },
    { 'C',  4, 26 },
    { 'C',  5, 27 },
    { 'C',  6, 28 },
    { 'C',  7, 29 },
    { 'A',  7, 33 },
    { 'A',  6, 34 },
    { 'A',  5, 35 },
    { 'A',  4, 36 },
    { 'A',  3, 37 },
    { 'A',  2, 38 },
    { 'A',  1, 39 },
    { 'A',  0, 40 },
};

McuAdcPinInfo AvrAtmega16or32AdcPinInfo[] = {
    { 40, 0x00 },
    { 39, 0x01 },
    { 38, 0x02 },
    { 37, 0x03 },
    { 36, 0x04 },
    { 35, 0x05 },
    { 34, 0x06 },
    { 33, 0x07 },
};


//-----------------------------------------------------------------------------
// ATmega8

McuIoPinInfo AvrAtmega8IoPinInfo[] = {
    { 'D',  0,  2 },
    { 'D',  1,  3 },
    { 'D',  2,  4 },
    { 'D',  3,  5 },
    { 'D',  4,  6 },
    { 'D',  5, 11 },
    { 'D',  6, 12 },
    { 'D',  7, 13 },
    { 'B',  0, 14 },
    { 'B',  1, 15 },
    { 'B',  2, 16 },
    { 'B',  3, 17 },
    { 'B',  4, 18 },
    { 'B',  5, 19 },
    { 'C',  0, 23 },
    { 'C',  1, 24 },
    { 'C',  2, 25 },
    { 'C',  3, 26 },
    { 'C',  4, 27 },
    { 'C',  5, 28 },
};

McuAdcPinInfo AvrAtmega8AdcPinInfo[] = {
    { 23, 0x00 }, // ADC0
    { 24, 0x01 },
    { 25, 0x02 },
    { 26, 0x03 },
    { 27, 0x04 },
    { 28, 0x05 }, // ADC5
};


//-----------------------------------------------------------------------------
// A variety of 18-pin PICs that share the same digital IO assignment.

McuIoPinInfo Pic18PinIoInfo[] = {
    { 'A',  2,  1 },
    { 'A',  3,  2 },
    { 'A',  4,  3 },
    { 'A',  5,  4 },
    { 'B',  0,  6 },
    { 'B',  1,  7 },
    { 'B',  2,  8 },
    { 'B',  3,  9 },
    { 'B',  4, 10 },
    { 'B',  5, 11 },
    { 'B',  6, 12 },
    { 'B',  7, 13 },
    { 'A',  6, 15 },
    { 'A',  7, 16 },
    { 'A',  0, 17 },
    { 'A',  1, 18 },
};

McuAdcPinInfo Pic16f819AdcPinInfo[] = {
    {  1, 0x02 },
    {  2, 0x03 },
    {  3, 0x04 },
    { 17, 0x00 },
    { 18, 0x01 },
};

McuAdcPinInfo Pic16f88AdcPinInfo[] = {
    {  1, 0x02 },
    {  2, 0x03 },
    {  3, 0x04 },
    { 12, 0x05 },
    { 13, 0x06 },
    { 17, 0x00 },
    { 18, 0x01 },
};


//-----------------------------------------------------------------------------
// PIC16F877

McuIoPinInfo Pic16f877IoPinInfo[] = {
    { 'A',  0,  2 },
    { 'A',  1,  3 },
    { 'A',  2,  4 },
    { 'A',  3,  5 },
    { 'A',  4,  6 },
    { 'A',  5,  7 },
    { 'E',  0,  8 },
    { 'E',  1,  9 },
    { 'E',  2, 10 },
    { 'C',  0, 15 },
    { 'C',  1, 16 },
    { 'C',  2, 17 },
    { 'C',  3, 18 },
    { 'D',  0, 19 },
    { 'D',  1, 20 },
    { 'D',  2, 21 },
    { 'D',  3, 22 },
    { 'C',  4, 23 },
    { 'C',  5, 24 },
    { 'C',  6, 25 },
    { 'C',  7, 26 },
    { 'D',  4, 27 },
    { 'D',  5, 28 },
    { 'D',  6, 29 },
    { 'D',  7, 30 },
    { 'B',  0, 33 },
    { 'B',  1, 34 },
    { 'B',  2, 35 },
    { 'B',  3, 36 },
    { 'B',  4, 37 },
    { 'B',  5, 38 },
    { 'B',  6, 39 },
    { 'B',  7, 40 },
};

McuAdcPinInfo Pic16f877AdcPinInfo[] = {
    {  2,   0x00 },
    {  3,   0x01 },
    {  4,   0x02 },
    {  5,   0x03 },
    {  7,   0x04 },
    {  8,   0x05 },
    {  9,   0x06 },
    { 10,   0x07 },
};


//-----------------------------------------------------------------------------
// PIC16F876

McuIoPinInfo Pic16f876IoPinInfo[] = {
    { 'A', 0,  2 },
    { 'A', 1,  3 },
    { 'A', 2,  4 },
    { 'A', 3,  5 },
    { 'A', 4,  6 },
    { 'A', 5,  7 },
    { 'C', 0, 11 },
    { 'C', 1, 12 },
    { 'C', 2, 13 },
    { 'C', 3, 14 },
    { 'C', 4, 15 },
    { 'C', 5, 16 },
    { 'C', 6, 17 },
    { 'C', 7, 18 },
    { 'B', 0, 21 },
    { 'B', 1, 22 },
    { 'B', 2, 23 },
    { 'B', 3, 24 },
    { 'B', 4, 25 },
    { 'B', 5, 26 },
    { 'B', 6, 27 },
    { 'B', 7, 28 },
};

McuAdcPinInfo Pic16f876AdcPinInfo[] = {
    {  2, 0x00 },
    {  3, 0x01 },
    {  4, 0x02 },
    {  5, 0x03 },
    {  7, 0x04 }
};


//-----------------------------------------------------------------------------
// PIC16F887

McuIoPinInfo Pic16f887IoPinInfo[] = {
    { 'A',  0,  2 },
    { 'A',  1,  3 },
    { 'A',  2,  4 },
    { 'A',  3,  5 },
    { 'A',  4,  6 },
    { 'A',  5,  7 },
    { 'E',  0,  8 },
    { 'E',  1,  9 },
    { 'E',  2, 10 },
    { 'C',  0, 15 },
    { 'C',  1, 16 },
    { 'C',  2, 17 },
    { 'C',  3, 18 },
    { 'D',  0, 19 },
    { 'D',  1, 20 },
    { 'D',  2, 21 },
    { 'D',  3, 22 },
    { 'C',  4, 23 },
    { 'C',  5, 24 },
    { 'C',  6, 25 },
    { 'C',  7, 26 },
    { 'D',  4, 27 },
    { 'D',  5, 28 },
    { 'D',  6, 29 },
    { 'D',  7, 30 },
    { 'B',  0, 33 },
    { 'B',  1, 34 },
    { 'B',  2, 35 },
    { 'B',  3, 36 },
    { 'B',  4, 37 },
    { 'B',  5, 38 },
    { 'B',  6, 39 },
    { 'B',  7, 40 },
};

McuAdcPinInfo Pic16f887AdcPinInfo[] = {
    {  2,   0x00 },
    {  3,   0x01 },
    {  4,   0x02 },
    {  5,   0x03 },
    {  7,   0x04 },
    {  8,   0x05 },
    {  9,   0x06 },
    { 10,   0x07 },
    { 33,   0x0c },
    { 34,   0x0a },
    { 35,   0x08 },
    { 36,   0x09 },
    { 37,   0x0b },
    { 38,   0x0d },
};


//-----------------------------------------------------------------------------
// PIC16F886

McuIoPinInfo Pic16f886IoPinInfo[] = {
    { 'A', 0,  2 },
    { 'A', 1,  3 },
    { 'A', 2,  4 },
    { 'A', 3,  5 },
    { 'A', 4,  6 },
    { 'A', 5,  7 },
    { 'C', 0, 11 },
    { 'C', 1, 12 },
    { 'C', 2, 13 },
    { 'C', 3, 14 },
    { 'C', 4, 15 },
    { 'C', 5, 16 },
    { 'C', 6, 17 },
    { 'C', 7, 18 },
    { 'B', 0, 21 },
    { 'B', 1, 22 },
    { 'B', 2, 23 },
    { 'B', 3, 24 },
    { 'B', 4, 25 },
    { 'B', 5, 26 },
    { 'B', 6, 27 },
    { 'B', 7, 28 },
};

McuAdcPinInfo Pic16f886AdcPinInfo[] = {
    {  2, 0x00 },
    {  3, 0x01 },
    {  4, 0x02 },
    {  5, 0x03 },
    {  7, 0x04 },
    { 21, 0x0c },
    { 22, 0x0a },
    { 23, 0x08 },
    { 24, 0x09 },
    { 25, 0x0b },
    { 26, 0x0d },
};


#define arraylen(x) (sizeof(x)/sizeof((x)[0]))

McuIoInfo SupportedMcus[NUM_SUPPORTED_MCUS] = {
    {
        "Atmel AVR ATmega128 64-TQFP",
        'P',
        { 0x39, 0x36, 0x33, 0x30, 0x21, 0x20, 0x63 }, // PINx
        { 0x3b, 0x38, 0x35, 0x32, 0x23, 0x62, 0x65 }, // PORTx
        { 0x3a, 0x37, 0x34, 0x31, 0x22, 0x61, 0x64 }, // DDRx
        64*1024,
        { { 0x100, 4096 } },
        AvrAtmega128_64TQFPIoPinInfo,
        arraylen(AvrAtmega128_64TQFPIoPinInfo),
        AvrAtmega128_64TQFPAdcPinInfo,
        arraylen(AvrAtmega128_64TQFPAdcPinInfo),
        1023,
        { 27, 28 },
        17,
        ISA_AVR,
        TRUE,
        0
    },
    {
        "Atmel AVR ATmega64 64-TQFP",
        'P',
        { 0x39, 0x36, 0x33, 0x30, 0x21, 0x20, 0x63 }, // PINx
        { 0x3b, 0x38, 0x35, 0x32, 0x23, 0x62, 0x65 }, // PORTx
        { 0x3a, 0x37, 0x34, 0x31, 0x22, 0x61, 0x64 }, // DDRx
        32*1024,
        { { 0x100, 4096 } },
        AvrAtmega128_64TQFPIoPinInfo,
        arraylen(AvrAtmega128_64TQFPIoPinInfo),
        AvrAtmega128_64TQFPAdcPinInfo,
        arraylen(AvrAtmega128_64TQFPAdcPinInfo),
        1023,
        { 27, 28 },
        17,
        ISA_AVR,
        TRUE,
        0
    },
    {
        "Atmel AVR ATmega162 40-PDIP",
        'P',
        { 0x39, 0x36, 0x33, 0x30, 0x25 }, // PINx
        { 0x3b, 0x38, 0x35, 0x32, 0x27 }, // PORTx
        { 0x3a, 0x37, 0x34, 0x31, 0x26 }, // DDRx
        8*1024,
        { { 0x100, 1024 } },
        AvrAtmega162IoPinInfo,
        arraylen(AvrAtmega162IoPinInfo),
        NULL,
        0,
        0,
        { 0, 0 },
        0,
        ISA_AVR,
        TRUE,
        0
    },
    {
        "Atmel AVR ATmega32 40-PDIP",
        'P',
        { 0x39, 0x36, 0x33, 0x30 }, // PINx
        { 0x3b, 0x38, 0x35, 0x32 }, // PORTx
        { 0x3a, 0x37, 0x34, 0x31 }, // DDRx
        16*1024,
        { { 0x60, 2048 } },
        AvrAtmega16or32IoPinInfo,
        arraylen(AvrAtmega16or32IoPinInfo),
        AvrAtmega16or32AdcPinInfo,
        arraylen(AvrAtmega16or32AdcPinInfo),
        1023,
        { 14, 15 },
        0,
        ISA_AVR,
        TRUE,
        0
    },
    {
        "Atmel AVR ATmega16 40-PDIP",
        'P',
        { 0x39, 0x36, 0x33, 0x30 }, // PINx
        { 0x3b, 0x38, 0x35, 0x32 }, // PORTx
        { 0x3a, 0x37, 0x34, 0x31 }, // DDRx
        8*1024,
        { { 0x60, 1024 } },
        AvrAtmega16or32IoPinInfo,
        arraylen(AvrAtmega16or32IoPinInfo),
        AvrAtmega16or32AdcPinInfo,
        arraylen(AvrAtmega16or32AdcPinInfo),
        1023,
        { 14, 15 },
        0,
        ISA_AVR,
        TRUE,
        0
    },
    {
        "Atmel AVR ATmega8 28-PDIP",
        'P',
        { 0xff, 0x36, 0x33, 0x30 }, // PINx     (but there is no xxxA)
        { 0xff, 0x38, 0x35, 0x32 }, // PORTx
        { 0xff, 0x37, 0x34, 0x31 }, // DDRx
        4*1024,
        { { 0x60, 1024 } },
        AvrAtmega8IoPinInfo,
        arraylen(AvrAtmega8IoPinInfo),
        AvrAtmega8AdcPinInfo,
        arraylen(AvrAtmega8AdcPinInfo),
        1023,
        { 2, 3 },
        17,
        ISA_AVR,
        TRUE,
        0
    },
    {
        "Microchip PIC16F628 18-PDIP or 18-SOIC",
        'R',
        { 0x05, 0x06 }, // PORTx
        { 0x05, 0x06 }, // PORTx
        { 0x85, 0x86 }, // TRISx
        2048,
        { { 0x20, 96 }, { 0xa0, 80 }, { 0x120, 48 } },
        Pic18PinIoInfo,
        arraylen(Pic18PinIoInfo),
        NULL,
        0,
        0,
        { 7, 8 },
        0,
        ISA_PIC16,
        FALSE,
        // code protection off, data code protection off, LVP disabled,
        // BOD reset enabled, RA5/nMCLR is RA5, PWRT enabled, WDT disabled,
        // HS oscillator
        0x3f62
    },
    {
        "Microchip PIC16F88 18-PDIP or 18-SOIC",
        'R',
        { 0x05, 0x06 }, // PORTx
        { 0x05, 0x06 }, // PORTx
        { 0x85, 0x86 }, // TRISx
        4096,
        { { 0x20, 96 }, { 0xa0, 80 }, { 0x120, 48 } },
        Pic18PinIoInfo,
        arraylen(Pic18PinIoInfo),
        Pic16f88AdcPinInfo,
        arraylen(Pic16f88AdcPinInfo),
        1023,
        { 8, 11 },
        0,
        ISA_PIC16,
        FALSE,
            (1 << 13) |         // CP off
            (1 << 12) |         // CCP on RB2 (doesn't matter)
            (1 << 11) |         // ICD disabled
            (3 <<  9) |         // flash write protection off
            (1 <<  8) |         // code protection off
            (0 <<  7) |         // LVP disabled
            (1 <<  6) |         // BOR enabled
            (0 <<  5) |         // RA5/nMCLR is RA5
            (0 <<  4) |         // for osc sel, later
            (0 <<  3) |         // PWRT enabled
            (0 <<  2) |         // WDT disabled
            (2 <<  0),          // HS oscillator
    },
    {
        "Microchip PIC16F819 18-PDIP or 18-SOIC",
        'R',
        { 0x05, 0x06 }, // PORTx
        { 0x05, 0x06 }, // PORTx
        { 0x85, 0x86 }, // TRISx
        2048,
        { { 0x20, 96 } },
        Pic18PinIoInfo,
        arraylen(Pic18PinIoInfo),
        Pic16f819AdcPinInfo,
        arraylen(Pic16f819AdcPinInfo),
        1023,
        { 0, 0 },
        0,
        ISA_PIC16,
        FALSE,
            (1 << 13) | // code protect off
            (1 << 12) | // CCP1 on RB2 (doesn't matter, can't use)
            (1 << 11) | // disable debugger
            (3 <<  9) | // flash protection off
            (1 <<  8) | // data protect off
            (0 <<  7) | // LVP disabled
            (1 <<  6) | // BOR enabled
            (0 <<  5) | // nMCLR/RA5 is RA5
            (0 <<  3) | // PWRTE enabled
            (0 <<  2) | // WDT disabled
            (2 <<  0),  // HS oscillator
    },
    {
        "Microchip PIC16F877 40-PDIP",
        'R',
        { 0x05, 0x06, 0x07, 0x08, 0x09 }, // PORTx
        { 0x05, 0x06, 0x07, 0x08, 0x09 }, // PORTx
        { 0x85, 0x86, 0x87, 0x88, 0x89 }, // TRISx
        8*1024,
        { { 0x20, 96 }, { 0xa0, 80 }, { 0x110, 96 }, { 0x190, 96 } },
        Pic16f877IoPinInfo,
        arraylen(Pic16f877IoPinInfo),
        Pic16f877AdcPinInfo,
        arraylen(Pic16f877AdcPinInfo),
        1023,
        { 26, 25 },
        16,
        ISA_PIC16,
        FALSE,
        // code protection off, debug off, flash write off, EE code protection
        // off, LVP disabled, BOD enabled, CP off, PWRT enabled, WDT disabled,
        // HS oscillator
        0x3f72
    },
    {
        "Microchip PIC16F876 28-PDIP or 28-SOIC",
        'R',
        { 0x05, 0x06, 0x07 }, // PORTx
        { 0x05, 0x06, 0x07 }, // PORTx
        { 0x85, 0x86, 0x87 }, // TRISx
        8*1024,
        { { 0x20, 96 }, { 0xa0, 80 }, { 0x110, 96 }, { 0x190, 96 } },
        Pic16f876IoPinInfo,
        arraylen(Pic16f876IoPinInfo),
        Pic16f876AdcPinInfo,
        arraylen(Pic16f876AdcPinInfo),
        1023,
        { 18, 17 },
        12,
        ISA_PIC16,
        FALSE,
        // code protection off, debug off, flash write off, EE code protection
        // off, LVP disabled, BOD enabled, CP off, PWRT enabled, WDT disabled,
        // HS oscillator
        0x3f72
    },
    {
        "Microchip PIC16F887 40-PDIP",
        'R',
        { 0x05, 0x06, 0x07, 0x08, 0x09 }, // PORTx
        { 0x05, 0x06, 0x07, 0x08, 0x09 }, // PORTx
        { 0x85, 0x86, 0x87, 0x88, 0x89 }, // TRISx
        8*1024,
        { { 0x20, 96 }, { 0xa0, 80 }, { 0x120, 80 }, { 0x1a0, 80 } },
        Pic16f887IoPinInfo,
        arraylen(Pic16f887IoPinInfo),
        Pic16f887AdcPinInfo,
        arraylen(Pic16f887AdcPinInfo),
        1023,
        { 26, 25 },
        16,
        ISA_PIC16,
        FALSE,
            (3 << (9+16)) | // flash write protection off
            (0 << (8+16)) | // BOR at 2.1 V
            (1 << 13) |     // ICD disabled
            (0 << 12) |     // LVP disabled
            (0 << 11) |     // fail-safe clock monitor disabled
            (0 << 10) |     // internal/external switchover disabled
            (3 <<  8) |     // brown-out detect enabled
            (1 <<  7) |     // data code protection disabled
            (1 <<  6) |     // code protection disabled
            (1 <<  5) |     // nMCLR enabled
            (0 <<  4) |     // PWRTE enabled
            (0 <<  3) |     // WDTE disabled
            (2 <<  0)       // HS oscillator

    },
    {
        "Microchip PIC16F886 28-PDIP or 28-SOIC",
        'R',
        { 0x05, 0x06, 0x07, 0x08, 0x09 }, // PORTx
        { 0x05, 0x06, 0x07, 0x08, 0x09 }, // PORTx
        { 0x85, 0x86, 0x87, 0x88, 0x89 }, // TRISx
        8*1024,
        { { 0x20, 96 }, { 0xa0, 80 }, { 0x120, 80 }, { 0x1a0, 80 } },
        Pic16f886IoPinInfo,
        arraylen(Pic16f886IoPinInfo),
        Pic16f886AdcPinInfo,
        arraylen(Pic16f886AdcPinInfo),
        1023,
        { 18, 17 },
        12,
        ISA_PIC16,
        FALSE,
            (3 << (9+16)) | // flash write protection off
            (0 << (8+16)) | // BOR at 2.1 V
            (1 << 13) |     // ICD disabled
            (0 << 12) |     // LVP disabled
            (0 << 11) |     // fail-safe clock monitor disabled
            (0 << 10) |     // internal/external switchover disabled
            (3 <<  8) |     // brown-out detect enabled
            (1 <<  7) |     // data code protection disabled
            (1 <<  6) |     // code protection disabled
            (1 <<  5) |     // nMCLR enabled
            (0 <<  4) |     // PWRTE enabled
            (0 <<  3) |     // WDTE disabled
            (2 <<  0)       // HS oscillator
    },
    {
        "ANSI C Code",
        'x',
        { 0x00 },
        { 0x00 },
        { 0x00 },
        0,
        { { 0x00, 0 } },
        NULL,
        0,
        NULL,
        0,
        0,
        { 0, 0 },
        0,
        ISA_ANSIC,
        FALSE,
        0x00
    },
    {
        "Interpretable Byte Code",
        'x',
        { 0x00 },
        { 0x00 },
        { 0x00 },
        0,
        { { 0x00, 0 } },
        NULL,
        0,
        NULL,
        0,
        0,
        { 0, 0 },
        0,
        ISA_INTERPRETED,
        FALSE,
        0x00
    }
};

