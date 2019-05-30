#pragma once

/////////////////////////////////////////////////////////////////////////////
// 
#define REVISION "1.13"
#define UPDATE "2019.05.07"
// 
//
//
 

#define CLEAR_SCREEN		puts("\x1b[2J")		// cleae screen
#define MOVE_CURSOR_TO_TOP	puts("\x1b[0;0H")	// move cursol to top

// ####################################
// For debugging : if defined, program done until end even if error occured.
// #define __DEBUGPASS__
// ####################################

//#define NETWORK	"wlan0"
#define NETWORK	"eth0"

/* SQL server */
#define SQL_SERVER_IP	"192.168.11.101"
//#define SQL_SERVER_IP	"192.168.3.18"

#define SQL_USER_NAME	"JCS_J_admin"
#define SQL_PASSWD		"passpass"
#define SQL_DB_NAME		"jcs_j"

#define OPEMODE_SHIGYO 0
#define OPEMODE_INSPECT 1
#define OPEMODE_MAINMTE 2

#define	JUDGE_OK		0
#define	JUDGE_NG		1

#define SIZE_OF_CAPINFO	56
#define SIZE_OF_CAPDATA	56

#define ON        0		// because Active Low
#define OFF       1		// because Active Low
#define LOW       0		// because Active Low
#define HIGH      1		// because Active Low

#define NO_DETECT			0x80
#define START_DETECT 		0x81	// 1000 0001
#define SHIGYO_DETECT		0x82	// 1000 0010
#define STOP_DETECT			0x82	// 1000 0011
#define SHUTDOWN_DETECT 	0x84	// 1000 0100


#define CAP_ENABLE() gpio_output(CAP_RESET, OFF);	// PSoC4 RESET pin to High
#define CAP_DISABLE() gpio_output(CAP_RESET, ON);	// PSoC4 RESET pin to Low

#define NUMBER_OF_DATA 27

#define SIZE_OF_PARTNAME 40

#define MYSQL_TIMEOUT	2.5		// sec
#define ERROR_TIMEOUT	5000	// msec

typedef struct 
{
	char part_name[SIZE_OF_PARTNAME];	// Product name
	char part_name_length;				// sizw of product name
	char kensaki_no;					// number of kensaki
	char kiban_no;						// number of CAP board
	char kensa_kasho;					// point of inspection place
	char data_packet_length;			// Packet size of innfo data packet
	char summary_cnt;					// Data Sampling count
	
//	int raw_data[24][27];				// Raw Data (11) with no finger
//	int max_data[24][27];				// Max Data (13) with no finger
//	int min_data[24][27];				// Min Data (14) with no finger

	// Measerement result data buffer
	// data[ number of CAP board ][ number of Raw data = 27 ]
	float raw_data_nf[15][27];	// Raw Data (11) with no finger
	int max_data_nf[15][27];	// Max Data (13) with no finger
	int min_data_nf[15][27];	// Min Data (14) with no finger
	float raw_data_of[15][27];	// Raw Data (11) with finger on
	int max_data_of[15][27];	// Max Data (13) with finger on
	int min_data_of[15][27];	// Min Data (14) with finger on

	// use for judgement data and store this data to database(result_drtail)
	float baseline[1024];			// average of raw data
	int rawpeakpeak[1024];		// max - min
	float diff[1024];				// fiinger on average data - finger off average data
		
	int baseline_judge[1024];	// judgement data OK: 0, NG: NG button count
	int rawpeakpeak_judge[1024];	// judgement data OK: 0, NG: NG button count 
	int diff_judge[1024];		// judgement data OK: 0, NG: NG button count
	
	int baseline_judge_count;
	int rawpeakpeak_judge_count;
	int diff_judge_count;
	
	int ng_button_cnt;			// NG Button ncount
	
	int cap_fd[24];
	
	
} INFO_DATA;

typedef struct
{
	int  inspect_index[254][2];
	char registered_data[254][21];
	int  kensaki_id[254];
	int  button_no[254];
	int  packet_position[254];
	int  raw_peak_peak_upper[254];
	int  raw_peak_peak_lower[254];
	int  baseline_upper[254];
	int  baseline_lower[254];
	int  diff_upper[254];
	int  diff_lower[254];
	
} SHIGYO_SPEC_DATA;

typedef struct
{
	int  inspect_index[254][2];
	char registered_data[254][21];
	int  kensaki_id[254];
	int  button_no[254];
	char button_name[254][16];
	int  kiban_no[254];
	int  packet_position[254];
	int  raw_peak_peak_upper[254];
	int  raw_peak_peak_lower[254];
	int  baseline_upper[254];
	int  baseline_lower[254];
	int  diff_upper[254];
	int  diff_lower[254];
	
} KENSA_SPEC_DATA;



typedef struct
{
	int max_min[2][27];			// variation of data
	int fon_foff[2][27];		// finger on - finger off
	
} RESULT;


// KENSAKI テーブル
// 品名とか検査機番号などの情報を保存する。
typedef struct {
	int   kensaki_id;				// INT NOT NULL AUTO_INCREMENT,	/* PK ／ AUTO_INCREMENT */
	char  registered_date[22];		// DATETIME NOT NULL,			/* このレコードを登録した日時 */
	char  part_name[40];			// CHAR(40) NOT NULL,			/* 品名 */
	int   kensaki_no;				// TINYINT NOT NULL,			/* 検査機番号 */
	int   part_name_length;			// TINYINT NOT NULL,			/* 品名の文字数 */
	int   kiban_no;					// TINYINT NOT NULL,			/* CAP基板の枚数 */
	int   kensa_kasho;				// TINYINT NOT NULL,			/* 検査個所数 */
	int   data_packet_length;		// TINYINT NOT NULL,			/* データパケットのバイト数 */
	int   summary_cnt;				// TINYINT NOT NULL,			/* 集計するスキャン数 */
	char  shigyo_master_name[16];	// CHAR(16) NOT NULL,			/* 始業点検に使うサンプルの名前。始業点検サンプルの変更も、このテーブルに新たなレコードを追加して、つまり、新たな kensaki_id を登録する。 */
	char  comment[40];				// CHAR(40) NOT NULL,			/* ちょっとだけ違った検査条件を登録するための予備的なフィールド。規格変更もこのテーブルに新たなレコードを追加して、つまり、新たな kensaki_id を登録する。 */
} kensaki_t;

// KENSA_SPEC テーブル
// 検査箇所ごとに、データの所在や、検査規格を保管する。
typedef struct {
	char registered_date[22];	// DATETIME NOT NULL,				/* このレコードを登録した日時 */
	int kensaki_id;				// INT NOT NULL,					/* PK。 */
	int button_no;				// TINYINT NOT NULL,				/* PK。検査箇所を示す番号。１～２５５の整数。 */
	char button_name[16];		// CHAR(16) NOT NULL,				/* 検査箇所の名前。"A" とか、"STOP-KEY" とか。 */
	int kiban_no;				// TINYINT NOT NULL,				/* この検査箇所のデータが、どのCAP基板からのデータに含まれているか */
	int packet_position;		// TINYINT NOT NULL,				/* この検査箇所のデータが、データパケットのどこにあるか */
	int raw_peak_peak_upper;	// INT NOT NULL,					/* RawPeakPeak の上限 */
	int raw_peak_peak_lower;	// INT NOT NULL,					/* RawPeakPeak の下限 */
	int baseline_upper;			// INT NOT NULL,					/* Baseline の上限 */
	int baseline_lower;			// INT NOT NULL,					/* Baseline の下限 */
	int diff_upper;				// INT NOT NULL,					/* Diff の上限 */
	int diff_lower;				// INT NOT NULL,					/* Diff の下限 */
} kensa_spec_t;
	
// KENSA_RESULT テーブル
// サンプルを１枚検査する毎に、基本的な情報を保存する。
typedef struct {
	int kensa_id;			//	BIGINT NOT NULL AUTO_INCREMENT,	/* PK, AUTO_INCREMENT ： これだけ BIGINT 型 */
	int kensaki_id;			//	INT NOT NULL,					/* 品名と検査機番号 */
	char kensa_date[22];	//	DATETIME NOT NULL,				/* 検査日時 */
	int lot_id;				//	INT NOT NULL,					/* ロット番号を示す */
	int shigyo_id;			//	INT NOT NULL,					/* 始業の情報 */
	int total_judge;		//	TINYINT NOT NULL,				/* その検査対象サンプルの総合判定結果 */
	char ip[15];			//	CHAR(15) NOT NULL,				/* この検査を実施した時のラズパイのＩＰアドレス */

	float temperature;		//	CHAR(10) NOT NULL,	/* 気温、データ型はとりあえず、文字列 */ データベースは文字列だがここではfloatとする
	float humidity;			//	CHAR(10) NOT NULL,	/* 湿度、データ型はとりあえず、文字列 */ データベースは文字列だがここではfloatとする

//	char temperature[11];	//	CHAR(10) NOT NULL,				/* 気温、データ型はとりあえず、文字列 */
//	char humidity[11];		//	CHAR(10) NOT NULL,				/* 湿度、データ型はとりあえず、文字列 */
} kensa_result_t;

// KENSA_RESULT_DETAIL テーブル
// サンプルを検査した結果の詳細情報を保存する。
typedef struct {
	int   kensa_id;			//	BIGINT NOT NULL,				/* PK, BIGINT 型 */
	int   button_no;		//	TINYINT NOT NULL,				/* PK。検査箇所を示す番号。１～２５５の整数。 */
	float raw_result;		//	FLOAT NOT NULL,					/* RawPeakPeak の値 */
	int   raw_judge;		//	TINYINT NOT NULL,				/* RawPeakPeak の判定 */
	float baseline_result;	//	FLOAT NOT NULL,					/* Baseline の値 */
	int   baseline_judge;	//	TINYINT NOT NULL,				/* Baseline の判定 */
	float diff_result;		//	FLOAT NOT NULL,					/* Diff の値 */
	int   diff_judge;		//	TINYINT NOT NULL,				/* Diff の判定 */
} kensa_result_detail_t;

// SHIGYO_SPEC テーブル
// 検査箇所ごとに、始業点検の際の検査規格を保管する。データの所在は、始業点検でも、KENSA_SPEC テーブルを参照する。
typedef struct {
	char registered_date[22];	//DATETIME NOT NULL,				/* このレコードを登録した日時 */
	int kensaki_id;				//INT NOT NULL,					/* PK。 */
	int button_no;				//TINYINT NOT NULL,				/* PK。検査箇所を示す番号。１～２５５の整数。 */
	int raw_peak_peak_upper;	//INT NOT NULL,					/* RawPeakPeak の上限 */
	int raw_peak_peak_lower;	//INT NOT NULL,					/* RawPeakPeak の下限 */
	int baseline_upper;			//INT NOT NULL,					/* Baseline の上限 */
	int baseline_lower;			//INT NOT NULL,					/* Baseline の下限 */
	int diff_upper;				//INT NOT NULL,					/* Diff の上限 */
	int diff_lower;				//INT NOT NULL,					/* Diff の下限 */
} shigyo_spec_t;

// SHIGYO_RESULT テーブル
// サンプルを１枚始業点検する毎に、基本的な情報を保存する。
typedef struct {
	int shigyo_id;			//	INT NOT NULL AUTO_INCREMENT,	/* PK ／ AUTO_INCREMENT */
	int kensaki_id;			//	INT NOT NULL,					/* 品名と検査機番号 */
	char kensa_date[22];	//	DATETIME NOT NULL,				/* 検査日時 */
	int total_judge;		//	TINYINT NOT NULL,				/* この始業点検の総合判定結果 */
	char ip[15];			//	CHAR(15) NOT NULL,				/* この始業点検を実施した時のラズパイのＩＰアドレス */

	float temperature;		//	CHAR(10) NOT NULL,	/* 気温、データ型はとりあえず、文字列 */ データベースは文字列だがここではfloatとする
	float humidity;			//	CHAR(10) NOT NULL,	/* 湿度、データ型はとりあえず、文字列 */ データベースは文字列だがここではfloatとする

} shigyo_result_t;

// SHIGYO_RESULT_DETAIL テーブル
// 始業点検した結果の詳細情報を保存する。
typedef   struct {
	int   shigyo_id;		// INT NOT NULL,		/* PK。 */
	int   button_no;		// TINYINT NOT NULL,	/* PK。検査箇所を示す番号。１～２５５の整数。 */
	float raw_result;		// FLOAT NOT NULL,		/* RawPeakPeak の値 */
	int   raw_judge;		// TINYINT NOT NULL,	/* RawPeakPeak の判定 */
	float baseline_result;	// FLOAT NOT NULL,		/* Baseline の値 */
	int   baseline_judge;	// TINYINT NOT NULL,	/* Baseline の判定 */
	float diff_result;		// FLOAT NOT NULL,		/* Diff の値 */
	int   diff_judge;		// TINYINT NOT NULL,	/* Diff の判定 */
} shigyo_result_detail_t;
	
// LOT テーブル
// ロット番号を保存する。
typedef struct {
	int  lot_id;				// INT NOT NULL AUTO_INCREMENT,	/* PK ／ AUTO_INCREMENT */
	char registered_date[22];	// DATETIME NOT NULL,		/* このレコードを登録した日時 */
	int  kensaki_id;			// INT NOT NULL,			/* */
	char lot_no[20];			// CHAR(20) NOT NULL,		/* ロット番号の本体。文字列。 */
} lot_t;

	// ERROR テーブル
//エラーの情報を保存する。
typedef struct {
	char registered_date[22];	// DATETIME NOT NULL,				/* PK ／ エラー判定した日時 */
	int  kensaki_id;			// INT NOT NULL,					/* PK */
	char ip[15];				// CHAR(15) NOT NULL,				/* この検査を実施した時のラズパイのＩＰアドレス */
	int  error_code;			// INT NOT NULL,					/* エラーコード */
	char error_message[40];		// CHAR(40) NOT NULL,				/* エラーを説明する文字列 */
	
	bool flg_admin;				// Administrator mode
	bool flg_stpsvc;			// Auto test service stop
	int  capid_eno;				// Cap data read end ID numbor
	char ofname[16];			// Special, file name of redirection file name
} error_t;

