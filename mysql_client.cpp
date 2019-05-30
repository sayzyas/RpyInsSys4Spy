/* Series code:   5000  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <mysql/mysql.h>

#include "common.h"
#include "mysql_client.h"
#include "main.h"
#include "gpio.h"
#include "debug.h"


db_ctrl::db_ctrl()
{
	conn = NULL;
	resp = NULL;
}

db_ctrl::~db_ctrl()
{
}

/* Series code 501** */
bool db_ctrl::connect(error_t* err)
{
	// initial
	conn = mysql_init( NULL );
	// option setting
	unsigned int timeout= MYSQL_TIMEOUT; // timeout = 10sec
	mysql_options( 
		conn,
		MYSQL_OPT_CONNECT_TIMEOUT,
		(void *)&timeout
	);
	
//	ADMIN_PRINT( err->flg_admin, "mysql_init_pass", err->ofname );
	if( !mysql_real_connect( 
			conn,
			sql_serv,	// server address
			user,		// use account
			passwd,		// pass word
			db_name,	// data base name
			0,			// port
			NULL,		//
			0			//
		) )
	{
		err->error_code = 50101;
		strcpy( err->error_message, "ERROR: MySQL connect");	
		ADMIN_PRINT( err->flg_admin, "ERROR: MySQL connect", err->ofname );
		return false;
	}
	else
	{
		ADMIN_PRINT( err->flg_admin, "MySQL connect OK", err->ofname );
		return true;
	}
}

/* Series code 502** */
bool db_ctrl::close()
{
	mysql_close(conn);
	return true;		
}

/* Series code 503** */
int db_ctrl::queryCtrl( const char* que, error_t* err )
{
	if( mysql_query( conn, que ) )
	{
		ADMIN_PRINT( err->flg_admin, "ERROR: MySQL query", err->ofname );
		return -1;
	}
	else
	{
		resp = mysql_use_result(conn);
		int num_fields = mysql_num_fields(resp);
		return num_fields;
	}
}

/* Series code 504** */
bool db_ctrl::readQueryData( const char* que, char* date, error_t* err )
{
	if( queryCtrl( que, err ) == true )
	{
		while(( row = mysql_fetch_row(resp)) != NULL )
		{
			debug4( "%s\n", row[0] );
			strcpy( date, row[0]);
		}
		return true;
	}
	else
	{
		return false;
	}
}

/* Series code 505** */
bool db_ctrl::get_cntDateTime(error_t* err)
{
	bool dt = readQueryData("select now()", &date_time[0], err);
	bool dd = readQueryData("select current_date from dual", &date[0], err);
	bool tt = readQueryData("select current_time from dual", &time[0], err);
	
	if(( dt || dd || tt ) == false )
	{
		err->error_code = 50501;
		ADMIN_PRINT( err->flg_admin, "ERROR: get_cntDateTime", err->ofname );
		strcpy( err->error_message, "ERROR: get_cntDateTime" );		
		return false;
	}
	else
	{
		return true;
	}
}

#define MAKE_CMD_INT(data) { memset(&tmp,0x00,sizeof(tmp));sprintf(tmp,"%d",data); }
#define MAKE_CMD_FINT(data) { memset(&tmp,0x00,sizeof(tmp));sprintf(tmp,"%.01f",data); }
#define MAKE_CMD_FSTR(data) { memset(&tmp,0x00,sizeof(tmp));sprintf(tmp,"%.01f",data); }
#define MAKE_CMD_STR(data) { memset(&tmp,0x00,sizeof(tmp));sprintf(tmp,"%s",data); }

/* Series code 506** */
int db_ctrl::readTable_kensaki(  
	kensaki_t* kensaki,	// pointer of kensaki structure
	INFO_DATA* info, 	// pointer of info structure
	error_t* err
){
	int i = 0;
	int rts;
	int timeout = 0;
	char tmp[36];
	
	char str[256];
	char cmd[256];
	memset( &cmd[0] , 0x00 , sizeof(cmd) );
	
	while(1)
	{
		strcpy( cmd, "select * from jcs_j.kensaki where kensaki_id=(select max(kensaki_id) from kensaki where part_name=");
		strcat( cmd, "'");
		MAKE_CMD_STR(info->part_name)
		strcat( cmd, tmp );
		strcat( cmd, "'");
		strcat( cmd, " and kensaki_no=");
		MAKE_CMD_INT(info->kensaki_no)
		strcat( cmd, tmp );
		strcat( cmd, ")");
		ADMIN_PRINT( err->flg_admin, cmd, err->ofname );
		
		if( mysql_query( conn, cmd ))
		{
			debug4("readTable_kensaki error\n");
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 50601;
				strcpy( err->error_message, "ERROR: readTable_kensaki timeout");
				ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
				rts = -1;
				break;
			}
			timeout++;
		}
		else
		{
			resp = mysql_use_result(conn);

			while(( row = mysql_fetch_row(resp)) != NULL )
			{
				kensaki->kensaki_id 		= atoi(row[0]);
				strcpy( kensaki->registered_date, row[1] );
				strcpy( kensaki->part_name, row[2]);
				kensaki->kensaki_no 		= atoi(row[3]);
				kensaki->part_name_length 	= atoi(row[4]);
				kensaki->kiban_no 			= atoi(row[5]);
				kensaki->kensa_kasho 		= atoi(row[6]);
				kensaki->data_packet_length = atoi(row[7]);
				kensaki->summary_cnt 		= atoi(row[8]);
				i++;
			}
			ADMIN_PRINT( err->flg_admin, "-- KENSAKI table -------------------", err->ofname );
			sprintf( str, "Number of registorated datas of kensaki = %d", i);
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "Serched maximun kensaki_id data is");
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  kensaki_id          : %d", kensaki->kensaki_id );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  part_name           : %s", kensaki->part_name );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  kensaki_no          : %d", kensaki->kensaki_no );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  part_name_length    : %d", kensaki->part_name_length );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  kiban_no            : %d", kensaki->kiban_no );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  kensa_kasho         : %d", kensaki->kensa_kasho );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  data_packet_length  : %d", kensaki->data_packet_length );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  summary_cnt         : %d", kensaki->summary_cnt );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			ADMIN_PRINT( err->flg_admin, "---------------------------------", err->ofname );

			rts = i;
			break;
		}
	}
	return rts;
}

/* Series code 507** */
int db_ctrl::readTable_lot(  
	lot_t* lot,
	INFO_DATA* info,
	int kensaki_id,
	error_t* err
){
	int rts;
	int timeout = 0;
	int i = 0;
	char str[256];
	
	while(1)
	{
		memset( &str[0] , 0x00 , sizeof(str) );
		sprintf( str, "select * from jcs_j.lot where lot_id = ( select max(lot_id) from lot where kensaki_id=%d)", kensaki_id );
		ADMIN_PRINT( err->flg_admin, str, err->ofname );

		if( mysql_query( conn , str ))
		{
		//	debug4("ERROR: mysql query [readTable_lot]\n");
			rts = -1;
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 50701;
				strcpy( err->error_message, "ERROR: read lot table timeout");
				ADMIN_PRINT( err->flg_admin, "ERROR: read lot table timeout", err->ofname );
				rts = -1;
				break;
			}
			timeout++;
		}
		else
		{
			resp = mysql_use_result(conn);

			while(( row = mysql_fetch_row(resp)) != NULL )
			{
				lot->lot_id 			= atoi(row[0]);
				strcpy(lot->registered_date, row[1]);
				lot->kensaki_id 		= atoi(row[2]);
				strcpy(lot->lot_no, row[3]);
				i++;
			}
			sprintf( str, "-- LOT table --------------------" );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "Number of registorated datas of lot = %d", i);
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "Serched maximun lot_id data is [%d]", kensaki_id );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  lot_id          : %d", lot->lot_id );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  registered_date : %s", lot->registered_date );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  kensaki_id      : %d", lot->kensaki_id );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "  lot_no          : %s", lot->lot_no );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			sprintf( str, "---------------------------------" );
			ADMIN_PRINT( err->flg_admin, str, err->ofname );
			rts = i;
			break;
		}
	}
	return rts;
}

/* Series code 508** */
bool db_ctrl::readTable_shigyo_spec(
	SHIGYO_SPEC_DATA* spec,
	KENSA_SPEC_DATA* kspec,
	int kensaki_id,
	error_t* err
){
	bool rts;
	int i = 0;
	int timeout = 0;
	char tmp[36];
	char str[256];
	
	while(1)
	{
		memset( &tmp[0], 0x00 , sizeof(tmp) );
		sprintf( tmp, "%d", kensaki_id);

		memset( &str[0] , 0x00 , sizeof(str) );
		strcpy( str, "select * from jcs_j.shigyo_spec where kensaki_id=");	
		strcat( str, tmp );
		ADMIN_PRINT( err->flg_admin, str, err->ofname );
		
		if( mysql_query( conn, str ) )
		{
			debug0("Error mysql query ( select * from jcs_j.shigyo_spec where kensaki_id= )\n");
			rts = false;
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 50801;
				strcpy( err->error_message, "ERROR: readTable_shigyo_spec timeout");
				ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
				rts = false;
				break;
			}
			timeout++;
		}
		else
		{
			resp = mysql_use_result(conn);
			
			int num_fields = mysql_num_fields(resp);
			printf("num_fields=%d\n", num_fields);		

			while(( row = mysql_fetch_row(resp)) != NULL )
			{
				strcpy( &spec->registered_data[i][0], row[0] );
				spec->kensaki_id[i]				= atoi(row[1]);
				spec->button_no[i]				= atoi(row[2]);
				spec->raw_peak_peak_upper[i]	= atoi(row[3]);
				spec->raw_peak_peak_lower[i]	= atoi(row[4]);
				spec->baseline_upper[i]			= atoi(row[5]);
				spec->baseline_lower[i]		= atoi(row[6]);
				spec->diff_upper[i]				= atoi(row[7]);
				spec->diff_lower[i]				= atoi(row[8]);
				
				spec->inspect_index[i][0] = kspec->inspect_index[i][0];
				spec->inspect_index[i][1] = kspec->inspect_index[i][1];
				
				
				sprintf( str, "-- SHIGYO table --------------------" );
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "registered_data     %s", &spec->registered_data[i][0]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "kensaki_id          %d", spec->kensaki_id[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "button_no           %d", spec->button_no[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "raw_peak_peak_upper %d", spec->raw_peak_peak_upper[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "raw_peak_peak_lower %d", spec->raw_peak_peak_lower[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "baseline_upper      %d", spec->baseline_upper[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "baseline_lower      %d", spec->baseline_lower[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "diff_upper          %d", spec->diff_upper[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "diff_lower          %d", spec->diff_lower[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "------------------------------------" );
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				i++;
			}
			rts = true;
			break;
		}
	}
	return rts;
}

/* Series code 509** */
bool db_ctrl::readTable_kensa_spec(
	KENSA_SPEC_DATA* spec,
	int kensaki_id,
	error_t* err
){
	int i = 0;
	int timeout = 0;
	bool rts;
	char tmp[36];
	char str[256];
	
	while(1)
	{
		memset( &tmp[0] , 0x00 , sizeof(tmp) );
		sprintf( tmp, "%d", kensaki_id);

		memset( &str[0] , 0x00 , sizeof(str) );
		strcpy( str, "select * from jcs_j.kensa_spec where kensaki_id=");	
		strcat( str, tmp );
		ADMIN_PRINT( err->flg_admin, str, err->ofname );

		if( mysql_query( conn, str ) )
		{
			debug0("Error mysql query ( select * from jcs_j.kensa_spec where kensaki_id= )\n");
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 50901;
				strcpy( err->error_message, "ERROR: readTable_kensa_spec timeout");
				ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
				rts = false;
				break;
			}
			timeout++;
		}
		else
		{
			resp = mysql_use_result(conn);
			
			//int num_fields = mysql_num_fields(resp);
			//printf("num_fields=%d\n", num_fields);		

			while(( row = mysql_fetch_row(resp)) != NULL )
			{
				strcpy( &spec->registered_data[i][0], row[0] );
				spec->kensaki_id[i]				= atoi(row[1]);
				spec->button_no[i]				= atoi(row[2]);
				strcpy( &spec->button_name[i][0],row[3] );;
				spec->kiban_no[i]				= atoi(row[4]); // 1, 2 ...
				spec->packet_position[i]		= atoi(row[5]); // 1, 2 ...
				spec->raw_peak_peak_upper[i]	= atoi(row[6]);
				spec->raw_peak_peak_lower[i]	= atoi(row[7]);
				spec->baseline_upper[i]			= atoi(row[8]);
				spec->baseline_lower[i]			= atoi(row[9]);
				spec->diff_upper[i]				= atoi(row[10]);
				spec->diff_lower[i]				= atoi(row[11]);
				
				spec->inspect_index[i][0] = atoi(row[4]) - 1;
				spec->inspect_index[i][1] = atoi(row[5]);
				
				sprintf( str, "-- KENSA SPEC table [%d] ----------------", i );
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "registered_data     %s", &spec->registered_data[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "kensaki_id          %d", spec->kensaki_id[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "button_no           %d", spec->button_no[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "button_name         %s", &spec->button_name[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "kiban_no            %d", spec->kiban_no[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "packet_position     %d", spec->packet_position[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "raw_peak_peak_upper %d", spec->raw_peak_peak_upper[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "raw_peak_peak_lower %d", spec->raw_peak_peak_lower[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "baseline_upper      %d", spec->baseline_upper[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "baseline_lower      %d", spec->baseline_lower[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "diff_upper          %d", spec->diff_upper[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "diff_lower          %d", spec->diff_lower[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "judge_index [ %d, %d ]", spec->kiban_no[i], spec->packet_position[i]);
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				sprintf( str, "-----------------------------------------" );
				ADMIN_PRINT( err->flg_admin, str, err->ofname );
				i++;
			}
			rts = true;
			break;
		}
	}
	return rts;
}

/* Series code 510** */
bool db_ctrl::chkTable_shigyo_result(
	kensa_result_t* result,
	error_t* err
)
{
	bool rts =false;
	int timeout = 0;
	char tmp_s[11];
	int kensaki_id = 0;

	char tmp[36];
	char str[256];

	while(1)
	{
		memset( &str[0] , 0x00 , sizeof(str) );
		sprintf( str, "select kensa_date, total_judge, kensaki_id, shigyo_id from shigyo_result where shigyo_id=(select max(shigyo_id) from shigyo_result where kensaki_id=%d)", result->kensaki_id);
		ADMIN_PRINT( err->flg_admin, str, err->ofname );
		
		if( mysql_query( conn, str ) )
//		if( mysql_query( conn, "select kensa_date, total_judge, kensaki_id, shigyo_id from shigyo_result where shigyo_id=(select max(shigyo_id) from shigyo_result)" ) )
		{
			debug0("Error mysql query ( chkTable_shigyo_result\n");
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 51001;
				strcpy( err->error_message, "ERROR: chkTable_shigyo_result timeout");
				ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
				rts = false;
				break;
			}
			timeout++;
		}
		else
		{
			resp = mysql_use_result(conn);
			int s_pass;

			while(( row = mysql_fetch_row(resp)) != NULL )
			{	
				strncpy( tmp_s, row[0], 10 );
				tmp_s[10] = '\0';			
				
				if( strncmp( tmp_s, date, 10 ) == 0 )
				{
					s_pass = atoi(row[1]);
#ifdef __DEBUGPASS__
					ADMIN_PRINT( err->flg_admin, "**Pass for debugging**", err->ofname );
					s_pass = 0; // force set for program pass enen if error condition.
#endif
					if( s_pass != 0 ) // shigyo total judge ( 0 is OK, other NG position number )
					{
						err->error_code = 51002;
						sprintf( err->error_message, "ERROR: shigyo total judge NG [%d]", atoi(row[1]) );
						ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
						rts = false;
					}
					else // Shigyo OK
					{
						
						kensaki_id = atoi(row[2]); // kensaki id
						if( kensaki_id == result->kensaki_id )
						{
							result->shigyo_id = atoi(row[3]);
							sprintf( err->error_message, "shigyo id [%d]", result->shigyo_id );
							ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
							ADMIN_PRINT( err->flg_admin, "OK [ both date and judgement are OK ]", err->ofname );
							rts = true;
						}
						else
						{
							err->error_code = 51003;
							sprintf( err->error_message, "ERROR: kensaki id isn't match shigyo[%d] vs kensa [%d]", result->kensaki_id, kensaki_id );
							ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
							rts = false;
						}
					}
				}
				else
				{
					err->error_code = 51004;
					strcpy( err->error_message, "ERROR: shigyo date isn't today");
					rts = false;
				}
				if( rts == true )
				{
					break;
				}
			}
			break;
		}
	}
	return rts;
}

/* Series code 511** */
bool db_ctrl::writeTable_kensa_result(
	kensa_result_t* result, 	// pointer of info structure
	error_t* err
)
{
	int timeout = 0;
	bool rts;
	char cmd[256];
	memset( &cmd[0] , 0x00 , sizeof(cmd) );

	char tmp[48];
	strcpy( cmd, "insert into jcs_j.kensa_result values ( NULL" );
	strcat( cmd, "," );
	MAKE_CMD_INT( result->kensaki_id); strcat( cmd, tmp );
	strcat( cmd, "," );
	strcat( cmd, "'" );
	MAKE_CMD_STR( result->kensa_date ); strcat( cmd, tmp );
	strcat( cmd, "'" );
	strcat( cmd, "," );
	MAKE_CMD_INT( result->lot_id ); strcat( cmd, tmp ); strcat( cmd, "," );
	MAKE_CMD_INT( result->shigyo_id ); strcat( cmd, tmp ); strcat( cmd, "," );
	MAKE_CMD_INT( result->total_judge ); strcat( cmd, tmp ); strcat( cmd, "," );
	strcat( cmd, "'" );
	MAKE_CMD_STR( result->ip ); strcat( cmd, tmp ); 
	strcat( cmd, "'" );
	strcat( cmd, "," );
	strcat( cmd, "'" );
	MAKE_CMD_FSTR( result->temperature ); strcat( cmd, tmp ); 
	strcat( cmd, "'" );
	strcat( cmd, "," );
	strcat( cmd, "'" );
	MAKE_CMD_FSTR( result->humidity ); strcat( cmd, tmp ); 
	strcat( cmd, "'" );
	strcat( cmd, ")" );
	ADMIN_PRINT( err->flg_admin, cmd, err->ofname );

	while(1)
	{
		if( mysql_query( conn , cmd )) 
		{
			debug4("mysql query error\n");
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 51101;
				strcpy( err->error_message, "kensa table write to error");
				ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
				rts = false;
				break;
			}
			timeout++;
		}
		else
		{
			rts = true;
			break;
		}	
	}
	return rts;
}


/* Series code 512** */
bool db_ctrl::writeTable_kensa_result_detail(
	kensa_result_detail_t* result,
	error_t* err
)
{
	bool rts;
	int timeout = 0;
	char cmd[256];

	memset( &cmd[0] , 0x00 , sizeof(cmd) );	
	strcpy( cmd, "select max(kensa_id) from jcs_j.kensa_result");	
	ADMIN_PRINT( err->flg_admin, cmd, err->ofname );
	
	while(1)
	{
		if( mysql_query( conn, cmd ) )
		{
			debug4("mysql query error ( chkTable_kensa_result )\n");
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 51201;
				strcpy( err->error_message, "writeTable_kensa_result_detail to1");
				ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
				rts = false;
				break;
			}
			timeout++;
		}
		else
		{	
			char tmp[48];
			resp = mysql_use_result(conn);

			while(( row = mysql_fetch_row(resp)) != NULL )
			{
				result->kensa_id = atoi(row[0]);
			}
			strcpy( cmd, "insert into jcs_j.kensa_result_detail values ( " );
			MAKE_CMD_INT( result->kensa_id ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_INT( result->button_no ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_FINT( result->raw_result ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_INT( result->raw_judge ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_FINT( result->baseline_result ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_INT( result->baseline_judge ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_FINT( result->diff_result ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_INT( result->diff_judge ); strcat( cmd, tmp );
			strcat( cmd, " )" );
			ADMIN_PRINT( err->flg_admin, cmd, err->ofname );
			
			if( mysql_query( conn , cmd )) 
			{
				delay(1000);
				if ( timeout * 1000 > ERROR_TIMEOUT )
				{
					err->error_code = 51202;
					strcpy( err->error_message, "writeTable_kensa_result_detail to2");
					ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
					rts = false;
					break;
				}
				timeout++;
			}
			else
			{
				rts = true;
				break;
			}
		}
	}
	return rts;

}

/* Series code 513** */
bool db_ctrl::writeTable_shigyo_result(
	shigyo_result_t* result,
	error_t* err
)
{
	int timeout = 0;
	bool rts;
	char cmd[256];
	memset( &cmd[0] , 0x00 , sizeof(cmd) );

	char tmp[48];
	strcpy( cmd, "insert into jcs_j.shigyo_result values ( NULL" );
	strcat( cmd, "," );
	MAKE_CMD_INT( result->kensaki_id);  strcat( cmd, tmp );
	strcat( cmd, "," );
	strcat( cmd, "'" );
	MAKE_CMD_STR( result->kensa_date ); strcat( cmd, tmp ); 
		strcat( cmd, "'" );
	strcat( cmd, "," );
	MAKE_CMD_INT( result->total_judge ); strcat( cmd, tmp ); strcat( cmd, "," );
	strcat( cmd, "'" );
	MAKE_CMD_STR( result->ip ); strcat( cmd, tmp ); 
	strcat( cmd, "'" );
	strcat( cmd, "," );
	strcat( cmd, "'" );
	printf("%.2f\n", result->temperature);
	MAKE_CMD_FSTR( result->temperature ); strcat( cmd, tmp );
	strcat( cmd, "'" );
	strcat( cmd, "," );
	strcat( cmd, "'" );
	printf("%.2f\n", result->humidity);
	MAKE_CMD_FSTR( result->humidity ); strcat( cmd, tmp ); 
	strcat( cmd, "'" );
	strcat( cmd, ")" );
	ADMIN_PRINT( err->flg_admin, cmd, err->ofname );

	while(1)
	{
		if( mysql_query( conn , cmd )) 
		{
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 51301;
				strcpy( err->error_message, "writeTable_shigyo_result timeout");
				ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
				rts = false;
				break;
			}
			timeout++;
		}
		else
		{
			rts = true;
			break;
		}	
	}
	return rts;
}

/* Series code 514** */
bool db_ctrl::writeTable_shigyo_result_detail(
	shigyo_result_detail_t* result,
	error_t* err
)
{
	bool rts;
	int timeout = 0;
	char cmd[256];
	
	memset( &cmd[0] , 0x00 , sizeof(cmd) );
	strcpy( cmd, "select max(shigyo_id) from jcs_j.shigyo_result");	
	
	while(1)
	{
		if( mysql_query( conn, cmd ) )
		{
			debug4("mysql query error ( chkTable_shigyo_result\n");
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 51401;
				strcpy( err->error_message, "writeTable_shigyo_result_detail to1");
				ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
				rts = false;
				break;
			}
			timeout++;
		}
		else
		{
			char tmp[48];	
			resp = mysql_use_result(conn);

			while(( row = mysql_fetch_row(resp)) != NULL )
			{
				result->shigyo_id = atoi(row[0]);
			}
			strcpy( cmd, "insert into jcs_j.shigyo_result_detail values ( " );
			
			MAKE_CMD_INT( result->shigyo_id); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_INT( result->button_no ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_FINT( result->raw_result ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_INT( result->raw_judge ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_FINT( result->baseline_result ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_INT( result->baseline_judge ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_FINT( result->diff_result ); strcat( cmd, tmp );
			strcat( cmd, "," );
			MAKE_CMD_INT( result->diff_judge ); strcat( cmd, tmp );
			strcat( cmd, " )" );
			ADMIN_PRINT( err->flg_admin, cmd, err->ofname );
			
			if( mysql_query( conn , cmd )) 
			{
				delay(1000);
				if ( timeout * 1000 > ERROR_TIMEOUT )
				{
					err->error_code = 51402;
					strcpy( err->error_message, "writeTable_shigyo_result_detail to2");
					ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
					rts = false;
					break;
				}
				timeout++;
			}
			else
			{
				rts = true;
				break;
			}
		}		
	}
	return rts;
}

/* Series code 515** */
bool db_ctrl::writeTable_error( error_t* err )
{

	int timeout = 0;
	bool rts;
	char cmd[256];
	memset( &cmd[0] , 0x00 , sizeof(cmd) );
	char tmp[48];	

	strcpy( cmd, "insert into jcs_j.error values ( " );
	strcat( cmd, "'" );
	MAKE_CMD_STR( err->registered_date ); strcat( cmd, tmp );
	strcat( cmd, "'" );
		
	strcat( cmd, "," );
	MAKE_CMD_INT( err->kensaki_id); strcat( cmd, tmp );
	strcat( cmd, "," );		
	strcat( cmd, "'" );
	MAKE_CMD_STR( err->ip ); strcat( cmd, tmp ); 
	strcat( cmd, "'" );
	strcat( cmd, "," );		
	MAKE_CMD_INT( err->error_code); strcat( cmd, tmp );
	strcat( cmd, "," );
	strcat( cmd, "'" );
	MAKE_CMD_STR( err->error_message ); strcat( cmd, tmp ); 
	strcat( cmd, "'" );
	strcat( cmd, ")" );
	ADMIN_PRINT( err->flg_admin, cmd, err->ofname );
	
	while(1)
	{
		if( mysql_query( conn , cmd )) 
		{
			debug4("mysql query error\n");
			delay(1000);
			if ( timeout * 1000 > ERROR_TIMEOUT )
			{
				err->error_code = 51501;
				strcpy( err->error_message, "writeTable_error timeout");
				ADMIN_PRINT( err->flg_admin, err->error_message, err->ofname );
				rts = false;
				break;
			}
			timeout++;
		}
		else
		{
			rts = true;
			break;
		}
	}
	return rts;
}

/* Series code 516** */
bool db_ctrl::writeTable( )
{
	if( mysql_query( conn , "insert into lot values(100, '2018-12-22', 1, '1')" ) ){
		debug4("mysql query error\n");
		return false;
	}
	else
	{
		return true;
	}	
}

	
#ifdef __DEBUG_Eni_hsi_DEBUGDEBUG_ka_z_amay_DEBUG__
// for debugging
bool db_ctrl::setTable_kensaki(void)
{
	printf("-----------------------------\n");
	printf("%d\n", mysql_query( conn, "insert into jcs_j.kensaki values( 10, '2018-12-28 9:36:45', 'Tachiage-test-00_SX', 12, 19, 5, 10, 56 ,10, 'Shigyo-samp_0A','----')" ));	
	printf("%d\n", mysql_query( conn, "insert into jcs_j.kensaki values( 88, '2018-12-28 9:36:45', 'Tachiage-test-00_SY', 13, 19, 2, 15, 56 ,11, 'Shigyo-samp_00','----')" ));	
	printf("%d\n", mysql_query( conn, "insert into jcs_j.kensaki values( 99, '2018-12-29 9:36:45', 'Tachiage-test-00_SY', 13, 19, 2, 15, 56 ,11, 'Shigyo-samp_00','----')" ));	
	printf("%d\n", mysql_query( conn, "insert into jcs_j.kensaki values( 22, '2018-12-30 9:36:45', 'Tachiage-test-00_SY', 13, 19, 2, 15, 56 ,11, 'Shigyo-samp_00','----')" ));	
	printf("%d\n", mysql_query( conn, "insert into jcs_j.kensaki values( 33, '2018-12-31 9:36:45', 'Tachiage-test-00_SY', 13, 19, 2, 15, 56 ,11, 'Shigyo-samp_00','----')" ));	
	printf("%d\n", mysql_query( conn, "insert into jcs_j.kensaki values( 12, '2018-12-31 9:36:45', 'Tachiage-test-01_SZ', 14, 19, 3, 20, 56 ,12, 'Shigyo-samp_0B','----')" ));	
	return true;
}

	

// for debugging
bool db_ctrl::setTable_lot(error_t* err)
{
	bool rts;
	debug4("set lot table\n");
	
	readQueryData("select now()", &date_time[0], err);
	char date[36];
	memset( &date[0] , 0x00 , sizeof(date) );
	sprintf( date, "%s", &date_time[0]);
	
	char kensaki_id[] = "15";
	
	// Make commend packet (ex: "insert into db_name.table_name values (NULL, data1, data2, ...)" )
	char cmd[256];
	memset( &cmd[0] , 0x00 , sizeof(cmd) );
	strcpy( cmd, "insert into jcs_j.lot");
	strcat( cmd, " values (NULL," ); // AUTO INCREMENT PK should be set to NULL
	strcat( cmd, "'");
	strcat( cmd , date);
	strcat( cmd, "'");
	strcat( cmd, ",");
	strcat( cmd, kensaki_id);
	strcat( cmd, ",");
	strcat( cmd, "20190101001");
	strcat( cmd, ")");
	
	debug4("%s\n", cmd );
	

	rts = mysql_query( conn, &cmd[0] );
	if( rts == 0 )
	{
		rts = true;
	}
	else
	{
		rts = false;
	}
	return rts;
}
	

// for debagging
bool db_ctrl::setTable_shigyo_result(error_t* err)
{
	debug4("set shigyo_result table\n");
	
	// read current date-time from server
	readQueryData("select now()", &date_time[0], err);
	char date[36];
	memset( &date[0] , 0x00 , sizeof(date) );
	sprintf( date, "%s", &date_time[0]);
//	debug4("%s", date);
	
	char kensaki_id[] = "15";
	
	// Make commend packet (ex: "insert into db_name.table_name values (NULL, data1, data2, ...)" )
	char cmd[256];
	memset( &cmd[0] , 0x00 , sizeof(cmd) );
	strcpy( cmd, "insert into jcs_j.shigyo_result");
	strcat( cmd, " values (NULL," ); // AUTO INCREMENT PK should be set to NULL
	strcat( cmd, kensaki_id);
	strcat( cmd, ",'");
	strcat( cmd , date);
	strcat( cmd, "',1,'192.168.11.98', '18', '50')" );
	
	debug4("%s\n", cmd );

	int rts = mysql_query( conn, &cmd[0] );
	if( rts == 0 )
	{
		return true;
	}
	else
	{
		return false;
	}
}



// for debugging
bool db_ctrl::setTable_shigyo_spec(void)
{
	printf("set shigyo_result table\n");
	// 注！既に同じIDで登録されていると書き込みエラーになる。書き込む前に削除すべし
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99,  1, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99,  2, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99,  3, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99,  4, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99,  5, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99,  6, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99,  7, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99,  8, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99,  9, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99, 10, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99, 11, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99, 12, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99, 13, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99, 14, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into shigyo_spec values( '2018-12-28 9:36:45', 99, 15, 7, 3, 7560, 7550, 4, 0)" ));
}
	

bool db_ctrl::setTable_kensa_spec(void)
{
	printf("set shigyo_result table\n");
	// 注！既に同じIDで登録されていると書き込みエラーになる。書き込む前に削除すべし
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99,  1, 'BUTTON_01', 1,  1, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99,  2, 'BUTTON_02', 2,  9, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99,  3, 'BUTTON_03', 1,  2, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99,  4, 'BUTTON_04', 2, 10, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99,  5, 'BUTTON_05', 1,  3, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99,  6, 'BUTTON_06', 2, 11, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99,  7, 'BUTTON_07', 1,  4, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99,  8, 'BUTTON_08', 2, 12, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99,  9, 'BUTTON_09', 1,  5, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99, 10, 'BUTTON_10', 2, 13, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99, 11, 'BUTTON_11', 1,  6, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99, 12, 'BUTTON_12', 2, 14, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99, 13, 'BUTTON_13', 1,  7, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99, 14, 'BUTTON_14', 2, 15, 7, 3, 7560, 7550, 4, 0)" ));
	printf("%d\n", mysql_query( conn, "insert into kensa_spec values( '2018-12-28 9:36:45', 99, 15, 'BUTTON_15', 1,  8, 7, 3, 7560, 7550, 4, 0)" ));
	return true;
}


// for debugging
//#define __MAIN__
#ifdef __MAIN__

int main()
{	
	INFO_DATA 	info;
//	RESULT		rlt;
	kensaki_t	kensaki;
	error_t 	err;

	shigyo_result_t 		shigyo_result;
	shigyo_result_detail_t 	shigyo_result_detail;
	kensa_result_t 			kensa_result;
	kensa_result_detail_t 	kensa_result_detail;

	db_ctrl 	db;
	lot_t 		lot;
	
	db.sql_serv  = "192.168.11.98";
//	db.sql_serv  = "192.168.3.27";
//	db.sql_serv  = "192.168.11.109";
//	db.sql_serv  = "192.168.11.87";
	db.user      = "JCS_J_admin";
	db.passwd    = "passpass";
	db.db_name   = "jcs_j";
	
	db.connect(&err);
	//db.get_cntDateTime();
	//debug4("%s\n", db.date_time);

	strcpy(info.part_name,"Tachiage-test-00_SY");
	strcpy(kensaki.part_name,"Tachiage-test-00_SY");
	kensaki.kensaki_no = 13;
	info.kensaki_no = 13;

//	db.setTable_kensaki();
//	db.setTable_shigyo_result(&err);
//	db.setTable_lot(&err);
	db.setTable_shigyo_spec();
	db.setTable_kensa_spec();
	
	strcpy(err.registered_date, "2019-01-09");
	strcpy(err.ip, "192.168.11.86");
	err.kensaki_id = 88;
	err.error_code = 123456;
	strcpy(err.error_message, "message");
	db.writeTable_error( &err );

//	db.writeTable_shigyo_result(&shigyo_result);
//	db.writeTable_shigyo_result_detail(&shigyo_result_detail);
//	db.writeTable_kensa_result(&kensa_result);
//	db.writeTable_kensa_result_detail(&kensa_result_detail);
	
//	db.readTable_lot( &lot, &info );	
//	db.readTable_kensaki( &kensaki, &info);
	

	
//	db.readTable_kensa_spec();

//	db.writeTable_shigyo_result();
	
	db.close();

	

}

#endif
#endif
/*


#endif


MySQL [jcs_j]> select *from lot;
+--------+---------------------+------------+--------+
| lot_id | registered_date     | kensaki_id | lot_no |
+--------+---------------------+------------+--------+
|      1 | 2018-12-20 00:00:00 |          1 | 1      |
+--------+---------------------+------------+--------+
1 row in set (0.01 sec)

MySQL [jcs_j]> insert into lot values(2,'2018-12-20',1,'1');
Query OK, 1 row affected (0.05 sec)

MySQL [jcs_j]> select *from lot;
+--------+---------------------+------------+--------+
| lot_id | registered_date     | kensaki_id | lot_no |
+--------+---------------------+------------+--------+
|      1 | 2018-12-20 00:00:00 |          1 | 1      |
|      2 | 2018-12-20 00:00:00 |          1 | 1      |
+--------+---------------------+------------+--------+
2 rows in set (0.01 sec)

MySQL [jcs_j]> insert into lot values(2,'2018-12-20',1,'1');

select * from jcs_j.kensaki where kensaki_id=(select max(kensaki_id) from kensaki where part_name='Tachiage-test-00_SY' and kensaki_no=13)
select * from jcs_j.kensaki where kensaki_id=(select max(kensaki_id) from kensaki where part_name='Tachiage-test-00_SY' and kensaki_no=13)
	

*/
