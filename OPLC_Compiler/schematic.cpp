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
// Stuff for managing the schematic, mostly related to allocating memory
// to the circuit matrix.
// Actual manipulation of circuit elements happens in circuit.cpp, though.
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

using namespace std;

#include <stddef.h>
#include <string.h>

#include "oplc_compiler.h"

// Not all options all available e.g. can't delete the only relay coil in
// a rung, can't insert two coils in series, etc. Keep track of what is
// allowed so we don't corrupt our program.
BOOL CanInsertEnd;
BOOL CanInsertOther;
BOOL CanInsertComment;

// Ladder logic program is laid out on a grid program; this matrix tells
// us which leaf element is in which box on the grid, which allows us
// to determine what element has just been selected when the user clicks
// on something, for example.
ElemLeaf *DisplayMatrix[DISPLAY_MATRIX_X_SIZE][DISPLAY_MATRIX_Y_SIZE];
int DisplayMatrixWhich[DISPLAY_MATRIX_X_SIZE][DISPLAY_MATRIX_Y_SIZE];
ElemLeaf *Selected;
int SelectedWhich;

ElemLeaf DisplayMatrixFiller;


//-----------------------------------------------------------------------------
// Rub out everything from DisplayMatrix. If we don't do that before freeing
// the program (e.g. when loading a new file) then there is a race condition
// when we repaint.
//-----------------------------------------------------------------------------
void ForgetEverything(void)
{
    memset(DisplayMatrix, 0, sizeof(DisplayMatrix));
    memset(DisplayMatrixWhich, 0, sizeof(DisplayMatrixWhich));
    Selected = NULL;
    SelectedWhich = 0;
}

//-----------------------------------------------------------------------------
// Rub out freed element from the DisplayMatrix, just so we don't confuse
// ourselves too much (or access freed memory)...
//-----------------------------------------------------------------------------
void ForgetFromGrid(void *p)
{
    int i, j;
    for(i = 0; i < DISPLAY_MATRIX_X_SIZE; i++)
    {
        for(j = 0; j < DISPLAY_MATRIX_Y_SIZE; j++)
        {
            if(DisplayMatrix[i][j] == p)
            {
                DisplayMatrix[i][j] = NULL;
            }
        }
    }
    if(Selected == p)
    {
        Selected = NULL;
    }
}
