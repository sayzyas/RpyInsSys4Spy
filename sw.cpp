#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "common.h"
#include "gpio.h"
#include "sw.h"
#include "debug.h"


bool sw_on_check( int io )
{
	bool 	rts = false;
	int 	timeout = 0;
	int 	i = 0;
	
	while(1)
	{
		while( !gpio_input(io) )
		{
			delay(10);
			if ( gpio_input(io) )
			{
				rts = false;
				break;
			}
			else if ( timeout > 35 )
			{
				while(1)
				{
					if( gpio_input(io) )
					{
						rts = true;
						break;
					}
					if( rts == true ) break;
				}
				if( rts == true ) break;
			}
			timeout++;
		}
		if( rts == true )
		{
			break;
		}
		delay(100);
		// 10 second timeout when restart button doesn't push within 10sec.
		if( i > 100 ) 
		{
			rts = false;
			break;
		}
		i++;
	}
	return rts;
}

/* Series code:   002  */
int main_sw_ctrl(void)
{
	int 	rts = NO_DETECT;
	int 	timeout = 0;
	

	while( !gpio_input(START) )
	{
		delay(10);
		if ( gpio_input(START) )
		{
			rts = NO_DETECT;
			break;
		}
		else if ( timeout > 15 )
		{
			while(1)
			{
				if( gpio_input(START) )
				{
					rts = START_DETECT;
					break;
				}
			}
		}
		timeout++;
		delay(10);
	}
	
	if( rts == START_DETECT )
	{
		if( !gpio_input( SHIGYO ) )
		{
			debug2( "Shigyo start\n" );
			rts = SHIGYO_DETECT;
		}
		else
		{
			debug2( "Kensa start\n" );
		}
	}
	return rts;
}

