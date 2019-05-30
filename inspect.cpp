/* ************************************************************
 * Project:       Shietsu Polymer Project1
 * Author:        y.saito
 * Date:          2019.01.04
 * 
 * Series code:   3000
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "common.h"
#include "inspect.h"
#include "gpio.h"
#include "cap.h"
#include "debug.h"

// -----------------------------------------------------------------------
// Get the inspection from CAP board
// return true  read capinfo success
//        false i2c initial erroe / read cap info error
// Series code 301**
// -----------------------------------------------------------------------
bool inspect_info( 
	INFO_DATA* info, 
	error_t* err 
){
	int cap0_fd;		// Handle of left CAP
	bool rts;
	
	CAP_RESET_CONDITION();
	delay(2000);

	CAP_READ_INFO_CONDITION(); 	// Left board -- connec -- i2c because of after reset
	cap0_fd = wiringPiI2CSetup(0x10); 		// Left board -- connec -- i2c because of after reset
	if( cap0_fd < 0 )
	{
		err->error_code = 30101;
		strcpy(err->error_message, "i2c init error");
		rts = false;
	}
	else
	{
		delay(200);
		rts = read_capinfo( cap0_fd, info, SIZE_OF_CAPINFO, err );
		CAP_RESET_CONDITION();
		if( rts == true )
		{
			delay(2000);
			rts = true;
		}
		else
		{
			err->error_code = 30102;
			strcpy(err->error_message, "read_capinfo error");
			rts = false;
		}
	}
	return rts;
}

// -----------------------------------------------------------------------
// Inspection main ( finger off and finger on )
// Series code 302**
// -----------------------------------------------------------------------
bool inspect_main(
	INFO_DATA* 	info,
	bool 		finger,
	error_t*	err
){
	char str[64];
	char str2[64];
	
	char buf[254];
	char buf2[254];
	
	int ret;
	int cap_fd[15];
	
	int end_status = 0;
	
	uint8_t check_id = 0;
	int	id;
	
	int ccc = 0;

REGET:

	//CAP_RESET_CONDITION();
	//usleep(200000);
	CAP_READ_DATA_CONDITION();
	usleep(200000);
	
	ADMIN_PRINT( err->flg_admin, "Read measurement data from CAP", err->ofname );
	ADMIN_PRINT( err->flg_admin, "ID 10:Sampling, 11:Ave[int] 12:Ave[rem] 13:Max 14:Min", err->ofname );
	ADMIN_PRINT( err->flg_admin, "ID [time]____CAP(FD)________01___02___03___04___05___06___07___08___09___10___11___12___13___14___15___16___17___18___19___20___21___22___23___24___25__cs", err->ofname );
	
	ret = true;
	for( int i = 0 ; i < 43 ; i++ )
	{
		// Get CAP data from all connected CAP board. no_cap is numbor of CAP board connected.
		for( int j = 0; j < info->kiban_no; j++ )
		{
			if( finger == false ) // finger off
			{
				ret = read_capdata( j+1, info->cap_fd[j], &(info->raw_data_nf[j][0]), &(info->max_data_nf[j][0]), &(info->min_data_nf[j][0]), SIZE_OF_CAPDATA, info->summary_cnt, err, finger, &end_status, &id );
				if( ret == false )
				{
					err->error_code = 30201;
					strcpy(err->error_message, "finger off read_capdata error");
					break;
				}
				else if(( j == info->kiban_no-1 ) && ( end_status == 1 ))
				{
					sprintf( str, "cap data read count = %d", i );
					strcpy(err->error_message, str);
					ADMIN_PRINT( err->flg_admin, str, err->ofname );
					break;
				}
			}
			else // finger on
			{
				ret = read_capdata( j+1, info->cap_fd[j], &(info->raw_data_of[j][0]), &(info->max_data_of[j][0]), &(info->min_data_of[j][0]), SIZE_OF_CAPDATA, info->summary_cnt, err, finger, &end_status, &id );
				if( id == 11 )
				{
					check_id |= 0x01;
				}
				else if( id == 12 )
				{
					check_id |= 0x02;
				}
				else if( id == 13 )
				{
					check_id |= 0x04;
				}
				else if( id == 14 )
				{
					check_id |= 0x08;
				}
				
				if(( check_id == 0x08 )||( check_id == 0x02 )||( check_id == 0x04 ))
				{
					if( ccc > 10 )
					{
						ret = false;
					}
					else
					{
						ccc++;
						goto REGET;
					}
				}
				if( ret == false )
				{
					err->error_code = 30202;
					strcpy(err->error_message, "finger on read_capdata error");
					break;
				}
				else if(( j == info->kiban_no-1 ) && ( end_status == 1 ))
				{
					if( check_id != 0x0f )
					{
						err->error_code = 30203;
						sprintf( str, "Error ID check = %d", check_id );
						strcpy(err->error_message, str);
						ADMIN_PRINT( err->flg_admin, str, err->ofname );
						ret = false;
					}
					//printf( "ID check = %d", check_id );
					sprintf( str, "ID check = %d", check_id );
					ADMIN_PRINT( err->flg_admin, str, err->ofname );
					break;
				}
			}
		}
		if( end_status == 1 )
		{
			CAP_RESET_CONDITION();
			break;
		}
	}
	
	if( ret == true )
	{
		// Display average data (no=13)
		for( int j = 0; j < info->kiban_no; j++ )
		{
			strcpy( buf, "Baseline[int]_________" );
			for( int i = 1 ; i < NUMBER_OF_DATA-1 ; i++ ) 
			{
				if( finger == false )
				{
					sprintf( buf2, "%.1f ", info->raw_data_nf[j][i] );
					strcat( buf, buf2 );
				}
				else
				{
					sprintf( buf2, "%.1f ", info->raw_data_of[j][i] );
					strcat( buf, buf2 );
				}
			}
			ADMIN_PRINT( err->flg_admin, buf, err->ofname );

			strcpy( buf, "RawPeakPeak___________" );
			for( int i = 1 ; i < NUMBER_OF_DATA-1 ; i++ ) 
			{
				if( finger == false )
				{
					sprintf( buf2, "%04d ", info->max_data_nf[j][i] - info->min_data_nf[j][i] );
					strcat( buf, buf2 );
				}
				else
				{
					sprintf( buf2, "%04d ", info->max_data_of[j][i] - info->min_data_of[j][i] );
					strcat( buf, buf2 );
				}
			}
			ADMIN_PRINT( err->flg_admin, buf, err->ofname );
		}
	}
	CAP_RESET_CONDITION();
	return ret;
}

// Series code 303**
bool inspect_judgeA
(
	INFO_DATA* 			info,
	KENSA_SPEC_DATA*  	spec, 
	bool				finger,
	error_t*			err
)
{
	char str[256];
	char str2[256];
	int ng_cnt;
	
	int total_judge = true;
	
	ADMIN_PRINT( err->flg_admin, "KENSA no finger judgement start", err->ofname );

	ADMIN_PRINT( err->flg_admin, "Raw -no finger-", err->ofname );
	ADMIN_PRINT( err->flg_admin, "[index ] value    low  high     judge", err->ofname );
	ng_cnt = 0;
	
	for( int i = 0; i < info->kensa_kasho; i++ )
	{
		sprintf( str, "[%02d, %02d] %.01f : ", 
			spec->inspect_index[i][0] + 1, 
			spec->inspect_index[i][1], 
			info->raw_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ]
		);

		info->baseline[i] = info->raw_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ];

		if( 
			( info->baseline[i] < spec->baseline_lower[i] ) ||
			( info->baseline[i] > spec->baseline_upper[i] )
		){
			ng_cnt++;
			info->baseline_judge[i] = JUDGE_NG;
			total_judge = false;
		}
		else
		{
			info->baseline_judge[i] = JUDGE_OK;
		}
		sprintf( str2, "%04d %04d ==> %d", spec->baseline_lower[i], spec->baseline_upper[i], info->baseline_judge[i]);
		strcat( str, str2 );
		ADMIN_PRINT( err->flg_admin, str, err->ofname );
	}
	info->baseline_judge_count = ng_cnt;
	info->ng_button_cnt = info->baseline_judge_count;
	ADMIN_PRINT( err->flg_admin, "Max-Min -no finger-", err->ofname );
	ADMIN_PRINT( err->flg_admin, "[index ] value  low  high    judge", err->ofname );
	ng_cnt = 0;

	for( int i = 0; i < info->kensa_kasho; i++ )
	{
		sprintf( str, "[%02d, %02d] %04d : ", 
			spec->inspect_index[i][0] + 1, 
			spec->inspect_index[i][1], 
			info->max_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ] - info->min_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ]
		);
		info->rawpeakpeak[i] = info->max_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ] - info->min_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ];
		if( 
			( info->rawpeakpeak[i] < spec->raw_peak_peak_lower[i] ) ||
			( info->rawpeakpeak[i] > spec->raw_peak_peak_upper[i] )
		){
			ng_cnt++;
			info->rawpeakpeak_judge[i] = JUDGE_NG;
			total_judge = false;
		}
		else
		{
			info->rawpeakpeak_judge[i] = JUDGE_OK;
		}
		sprintf( str2, "%04d %04d ==> %d", spec->raw_peak_peak_lower[i], spec->raw_peak_peak_upper[i], info->rawpeakpeak_judge[i]);
		strcat( str, str2 );
		ADMIN_PRINT( err->flg_admin, str, err->ofname );
	}	info->rawpeakpeak_judge_count = ng_cnt;
	if( info->ng_button_cnt < info->rawpeakpeak_judge_count )
	{
		info->ng_button_cnt = info->rawpeakpeak_judge_count;
	}
		
	sprintf( str, "KENSA no finger judgement end [RawNG=%d][Max-Min NG=%d]", info->baseline_judge_count, info->rawpeakpeak_judge_count );
	ADMIN_PRINT( err->flg_admin, str, err->ofname );
	return total_judge;
}

// Series code 304**
bool inspect_judgeB
(
	INFO_DATA* 			info,
	KENSA_SPEC_DATA*  	spec, 
	bool				finger,
	error_t*			err
)
{
	char str[256];
	char str2[256];
	int ng_cnt;

	int total_judge = true;
	
	ADMIN_PRINT( err->flg_admin, "KENSA finger on judgement start", err->ofname );

	ADMIN_PRINT( err->flg_admin, "Diff -finger on - of-", err->ofname );
	ADMIN_PRINT( err->flg_admin, "[index ] value low  high     judge", err->ofname );
	ng_cnt = 0;
	for( int i = 0; i < info->kensa_kasho; i++ )
	{
		info->diff[i] = info->raw_data_of[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ] - info->raw_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ] ;
		sprintf( str, "[%02d, %02d] %.1f - %.1f = %.1f : ", 
			spec->inspect_index[i][0] + 1, 
			spec->inspect_index[i][1], 
			info->raw_data_of[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ],
			info->raw_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ],
			info->diff[i]
		);
		if( 
			( info->diff[i] < spec->diff_lower[i] ) ||
			( info->diff[i] > spec->diff_upper[i] )
		){
			ng_cnt++;
			info->diff_judge[i] = JUDGE_NG;
			total_judge = false;
		}
		else
		{
			info->diff_judge[i] = JUDGE_OK;
		}
		sprintf( str2, "%04d %04d ==> %d", spec->diff_lower[i], spec->diff_upper[i], info->diff_judge[i]);
		strcat( str, str2 );
		ADMIN_PRINT( err->flg_admin, str, err->ofname );
	}
	info->diff_judge_count = ng_cnt;
	if( info->ng_button_cnt < info->diff_judge_count )
	{
		info->ng_button_cnt = info->diff_judge_count;
	}
	sprintf( str, "KENSA finger on judgement end [NG=%d]", info->diff_judge_count );
	ADMIN_PRINT( err->flg_admin, str, err->ofname );
	return total_judge;
}

bool shigyo_judgeA
(
	INFO_DATA* 			info,
	SHIGYO_SPEC_DATA*  	spec, 
	bool				finger,
	error_t*			err
)
{
	char str[256];
	char str2[256];
	int ng_cnt;
	
	int total_judge = true;
	
	ADMIN_PRINT( err->flg_admin, "SHIGYO no finger judgement start", err->ofname );
	
	ADMIN_PRINT( err->flg_admin, "Raw -no finger-", err->ofname );
	ADMIN_PRINT( err->flg_admin, "[index ] value  low  high judge", err->ofname );
	ng_cnt = 0;
	for( int i = 0; i < info->kensa_kasho; i++ )
	{
		sprintf( str, "[%02d, %02d] %.01f : ", 
			spec->inspect_index[i][0] + 1, 
			spec->inspect_index[i][1], 
			info->raw_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ]
		);
		info->baseline[i] = info->raw_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ];
		if( 
			( info->baseline[i] < spec->baseline_lower[i] ) ||
			( info->baseline[i] > spec->baseline_upper[i] )
		){
			ng_cnt++;
			info->baseline_judge[i] = JUDGE_NG;
			total_judge = false;
		}
		else
		{
			info->baseline_judge[i] = JUDGE_OK;
		}
		sprintf( str2, "%04d %04d ==> %d", spec->baseline_lower[i], spec->baseline_upper[i], info->baseline_judge[i]);
		strcat( str, str2 );
		ADMIN_PRINT( err->flg_admin, str, err->ofname );
	}

	info->baseline_judge_count = ng_cnt;
	info->ng_button_cnt = info->baseline_judge_count;
	ADMIN_PRINT( err->flg_admin, "Max-Min -no finger-", err->ofname );
	ADMIN_PRINT( err->flg_admin, "[index ] value    low  high      judge", err->ofname );
	ng_cnt = 0;
	for( int i = 0; i < info->kensa_kasho; i++ )
	{
		sprintf( str, "[%02d, %02d] %04d : ",  
			spec->inspect_index[i][0] + 1, 
			spec->inspect_index[i][1], 
			info->max_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ] - info->min_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ]
		);
		info->rawpeakpeak[i] = info->max_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ] - info->min_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ];
		if( 
			( info->rawpeakpeak[i] < spec->raw_peak_peak_lower[i] ) ||
			( info->rawpeakpeak[i] > spec->raw_peak_peak_upper[i] )
		){
			ng_cnt++;
			info->rawpeakpeak_judge[i] = JUDGE_NG;
			total_judge = false;
		}
		else
		{
			info->rawpeakpeak_judge[i] = JUDGE_OK;
		}
		sprintf( str2, "%04d %04d ==> %d", spec->raw_peak_peak_lower[i], spec->raw_peak_peak_upper[i], info->rawpeakpeak_judge[i]);
		strcat( str, str2 );
		ADMIN_PRINT( err->flg_admin, str, err->ofname );
	}
	info->rawpeakpeak_judge_count = ng_cnt;
	if( info->ng_button_cnt < info->rawpeakpeak_judge_count )
	{
		info->ng_button_cnt = info->rawpeakpeak_judge_count;
	}
	sprintf( str, "SHIGYO finger no judgement end [RawBG=%d][Max-Min NG=%d]", info->baseline_judge_count, info->rawpeakpeak_judge_count );
	ADMIN_PRINT( err->flg_admin, str, err->ofname );
	return total_judge;
}

// Series code 306**
bool shigyo_judgeB
(
	INFO_DATA* 			info,
	SHIGYO_SPEC_DATA*  	spec, 
	bool				finger,
	error_t*			err
)
{
	char str[256];
	char str2[256];
	int ng_cnt;
	
	int total_judge = true;
	
	ADMIN_PRINT( err->flg_admin, "SHIGYO finger on judgement start", err->ofname );
	
	ADMIN_PRINT( err->flg_admin, "Diff -finger on-", err->ofname );
	ADMIN_PRINT( err->flg_admin, "[index ] value low high       judge", err->ofname );
	ng_cnt = 0;
	
	for( int i = 0; i < info->kensa_kasho; i++ )
	{
		info->diff[i] = info->raw_data_of[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ] - info->raw_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ] ;
		sprintf( str, "[%02d, %02d] %.1f - %.1f = %.1f : ", 
			spec->inspect_index[i][0] + 1, 
			spec->inspect_index[i][1], 
			info->raw_data_of[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ],
			info->raw_data_nf[ spec->inspect_index[i][0] ][ spec->inspect_index[i][1] ],
			info->diff[i]
		);
		if( 
			( info->diff[i] < spec->diff_lower[i] ) ||
			( info->diff[i] > spec->diff_upper[i] )
		){
			ng_cnt++;
			info->diff_judge[i] = JUDGE_NG;
			total_judge = false;
		}
		else
		{
			info->diff_judge[i] = JUDGE_OK;
		}
		sprintf( str2, "%04d %04d ==> %d", spec->diff_lower[i], spec->diff_upper[i], info->diff_judge[i]);
		strcat( str, str2 );
		ADMIN_PRINT( err->flg_admin, str, err->ofname );
	}

	info->diff_judge_count = ng_cnt;
	if( info->ng_button_cnt < info->diff_judge_count )
	{
		info->ng_button_cnt = info->diff_judge_count;
	}
	sprintf( str, "SHIGYO finger on judgement end [Diff NG=%d]", info->diff_judge_count );
	ADMIN_PRINT( err->flg_admin, str, err->ofname );
	return total_judge;
}

