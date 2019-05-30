/* ************************************************************
 * Project:       Shietsu Polymer Project1
 * Author:        y.saito
 * Date:          2019.01.04
 * 
 * Series code:   4000
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "common.h"
#include "cap.h"
#include "debug.h"
#include "gpio.h"


// ---------------------------------------------------------
// Get the information from CAP board
// return true  success
//        false read error ( timeout )
// Series code 401**
// ---------------------------------------------------------
bool read_capinfo(
	int 		fd, 
	INFO_DATA* 	info, 
	int 		rd_size,
	error_t* 	err		// redirection out file name	
){
	bool rts = false;
	int timeout = 0;
	char buf[64];
	
	while(1)
	{
		//INFO_DATA info;
		uint8_t rdBuf[rd_size];

		delay(500);

		int ret = read(fd, rdBuf, rd_size);		// Read data from I2C
		if( ret < 0 )
		{
			delay(1000);
			if( timeout * (1000+500) > ERROR_TIMEOUT)
			{
				rts = false;
				break;
			}
			timeout++;
		}
		else
		{
			info->part_name_length = rdBuf[SIZE_OF_PARTNAME+1];

			// ***************************************************************
			// Notes:
			// There are any garbage in top of part name ?? -> pass 1st chara
			// and then, latest chara of part_name is not \0 
			// ***************************************************************
			int j = 0;
			ADMIN_PRINT( err->flg_admin, "-- Cap info ------------", err->ofname );
			for ( int i = 0; i < SIZE_OF_PARTNAME; i++ ) 
			{
				if ( i < SIZE_OF_PARTNAME )
				{
					if ( i < info->part_name_length ) 
					{
						info->part_name[i] = rdBuf[i+1]; // <== 1st chara is garbage
						j++;
					}
				}
			}
			info->part_name[j] = '\0';

			info->part_name_length = rdBuf[SIZE_OF_PARTNAME+1];		// Size of product name
			info->kensaki_no = rdBuf[SIZE_OF_PARTNAME+2];			// Numbor of inspet machine
			info->kiban_no = rdBuf[SIZE_OF_PARTNAME+3];				// Piece of CAP board
			info->kensa_kasho = rdBuf[SIZE_OF_PARTNAME+4];			// Numbor of inspect point
			info->data_packet_length = rdBuf[SIZE_OF_PARTNAME+5];	// Size of INFO data
			info->summary_cnt = rdBuf[SIZE_OF_PARTNAME+6];			// Summary
			
			sprintf(buf, "part name          : %s", info->part_name );
			ADMIN_PRINT( err->flg_admin, buf, err->ofname );
			sprintf(buf, "part name length   : %d", info->part_name_length);
			ADMIN_PRINT( err->flg_admin, buf, err->ofname );
			sprintf(buf, "kensaki number     : %d", info->kensaki_no);
			ADMIN_PRINT( err->flg_admin, buf, err->ofname );
			sprintf(buf, "kiban      number  : %d", info->kiban_no);
			ADMIN_PRINT( err->flg_admin, buf, err->ofname );
			sprintf(buf, "kensa kasyo        : %d", info->kensa_kasho);
			ADMIN_PRINT( err->flg_admin, buf, err->ofname );
			sprintf(buf, "dta Packet length  : %d", info->data_packet_length);
			ADMIN_PRINT( err->flg_admin, buf, err->ofname );
			sprintf(buf, "summary cnt        : %d", info->summary_cnt);
			ADMIN_PRINT( err->flg_admin, buf, err->ofname );
			ADMIN_PRINT( err->flg_admin, "------------------------", err->ofname );
			rts = true;
			break;
		}
	}
	return rts;
}

// Series code 402**
unsigned short b2s( unsigned char upper, unsigned char lower )
{
	uint16_t data;
	data = upper << 8;
	data |= lower;
	return data;
}

// ---------------------------------------------------------
// Get the data from CAP board
// Series code 403**
bool read_capdata(
	int cap_no,
	int fd, 
	float* raw_data, // 
	int* max_data, // 
	int* min_data, // 
	int rd_size,
	int sum_cnt,
	error_t* err,	// redirection out file name
	bool finger,
	int* end_status,
	int* id
){
	struct tm tm;
	
	char str[512];
	char str2[128];
	float ttt;
	
	bool rts = true;
	
	uint8_t rdBuf[rd_size];

	delay(350);
	*end_status = 0;
	
	int ret = read(fd, rdBuf, rd_size);		// Read data from I2C
	if( ret < 0 )
	{
		rts = false;
	}
	else
	{
		time_t t = time(NULL);
		localtime_r(&t, &tm);
		
		*id = rdBuf[1];
		
		if( ( 11 <= rdBuf[1] ) && ( rdBuf[1] <= err->capid_eno ) ) // valid data
		{
			sprintf( str, "%02d [%02d:%02d:%02d] %2d(%02d) ", rdBuf[1], tm.tm_hour, tm.tm_min, tm.tm_sec, cap_no, fd );
			for( int i = 1 ; i < NUMBER_OF_DATA+1 ; i++ ) 
			{
				int temp = b2s( rdBuf[2*i], rdBuf[2*i+1] );
				if( rdBuf[1] == 11 )
				{
					*(raw_data + (i-1)) = (float)temp;
				}
				else if( rdBuf[1] == 12 )
				{
					ttt = 0;
					ttt = ((float)temp / (float)sum_cnt) * 10;
					ttt = floor( ttt + 0.5 );
					ttt /= 10;
					*(raw_data + (i-1)) += ttt;
				}
				else if( rdBuf[1] == 13 )
				{
					*(max_data + (i-1)) = temp;
				}
				else if( rdBuf[1] == 14 )
				{
					*(min_data + (i-1)) = temp;
				}
				
				
				if( rdBuf[1] == err->capid_eno ) // final packet
				{
					if( finger == false )
					{
						gpio_output( CONTINUE, ON );
					}				
					*end_status = 1;
				}
				sprintf( str2, "%04d ", temp);
				strcat( str, str2 );
			}
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
		}
		delay(50);
	}
	return rts;
}
