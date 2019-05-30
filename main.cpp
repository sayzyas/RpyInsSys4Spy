
/* Series code:   1000 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <mysql/mysql.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPiI2C.h>

#include "common.h"
#include "mysql_client.h"
#include "inspect.h"
#include "gpio.h"
#include "sw.h"
#include "cap.h"
#include "main.h"
#include "debug.h"
#include "net.h"

//#define __AUTO__

bool flg_stop = false;
bool flg_shutdown = false;
void* thread_Stop(void*);
void* thread_Shutdown(void*);

void* thread_Stop(void* pParam)
{
	int 	timeout = 0;
	
	while(1)
	{
		while( !gpio_input(STOP) )
		{
			delay(10);
			if ( gpio_input(STOP) )
			{
				flg_stop = false;
				break;
			}
			else if ( timeout > 35 )
			{
				while(1)
				{
					if( gpio_input(SHUTDOWN) )
					{
						debug2("Thread: STOP button detected.\n");
						flg_stop = true;
						break;
					}
					if( flg_stop == true ) break;
				}
				if( flg_stop == true ) break;
			}
			timeout++;
		}
		delay( 500 );
	}
}

void* thread_Shutdown(void* pParam)
{
	int 	timeout = 0;
	
	while(1)
	{
		while( !gpio_input(SHUTDOWN) )
		{
			delay(10);
			if ( gpio_input(SHUTDOWN) )
			{
				flg_shutdown = false;
				break;
			}
			else if ( timeout > 35 )
			{
				while(1)
				{
					if( gpio_input(SHUTDOWN) )
					{
						debug2("Thread: SHUTDOWN button detected.\n");
						flg_shutdown = true;
						break;
					}
					if( flg_shutdown == true ) break;
				}
				if( flg_shutdown == true ) break;
			}
			timeout++;
		}
		delay( 500 );
	}
}

void f_usage(void)
{
	printf("Usage:\n");
	printf(" ~/test [option] [outfile name");
	printf("\n");
	printf("  option:\n");
	printf("      [f] service stop and then read data until capid = 19.\n");
	printf("      [p] service non stop and then read data until capid = 19.\n");
	printf("      [s] service stop and then read data until capid = 14.\n");
	printf("      [e] service non stop and then read data until capid = 14.\n");	
}

int main( int argc, char *argv[] )
{
	
	int sw_status;
	char buf[64];
	char exefname[16];

	mode md;
	gpio_init();

	//    0                   1        2
	// ./"execute file name" "option" "output file name"
	//
	
	md.err.flg_stpsvc = false;
	md.err.capid_eno = 14;

	if( argc == 1 ) // no option is auto mode
	{
		strcpy( exefname, argv[0] );
		printf( "%s\n", exefname );
		if( strcmp( exefname, "/home/pi/program/test" ) == 0 )
	//	if( strcmp( exefname, "./test" ) == 0 )
		{
			md.err.flg_admin = false; // normal
		}
		else
		{
			printf("Error: No command line option\n");
			printf("Can't execute program\n");	
			printf("Specify command line option and execute again\n");	
			md.err.flg_admin = true;
			md.err.flg_stpsvc = false;
			md.err.capid_eno = 0;
		}
	}	
	else if( argc == 2 ) // error
	{
		char *p = argv[1];
		if( *p == '?' )
		{
			f_usage();
		}
		else
		{
			printf("Error: Invalid command line option\n");
			printf("Can't execute program\n");			
		}
		md.err.flg_admin = true;
		md.err.flg_stpsvc = false;
		md.err.capid_eno = 0;
	}
	else if( argc == 3 ) // done with option is admin mode
	{
		md.err.flg_admin = true;
		strcpy( md.err.ofname, argv[2] );
		ADMIN_PRINT( md.err.flg_admin, "+--- ADMINISTRATOR MODE --------+", md.err.ofname );	
		sprintf( buf, " Program rev: %s", REVISION );
		ADMIN_PRINT( md.err.flg_admin, buf, md.err.ofname );
		sprintf( buf, " Release    : %s", UPDATE );
		ADMIN_PRINT( md.err.flg_admin, buf, md.err.ofname );
		sprintf( buf, " Outfile    : %s", md.err.ofname );
		ADMIN_PRINT( md.err.flg_admin, buf, md.err.ofname );
		ADMIN_PRINT( md.err.flg_admin, " - - - - - - - - - - - - - - - -", md.err.ofname );	
		
		char *p = argv[1];
		if(( *p == 'f' )||( *p == 'F' ))
		{
			// until ID=19 and stop service
			md.err.flg_stpsvc = true;
			md.err.capid_eno = 19;
			ADMIN_PRINT( md.err.flg_admin, " (1) Get until ID=19", md.err.ofname );
			ADMIN_PRINT( md.err.flg_admin, " (2) Service: Stop", md.err.ofname );
			gpio_output(ID19, ON);
		}
		else if(( *p == 'p' )||( *p == 'P' ))
		{
			// until ID=19 and non stop service
			md.err.flg_stpsvc = false;
			md.err.capid_eno = 19;
			ADMIN_PRINT( md.err.flg_admin, " (1) Get until ID=19", md.err.ofname );
			ADMIN_PRINT( md.err.flg_admin, " (2) Service: Non stop", md.err.ofname );
			gpio_output(ID19, ON);
		}
		else if(( *p == 's' )||( *p == 'S' ))
		{
			// until ID=14 and stop service
			md.err.flg_stpsvc = true;
			md.err.capid_eno = 14;
			ADMIN_PRINT( md.err.flg_admin, " (1) Get until ID=14", md.err.ofname );
			ADMIN_PRINT( md.err.flg_admin, " (2) Service: Stop", md.err.ofname );
			gpio_output(ID19, OFF);
		}
		else if(( *p == 'e' )||( *p == 'E' ))
		{
			// until ID=14 and non stop service
			md.err.flg_stpsvc = false;
			md.err.capid_eno = 14;
			ADMIN_PRINT( md.err.flg_admin, " (1) Get until ID=14", md.err.ofname );
			ADMIN_PRINT( md.err.flg_admin, " (2) Service: Non stop", md.err.ofname );
			gpio_output(ID19, OFF);
		}
		else
		{
			printf("Error: Invalid command line option\n");
			printf("Can't execute program\n");
			f_usage();
			md.err.flg_admin = true;
			md.err.flg_stpsvc = false;
			md.err.capid_eno = 0;
		}
		ADMIN_PRINT( md.err.flg_admin, "+-------------------------------+", md.err.ofname );
	}
	else // error display only
	{
		md.err.flg_admin = true;
		md.err.flg_stpsvc = false;
		md.err.capid_eno = 0;
		printf("Error: Invalid command line option or number of arguments\n");
		printf("Can't execute program\n");
		f_usage();
	}

	
	if( md.err.capid_eno > 0 )
	{
		if( md.err.flg_stpsvc == true )
		{
			ADMIN_PRINT( md.err.flg_admin, "** Service program stop **", md.err.ofname );
			system( "sudo systemctl stop program" );
		}
		delay( 1500 );	
		
		
		pthread_t tid1;
		pthread_t tid2;	
		if( md.err.flg_admin == false )
		{
			pthread_create(&tid1, NULL, thread_Stop, NULL);
			pthread_create(&tid2, NULL, thread_Shutdown, NULL);
		}
		
		while (1)
		{
			if( flg_shutdown == true)
			{
				flg_shutdown = false;
				system( "sudo shutdown -h now");
			}
			CAP_RESET_CONDITION();

			ADMIN_PRINT( md.err.flg_admin, "Waiting START button ... ", md.err.ofname );

	#ifndef __AUTO__
			while(1)
			{
				sw_status = main_sw_ctrl();
				if( flg_shutdown == true)
				{
					flg_shutdown = false;
					system( "sudo shutdown -h now");
				}
				if(( sw_status == START_DETECT ) || (sw_status == SHIGYO_DETECT ))
				{
					gpio_output( ERROR, OFF );
					gpio_output( JudgeOK, OFF);
					gpio_output( JudgeNG, OFF);
					break;
				}
				//debug0("wait...\n");
				delay(300);
			}
			gpio_output( RUNNING, ON );

			if( sw_status == SHIGYO_DETECT )
			{
				ADMIN_PRINT( md.err.flg_admin, "SHIGYO start", md.err.ofname );
				md.inspect( OPEMODE_SHIGYO );
			}
			else if( sw_status == START_DETECT )
			{
				ADMIN_PRINT( md.err.flg_admin, "KENSA start", md.err.ofname );
				md.inspect( OPEMODE_INSPECT );
				
			}
	#else
			md.inspect( OPEMODE_INSPECT );
	#endif	
			delay(300);
			
			gpio_output( RUNNING, OFF );
			gpio_output( CONTINUE, OFF );
			gpio_output( ID19, OFF );
			flg_stop = false;
			ADMIN_PRINT( md.err.flg_admin, "+-------+", md.err.ofname );
			ADMIN_PRINT( md.err.flg_admin, "   END   ", md.err.ofname );
			ADMIN_PRINT( md.err.flg_admin, "+-------+", md.err.ofname );
			
			if( md.err.flg_admin == true )
			{
				break;
			}
		}
		
		if(( md.err.flg_admin == true )&&( md.err.flg_stpsvc == true ))
		{
			ADMIN_PRINT( md.err.flg_admin, "** Service program start **", md.err.ofname );
			debug_menu( "\n");
			system( "sudo systemctl start program" );
			delay( 500 );
		}

		if( md.err.flg_admin == false )
		{
			pthread_join(tid1,NULL);
			pthread_join(tid2,NULL);
		}		
	}
    return(0);
}

//
// -- class --------------------------------------------------------------
//

mode::mode()
{
	db.sql_serv  = SQL_SERVER_IP;
	db.user      = SQL_USER_NAME;
	db.passwd    = SQL_PASSWD;
	db.db_name   = SQL_DB_NAME;	
}

bool mode::inspect( int opemode )
{
	char ip[15];
	bool total_judge_A = false;
	bool total_judge_B = false;
	
	char buf[64];
	
	strcpy( err.registered_date, "9999.99.99 00:00:00" );
	err.kensaki_id = 99999;
	strcpy( err.ip, "255.2550.255.0");

	// MySQL database check before start inspection main
	sprintf( ip, "%s", GetMyIpAddr( NETWORK ));
	strcpy( shigyo_result.ip, ip);
	strcpy( kensa_result.ip, ip);
	strcpy( err.ip, ip );
	sprintf( buf, "IP address : %s ", ip );
	ADMIN_PRINT( err.flg_admin, buf, err.ofname );	

	bool rt_bi;
	rt_bi = db.connect(&err);
	if( rt_bi ==  true )
	{
		rt_bi = db.get_cntDateTime(&err);
		strcpy( shigyo_result.kensa_date, db.date_time );
		strcpy( kensa_result.kensa_date, db.date_time );
		strcpy( err.registered_date, db.date_time );		
		db.close();
	}	
	if( rt_bi ==  true )
	{
		// i2c init and read info from CAP
		bool rt_ii = inspect_info( &info, &err );
		if( rt_ii == true )
		{
			for( int capn = 0; capn < info.kiban_no; capn++ )
			{
				info.cap_fd[capn] = wiringPiI2CSetup( 0x10 + capn );
			}			
			// Get current date and time from MySQL server
			bool rt_gt;
			rt_gt = db.connect(&err);
			if( rt_gt ==  true )
			{
				rt_gt = db.get_cntDateTime(&err);
				db.close();
			}
			if( rt_gt == true )
			{
				delay(1500);
				
				// Read kensaki table and serch max kensaki id data
				bool rt_rk;
				rt_rk = db.connect(&err);
				if( rt_rk == true )
				{
					rt_rk = db.readTable_kensaki( &kensaki, &info, &err );
					db.close();
				}
				if( rt_rk == true )
				{
					shigyo_result.kensaki_id = kensaki.kensaki_id;
					kensa_result.kensaki_id = kensaki.kensaki_id;
					err.kensaki_id =  kensaki.kensaki_id;
					
					sprintf( ip, "%s", GetMyIpAddr( NETWORK ));
					strcpy( shigyo_result.ip, ip);
					strcpy( kensa_result.ip, ip);
					strcpy( err.ip, ip );
					sprintf( buf, "IP address : %s ", ip );
					ADMIN_PRINT( err.flg_admin, buf, err.ofname );			

					ADMIN_PRINT( err.flg_admin, "Compare check : KENSAKI table and CAPinfo", err.ofname );
					ADMIN_PRINT( err.flg_admin, "-----------------------------------------", err.ofname );
					ADMIN_PRINT( err.flg_admin, "              KENSAKI : CAPinfo", err.ofname );
					sprintf( buf, "part_name          %s   : %s", kensaki.part_name, info.part_name );
					ADMIN_PRINT( err.flg_admin, buf, err.ofname );
					sprintf( buf, "part_name_length   %02d : %02d", kensaki.part_name_length, info.part_name_length);
					ADMIN_PRINT( err.flg_admin, buf, err.ofname );
					sprintf( buf, "kensaki_no         %02d : %02d", kensaki.kensaki_no, info.kensaki_no);
					ADMIN_PRINT( err.flg_admin, buf, err.ofname );
					sprintf( buf, "kiban_no           %02d : %02d", kensaki.kiban_no, info.kiban_no);
					ADMIN_PRINT( err.flg_admin, buf, err.ofname );
					sprintf( buf, "kensa_kasho        %02d : %02d", kensaki.kensa_kasho, info.kensa_kasho);
					ADMIN_PRINT( err.flg_admin, buf, err.ofname );
					sprintf( buf, "data_packet_length %02d : %02d", kensaki.data_packet_length, info.data_packet_length);
					ADMIN_PRINT( err.flg_admin, buf, err.ofname );
					sprintf( buf, "summary_cnt        %02d : %02d", kensaki.summary_cnt, info.summary_cnt);
					ADMIN_PRINT( err.flg_admin, buf, err.ofname );
					ADMIN_PRINT( err.flg_admin, "-----------------------------------------", err.ofname );
					// Compate db data and info data
					if( 
						( kensaki.part_name_length   == info.part_name_length   ) &&
						( kensaki.kiban_no           == info.kiban_no           ) &&
						( kensaki.kensa_kasho        == info.kensa_kasho        ) &&
						( kensaki.data_packet_length == info.data_packet_length ) &&
						( kensaki.summary_cnt        == info.summary_cnt        ) 
					){
						ADMIN_PRINT( err.flg_admin, "Compare OK", err.ofname );
						
						bool rt_sr = false;
						if( opemode == OPEMODE_SHIGYO )
						{
							rt_sr = true;
						}
						else if( opemode == OPEMODE_INSPECT )
						{
							rt_sr = db.connect(&err);
							if( rt_sr == true )
							{
								ADMIN_PRINT( err.flg_admin, "Check SHIGYO result table", err.ofname );
								rt_sr = db.chkTable_shigyo_result(&kensa_result, &err);
								db.close();
							}
						}
						if((  rt_sr == true )&&( flg_stop == false ))
						{
							ADMIN_PRINT( err.flg_admin, "SHIGYO result OK", err.ofname );
							bool rt_ks = false;
							rt_ks = db.connect(&err);
							if( rt_ks == true )
							{
								ADMIN_PRINT( err.flg_admin, "KENSA spec table read", err.ofname );
								rt_ks = db.readTable_kensa_spec( &kensa_spec_data, kensaki.kensaki_id, &err );
								if( opemode == OPEMODE_SHIGYO )
								{	
									ADMIN_PRINT( err.flg_admin, "KENSA spec table read", err.ofname );
									rt_ks = db.readTable_shigyo_spec( &shigyo_spec_data, &kensa_spec_data, kensaki.kensaki_id, &err );
									db.close();
								}
							}
							if(( rt_ks == true )&&( flg_stop == false ))
							{
								ADMIN_PRINT( err.flg_admin, "LOT table read", err.ofname );
								bool rt_lt;
								rt_lt = db.connect(&err);
								if( rt_lt == true )
								{
									rt_lt = db.readTable_lot( &lot, &info, kensaki.kensaki_id, &err );
									db.close();
								}
								if(( rt_lt == true )&&( flg_stop == false ))
								{
									kensa_result.lot_id = lot.lot_id;
									// ---------------------------------
									// Inspection start ( no finger )
									// ---------------------------------
									bool rt_im1 = inspect_main( &info, false, &err );
									if(( rt_im1 == true )&&( flg_stop == false ))
									{
										ADMIN_PRINT( err.flg_admin, "JUDGEMENT --no finger-- start", err.ofname );
										if( opemode == OPEMODE_SHIGYO )
										{
											total_judge_A = shigyo_judgeA( &info, &shigyo_spec_data, false, &err );
										}
										else if( opemode == OPEMODE_INSPECT )
										{						
											total_judge_A = inspect_judgeA( &info, &kensa_spec_data, false, &err );
										}
										
										gpio_output( CONTINUE, ON );
										
										ADMIN_PRINT( err.flg_admin, "ReStart button waiting", err.ofname );
										if( sw_on_check( RSTART ) == true )
										{
											gpio_output( CONTINUE, OFF );
											
											float tmp, hum;
											read_temphum( &tmp, &hum );
											sprintf( buf, "Temperature %.2f / Humidity %.2f", tmp, hum );
											ADMIN_PRINT( err.flg_admin, buf, err.ofname );

											kensa_result.temperature = tmp;
											kensa_result.humidity = hum;
											shigyo_result.temperature = tmp;
											shigyo_result.humidity = hum;
											
											CAP_RESET_CONDITION();
											CAP_RESET_CONDITION();
											CAP_RESET_CONDITION();
											// ---------------------------------
											// Inspection start ( finger on )
											// ---------------------------------
											bool rt_im2 = inspect_main( &info, true, &err );
											if(( rt_im2 == true )&&( flg_stop == false ))
											{
												// Judgement mesurement data
												if( opemode == OPEMODE_SHIGYO )
												{
													total_judge_B = shigyo_judgeB( &info, &shigyo_spec_data, false, &err);
													if(( total_judge_A == true )&&( total_judge_B == true ))
													{
														shigyo_result.total_judge = JUDGE_OK;
														gpio_output(JudgeOK, ON);
														sprintf( buf, "SHIGYO total judge OK [%d]", shigyo_result.total_judge );
														ADMIN_PRINT( err.flg_admin, buf, err.ofname );
													}
													else
													{
														shigyo_result.total_judge = info.ng_button_cnt;
														gpio_output(JudgeNG, ON);
														sprintf( buf, "SHIGYO total judge NG [%d]", shigyo_result.total_judge );
														ADMIN_PRINT( err.flg_admin, buf, err.ofname );
													}
												}
												else if( opemode == OPEMODE_INSPECT )
												{	
													total_judge_B = inspect_judgeB( &info, &kensa_spec_data, false, &err);
													if(( total_judge_A == true )&&( total_judge_B == true ))
													{
														kensa_result.total_judge = JUDGE_OK;
														gpio_output(JudgeOK, ON);
														sprintf( buf, "KENSA total judge OK [NG=%d]", kensa_result.total_judge );
														ADMIN_PRINT( err.flg_admin, buf, err.ofname );
													}
													else
													{
														kensa_result.total_judge = info.ng_button_cnt;
														gpio_output(JudgeNG, ON);
														sprintf( buf, "KENSA total judge NG [NG=%d]", kensa_result.total_judge );
														ADMIN_PRINT( err.flg_admin, buf, err.ofname );
													}
												}
												bool rt_sr = false;
												rt_sr = db.connect(&err);
												if( rt_sr == true )
												{
													if( opemode == OPEMODE_SHIGYO )
													{
														rt_sr = db.writeTable_shigyo_result( &shigyo_result, &err );
													}
													else if( opemode == OPEMODE_INSPECT )
													{
														rt_sr = db.writeTable_kensa_result( &kensa_result, &err );
													}
													db.close();
												}
												if( rt_sr == true )
												{
													for( int bno = 0; bno < info.kensa_kasho; bno++ )
													{
														if( opemode == OPEMODE_SHIGYO )
														{	
															shigyo_result_detail.raw_result = info.rawpeakpeak[bno];
															shigyo_result_detail.baseline_result = info.baseline[bno];
															shigyo_result_detail.diff_result = info.diff[bno];
															shigyo_result_detail.button_no = bno+1;
															shigyo_result_detail.baseline_judge = info.baseline_judge[bno];
															shigyo_result_detail.raw_judge = info.rawpeakpeak_judge[bno];
															shigyo_result_detail.diff_judge = info.diff_judge[bno];
															db.connect(&err);
															db.writeTable_shigyo_result_detail( &shigyo_result_detail, &err );
															db.close();
														}
														else
														{
															kensa_result_detail.raw_result = info.rawpeakpeak[bno];
															kensa_result_detail.baseline_result = info.baseline[bno];
															kensa_result_detail.diff_result = info.diff[bno];
															kensa_result_detail.button_no = bno+1;
															kensa_result_detail.baseline_judge = info.baseline_judge[bno];
															kensa_result_detail.raw_judge = info.rawpeakpeak_judge[bno];
															kensa_result_detail.diff_judge = info.diff_judge[bno];
															db.connect(&err);
															db.writeTable_kensa_result_detail( &kensa_result_detail, &err );
															db.close();
														}
													}
												}
												//debug0("Program finished\n");
												gpio_output( RUNNING, OFF );
											}
											else
											{
												ADMIN_PRINT( err.flg_admin, "ERROR: inspect_main finger on", err.ofname );
											//	strcpy( err.error_message, "ERROR : inspect_main finger on" );
												db.connect(&err);
												db.writeTable_error(&err);
												db.close();
												gpio_output( ERROR, ON );
											}
										}
										else
										{
											ADMIN_PRINT( err.flg_admin, "ERROR: RESTART sw input timeout", err.ofname );
											strcpy( err.error_message, "ERROR: RESTART sw input timeout");
											db.connect(&err);
											db.writeTable_error(&err);
											db.close();
											gpio_output( ERROR, ON );
										}
									}
									else
									{
										ADMIN_PRINT( err.flg_admin, "ERROR: inspect_main finger off", err.ofname );
										strcpy( err.error_message, "ERROR : inspect_main finger off" );
										db.connect(&err);
										db.writeTable_error(&err);
										db.close();
										gpio_output( ERROR, ON );
									}
								}
								else
								{
									ADMIN_PRINT( err.flg_admin, "ERROR: readTable_lot", err.ofname );
									strcpy( err.error_message, "ERROR : readTable_lot" );
									db.connect(&err);
									db.writeTable_error(&err);
									db.close();
									gpio_output( ERROR, ON );
								}
}
							else
							{
								ADMIN_PRINT( err.flg_admin, "ERROR: KENSA spec table read", err.ofname );
								strcpy( err.error_message, "ERROR : KENSA spec table read" );
								db.connect(&err);
								db.writeTable_error(&err);
								db.close();
								gpio_output( ERROR, ON );
							}
						}
						else
						{
							err.error_code = 00001;
							strcpy( err.error_message, "ERROR : Shigyo date or STOP detected" );
							db.connect(&err);
							db.writeTable_error(&err);
							db.close();
							ADMIN_PRINT( err.flg_admin, "ERROR : Shigyo date or STOP detected", err.ofname );
							gpio_output( ERROR, ON );
						}
					}
					else
					{
						err.error_code = 00004;
						strcpy( err.error_message, "ERROR : read inspect info" );
						db.connect(&err);
						db.writeTable_error(&err);
						db.close();
						ADMIN_PRINT( err.flg_admin, "ERROR : read inspect info", err.ofname );
						gpio_output( ERROR, ON );
					}		
				}
				else
				{
					err.error_code = 00003;
					strcpy( err.error_message, "ERROR : read kensaki table" );
					db.connect(&err);
					db.writeTable_error(&err);
					db.close();
					ADMIN_PRINT( err.flg_admin, "ERROR : read kensaki table", err.ofname );
					gpio_output( ERROR, ON );
				}		
				
				
			}
			else
			{
				err.error_code = 00002;
				strcpy( err.error_message, "ERROR : read datetime read from db" );
				db.connect(&err);
				db.writeTable_error(&err);
				db.close();
				ADMIN_PRINT( err.flg_admin, "ERROR : read datetime read from db", err.ofname );
				gpio_output( ERROR, ON );	
			}
		}
		else
		{
			err.error_code = 00001;
			strcpy( err.error_message, "ERROR : Info read" );
			db.connect(&err);
			db.writeTable_error(&err);
			db.close();
			ADMIN_PRINT( err.flg_admin, "ERROR : Info read", err.ofname );
			gpio_output( ERROR, ON );	
		}
	}
	else
	{
		err.error_code = 99999;
		strcpy( err.error_message, "ERROR : get data time from db" );
		ADMIN_PRINT( err.flg_admin, "ERROR : get data time from db", err.ofname );
		gpio_output( ERROR, ON );	
	}
	
	delay(300);
	return true;
}


