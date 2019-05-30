#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "common.h"
#include "gpio.h"

int gpio_input( int32_t pin )
{
    pinMode( pin, INPUT );
	pullUpDnControl( pin, PUD_UP );
	int rts = digitalRead( pin );
	return rts;	
}

void gpio_output( int32_t pin, int32_t val )
{
    pinMode(pin, OUTPUT);
	digitalWrite( pin, val );
}

void gpio_init( void )
{
	wiringPiSetupGpio(); 		// Setup WiringPI (using Broadcam pin numbor)
	
	delay( 1000 );

	gpio_output(CAP_INFO, OFF);		// Info OFF
	gpio_output(CAP_RESET, ON);		// CAP Reset

	gpio_output(RUNNING, OFF);		// 起動時、RUNNING OFF。
	gpio_output(CONTINUE, OFF);		// 起動時、CONTINUE OFF。
	gpio_output(JudgeNG, OFF);		// 起動時、JUDGE OFF。
	gpio_output(JudgeOK, OFF);		// 起動時、JUDGE OFF。
	gpio_output(ERROR, OFF);		// 起動時、ERROR OFF。	
}


// GPIO CAP Control
void gpio_cap_reset()
{
	gpio_output(CAP_INFO, OFF);
	gpio_output(CAP_RESET, ON);
}

void gpio_cap_readInfo()
{
	gpio_output(CAP_INFO, ON);
	gpio_output(CAP_RESET, OFF);
}

void gpio_cap_readData()
{
	gpio_output(CAP_INFO, OFF);
	gpio_output(CAP_RESET, OFF);
}

// IIC 
void read_temphum( float* tmp_out, float* hum_out )
{
	int temp_buf, humi_buf;
	double temp, humi;

	unsigned char rdBuf[6];
	unsigned char wtBuf[2] = {0x2c, 0x06};

	int fd = wiringPiI2CSetup( IIC_ID_TMPHUM );

	delay(50);

	write(fd, wtBuf, 2);

	delay(50);

	read(fd, rdBuf, 6);		// Read data from I2C

	temp_buf = (int)(256 * rdBuf[0] + rdBuf[1]);
	temp = -45 + ((double)(175*temp_buf))/65535;
	humi_buf = (int)(256 * rdBuf[3] + rdBuf[4]);
	humi = ((double)(100*humi_buf))/65535;
	
	*tmp_out = (float)temp;
	*hum_out = (float)humi;

	// printf("Temp %.2f / Humidity %.2f\n", temp, humi);
}
