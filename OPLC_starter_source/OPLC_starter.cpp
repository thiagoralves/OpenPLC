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
// This program is responsible to start the OpenPLC and monitor constantly if
// it is running or not.
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

//-----------------------------------------------------------------------------
// Helper function - Makes the running thread sleep for the ammount of time
// in milliseconds
//-----------------------------------------------------------------------------
void sleep_ms(int milliseconds)
{
	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
}


int main()
{
	while(1)
	{
		printf("Starting OpenPLC...\n");

		if (!fork())
		{
			//executes OpenPLC
			execl("./core/openplc", "./core/openplc", (char *) 0);
		}

		else
		{
			wait(0);
			printf("OpenPLC crashed! Restarting...\n");
		}

		sleep_ms(1000); //wait until the sockets are closed
	}
}
