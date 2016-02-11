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
// Constants, structures, declarations etc. for the ANSI C ladder logic compiler
// Contains also other declarations inherited from LDmicro
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

//-----------------------------------------------
// Win32 API typedefs. This makes the code compatible with other
// platforms
#ifndef DWORD
#define WINAPI
typedef unsigned long DWORD;
typedef short WCHAR;
typedef void * HANDLE;
#define MAX_PATH    260
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef short SWORD;
typedef bool BOOL;
#define FALSE	0
#define TRUE	1
#endif


//-----------------------------------------------
// Data structures for the actual ladder logic. A rung on the ladder
// is a series subcircuit. A series subcircuit contains elements or
// parallel subcircuits. A parallel subcircuit contains elements or series
// subcircuits. An element is a set of contacts (possibly negated) or a coil.
#define MAX_ELEMENTS_IN_SUBCKT  16

#define ELEM_PLACEHOLDER        0x01
#define ELEM_SERIES_SUBCKT      0x02
#define ELEM_PARALLEL_SUBCKT    0x03
#define ELEM_PADDING            0x04
#define ELEM_COMMENT            0x05

#define ELEM_CONTACTS           0x10
#define ELEM_COIL               0x11
#define ELEM_TON                0x12
#define ELEM_TOF                0x13
#define ELEM_RTO                0x14
#define ELEM_RES                0x15
#define ELEM_ONE_SHOT_RISING    0x16
#define ELEM_ONE_SHOT_FALLING   0x17
#define ELEM_MOVE               0x18
#define ELEM_ADD                0x19
#define ELEM_SUB                0x1a
#define ELEM_MUL                0x1b
#define ELEM_DIV                0x1c
#define ELEM_EQU                0x1d
#define ELEM_NEQ                0x1e
#define ELEM_GRT                0x1f
#define ELEM_GEQ                0x20
#define ELEM_LES                0x21
#define ELEM_LEQ                0x22
#define ELEM_CTU                0x23
#define ELEM_CTD                0x24
#define ELEM_CTC                0x25
#define ELEM_SHORT              0x26
#define ELEM_OPEN               0x27
#define ELEM_READ_ADC           0x28
#define ELEM_SET_PWM            0x29
#define ELEM_UART_RECV          0x2a
#define ELEM_UART_SEND          0x2b
#define ELEM_MASTER_RELAY       0x2c
#define ELEM_SHIFT_REGISTER     0x2d
#define ELEM_LOOK_UP_TABLE      0x2e
#define ELEM_FORMATTED_STRING   0x2f
#define ELEM_PERSIST            0x30
#define ELEM_PIECEWISE_LINEAR   0x31

#define CASE_LEAF \
        case ELEM_PLACEHOLDER: \
        case ELEM_COMMENT: \
        case ELEM_COIL: \
        case ELEM_CONTACTS: \
        case ELEM_TON: \
        case ELEM_TOF: \
        case ELEM_RTO: \
        case ELEM_CTD: \
        case ELEM_CTU: \
        case ELEM_CTC: \
        case ELEM_RES: \
        case ELEM_ONE_SHOT_RISING: \
        case ELEM_ONE_SHOT_FALLING: \
        case ELEM_EQU: \
        case ELEM_NEQ: \
        case ELEM_GRT: \
        case ELEM_GEQ: \
        case ELEM_LES: \
        case ELEM_LEQ: \
        case ELEM_ADD: \
        case ELEM_SUB: \
        case ELEM_MUL: \
        case ELEM_DIV: \
        case ELEM_MOVE: \
        case ELEM_SHORT: \
        case ELEM_OPEN: \
        case ELEM_READ_ADC: \
        case ELEM_SET_PWM: \
        case ELEM_UART_SEND: \
        case ELEM_UART_RECV: \
        case ELEM_MASTER_RELAY: \
        case ELEM_SHIFT_REGISTER: \
        case ELEM_LOOK_UP_TABLE: \
        case ELEM_PIECEWISE_LINEAR: \
        case ELEM_FORMATTED_STRING: \
        case ELEM_PERSIST:

#define MAX_NAME_LEN                128
#define MAX_COMMENT_LEN             384
#define MAX_LOOK_UP_TABLE_LEN        60

typedef struct ElemSubckParallelTag ElemSubcktParallel;

typedef struct ElemCommentTag {
    char    str[MAX_COMMENT_LEN];
} ElemComment;

typedef struct ElemContactsTag {
    char    name[MAX_NAME_LEN];
    BOOL    negated;
} ElemContacts;

typedef struct ElemCoilTag {
    char    name[MAX_NAME_LEN];
    BOOL    negated;
    BOOL    setOnly;
    BOOL    resetOnly;
} ElemCoil;

typedef struct ElemTimeTag {
    char    name[MAX_NAME_LEN];
    int     delay;
} ElemTimer;

typedef struct ElemResetTag {
    char    name[MAX_NAME_LEN];
} ElemReset;

typedef struct ElemMoveTag {
    char    src[MAX_NAME_LEN];
    char    dest[MAX_NAME_LEN];
} ElemMove;

typedef struct ElemMathTag {
    char    op1[MAX_NAME_LEN];
    char    op2[MAX_NAME_LEN];
    char    dest[MAX_NAME_LEN];
} ElemMath;

typedef struct ElemCmpTag {
    char    op1[MAX_NAME_LEN];
    char    op2[MAX_NAME_LEN];
} ElemCmp;

typedef struct ElemCounterTag {
    char    name[MAX_NAME_LEN];
    int     max;
} ElemCounter;

typedef struct ElemReadAdcTag {
    char    name[MAX_NAME_LEN];
} ElemReadAdc;

typedef struct ElemSetPwmTag {
    char    name[MAX_NAME_LEN];
    int     targetFreq;
} ElemSetPwm;

typedef struct ElemUartTag {
    char    name[MAX_NAME_LEN];
} ElemUart;

typedef struct ElemShiftRegisterTag {
    char    name[MAX_NAME_LEN];
    int     stages;
} ElemShiftRegister;

typedef struct ElemLookUpTableTag {
    char    dest[MAX_NAME_LEN];
    char    index[MAX_NAME_LEN];
    int     count;
    BOOL    editAsString;
    SWORD   vals[MAX_LOOK_UP_TABLE_LEN];
} ElemLookUpTable;

typedef struct ElemPiecewiseLinearTag {
    char    dest[MAX_NAME_LEN];
    char    index[MAX_NAME_LEN];
    int     count;
    SWORD   vals[MAX_LOOK_UP_TABLE_LEN];
} ElemPiecewiseLinear;

typedef struct ElemFormattedStringTag {
    char    var[MAX_NAME_LEN];
    char    string[MAX_LOOK_UP_TABLE_LEN];
} ElemFormattedString;

typedef struct ElemPerisistTag {
    char    var[MAX_NAME_LEN];
} ElemPersist;

#define SELECTED_NONE       0
#define SELECTED_ABOVE      1
#define SELECTED_BELOW      2
#define SELECTED_RIGHT      3
#define SELECTED_LEFT       4
typedef struct ElemLeafTag {
    int     selectedState;
    BOOL    poweredAfter;
    union {
        ElemComment         comment;
        ElemContacts        contacts;
        ElemCoil            coil;
        ElemTimer           timer;
        ElemReset           reset;
        ElemMove            move;
        ElemMath            math;
        ElemCmp             cmp;
        ElemCounter         counter;
        ElemReadAdc         readAdc;
        ElemSetPwmTag       setPwm;
        ElemUart            uart;
        ElemShiftRegister   shiftRegister;
        ElemFormattedString fmtdStr;
        ElemLookUpTable     lookUpTable;
        ElemPiecewiseLinear piecewiseLinear;
        ElemPersist         persist;
    }       d;
} ElemLeaf;

typedef struct ElemSubcktSeriesTag {
    struct {
        int     which;
        union {
            void                   *any;
            ElemSubcktParallel     *parallel;
            ElemLeaf               *leaf;
        } d;
    } contents[MAX_ELEMENTS_IN_SUBCKT];
    int count;
} ElemSubcktSeries;

typedef struct ElemSubckParallelTag {
    struct {
        int     which;
        union {
            void                   *any;
            ElemSubcktSeries       *series;
            ElemLeaf               *leaf;
        } d;
    } contents[MAX_ELEMENTS_IN_SUBCKT];
    int count;
} ElemSubcktParallel;

typedef struct McuIoInfoTag McuIoInfo;

typedef struct PlcProgramSingleIoTag {
    char        name[MAX_NAME_LEN];
#define IO_TYPE_PENDING         0

#define IO_TYPE_DIG_INPUT       1
#define IO_TYPE_DIG_OUTPUT      2
#define IO_TYPE_READ_ADC        3
#define IO_TYPE_UART_TX         4
#define IO_TYPE_UART_RX         5
#define IO_TYPE_PWM_OUTPUT      6
#define IO_TYPE_INTERNAL_RELAY  7
#define IO_TYPE_TON             8
#define IO_TYPE_TOF             9
#define IO_TYPE_RTO             10
#define IO_TYPE_COUNTER         11
#define IO_TYPE_GENERAL         12
    int         type;
#define NO_PIN_ASSIGNED         0
    int         pin;
} PlcProgramSingleIo;

#define MAX_IO  512
typedef struct PlcProgramTag {
    struct {
        PlcProgramSingleIo  assignment[MAX_IO];
        int                 count;
    }           io;
    McuIoInfo  *mcu;
    int         cycleTime;
    int         mcuClock;
    int         baudRate;

#define MAX_RUNGS 99
    ElemSubcktSeries *rungs[MAX_RUNGS];
    BOOL              rungPowered[MAX_RUNGS];
    int               numRungs;
} PlcProgram;


//-----------------------------------------------
// Processor definitions. These tables tell us where to find the I/Os on
// a processor, what bit in what register goes with what pin, etc. There
// is one master SupportedMcus table, which contains entries for each
// supported microcontroller.

typedef struct McuIoPinInfoTag {
    char    port;
    int     bit;
    int     pin;
} McuIoPinInfo;

typedef struct McuAdcPinInfoTag {
    int     pin;
    BYTE    muxRegValue;
} McuAdcPinInfo;

#define ISA_AVR             0x00
#define ISA_PIC16           0x01
#define ISA_ANSIC           0x02
#define ISA_INTERPRETED     0x03

#define MAX_IO_PORTS        10
#define MAX_RAM_SECTIONS    5

typedef struct McuIoInfoTag {
    char            *mcuName;
    char             portPrefix;
    DWORD            inputRegs[MAX_IO_PORTS];         // a is 0, j is 9
    DWORD            outputRegs[MAX_IO_PORTS];
    DWORD            dirRegs[MAX_IO_PORTS];
    DWORD            flashWords;
    struct {
        DWORD            start;
        int              len;
    }                ram[MAX_RAM_SECTIONS];
    McuIoPinInfo    *pinInfo;
    int              pinCount;
    McuAdcPinInfo   *adcInfo;
    int              adcCount;
    int              adcMax;
    struct {
        int             rxPin;
        int             txPin;
    }                uartNeeds;
    int              pwmNeedsPin;
    int              whichIsa;
    BOOL             avrUseIjmp;
    DWORD            configurationWord;
} McuIoInfo;

#define NUM_SUPPORTED_MCUS 15

//-----------------------------------------------
// Function prototypes

// main.cpp
extern PlcProgram Prog;
extern char CurrentSaveFile[MAX_PATH];
extern char CurrentCompileFile[MAX_PATH];
extern McuIoInfo SupportedMcus[NUM_SUPPORTED_MCUS];
static void CompileProgram(BOOL compileAs);

// miscutil.cpp
void Error(char *str, ...);
void CheckHeap(char *file, int line);
void *CheckMalloc(size_t n);
void CheckFree(void *p);
void dbp(char *str, ...);
// memory debugging, because I often get careless; ok() will check that the
// heap used for all the program storage is not yet corrupt, and oops() if
// it is
#define ok() CheckHeap(__FILE__, __LINE__)
#define oops() { \
        dbp("bad at %d %s\n", __LINE__, __FILE__); \
        Error("Internal error at line %d file '%s'\n", __LINE__, __FILE__); \
        exit(1); \
    }

// loadsave.cpp
char * fgetsNoCR(char *buffer, int size, FILE * stream);
BOOL LoadProjectFromFile(char *filename);

// iolist.cpp
int GenerateIoList(int prevSel);
BOOL LoadIoListFromFile(FILE *f);

//circuit.cpp
ElemLeaf *AllocLeaf(void);
ElemSubcktSeries *AllocSubcktSeries(void);
ElemSubcktParallel *AllocSubcktParallel(void);
void FreeCircuit(int which, void *any);
void FreeEntireProgram(void);
void NewProgram(void);
BOOL UartFunctionUsed(void);
BOOL PwmFunctionUsed(void);

//schematic.cpp
void ForgetEverything(void);
void ForgetFromGrid(void *p);
#define DISPLAY_MATRIX_X_SIZE 16
#define DISPLAY_MATRIX_Y_SIZE 512
extern ElemLeaf *DisplayMatrix[DISPLAY_MATRIX_X_SIZE][DISPLAY_MATRIX_Y_SIZE];
extern int DisplayMatrixWhich[DISPLAY_MATRIX_X_SIZE][DISPLAY_MATRIX_Y_SIZE];
extern ElemLeaf DisplayMatrixFiller;
#define PADDING_IN_DISPLAY_MATRIX (&DisplayMatrixFiller)
#define VALID_LEAF(x) ((x) != NULL && (x) != PADDING_IN_DISPLAY_MATRIX)
extern ElemLeaf *Selected;
extern int SelectedWhich;
extern BOOL CanInsertEnd;
extern BOOL CanInsertOther;
extern BOOL CanInsertComment;

// compilecommon.cpp
void AllocStart(void);
DWORD AllocOctetRam(void);
void AllocBitRam(DWORD *addr, int *bit);
void MemForVariable(char *name, DWORD *addrl, DWORD *addrh);
BYTE MuxForAdcVariable(char *name);
void MemForSingleBit(char *name, BOOL forRead, DWORD *addr, int *bit);
void MemCheckForErrorsPostCompile(void);
void BuildDirectionRegisters(BYTE *isInput, BYTE *isOutput);
void ComplainAboutBaudRateError(int divisor, double actual, double err);
void ComplainAboutBaudRateOverflow(void);
#define CompileError() longjmp(CompileErrorBuf, 1)
extern jmp_buf CompileErrorBuf;

//intcode.cpp
BOOL GenerateIntermediateCode(void);

//ansic.cpp
void CompileAnsiC(char *outFile);
