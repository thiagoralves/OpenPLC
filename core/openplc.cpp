//-----------------------------------------------------------------------------
// Copyright 2015 Thiago Alves
// This file is part of the OpenPLC Software Stack.
//
// OpenPLC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenPLC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenPLC.  If not, see <http://www.gnu.org/licenses/>.
//------
//
// This is the main file for the OpenPLC. It contains the initialization
// procedures for the hardware, network and the main loop
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

using namespace std;
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "ladder.h"

#define OPLC_CYCLE			0.050

pthread_mutex_t bufferLock; //mutex for the internal buffers


void *modbusThread(void *arg)
{
	startServer(502);
}

double measureTime(struct timespec *timer_start)
{
	struct timespec timer_end;
	double time_used;

	clock_gettime(CLOCK_MONOTONIC, &timer_end);

	time_used = (timer_end.tv_sec - timer_start->tv_sec);
	time_used += (timer_end.tv_nsec - timer_start->tv_nsec) / 1000000000.0;

	return time_used;
}

int main(void)
{
	cout << "OpenPLC Software running..." << endl;

	//======================================================
	//               MUTEX INITIALIZATION
	//======================================================
	if (pthread_mutex_init(&bufferLock, NULL) != 0)
    {
        cout << "Mutex init failed" << endl;
        exit(1);
    }

	//======================================================
	//              HARDWARE INITIALIZATION
	//======================================================
    initializeHardware();
    updateBuffers();

	struct timespec timer_start;
	pthread_t thread;
	pthread_create(&thread, NULL, modbusThread, NULL);

	//======================================================
	//                    MAIN LOOP
	//======================================================
	for(;;)
	{
		clock_gettime(CLOCK_MONOTONIC, &timer_start);
		PlcCycle();
		while (measureTime(&timer_start) < OPLC_CYCLE)
		{
			updateBuffers();
		}
	}
}
