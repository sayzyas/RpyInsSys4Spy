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


// KENSAKI �e�[�u��
// �i���Ƃ������@�ԍ��Ȃǂ̏���ۑ�����B
typedef struct {
	int   kensaki_id;				// INT NOT NULL AUTO_INCREMENT,	/* PK �^ AUTO_INCREMENT */
	char  registered_date[22];		// DATETIME NOT NULL,			/* ���̃��R�[�h��o�^�������� */
	char  part_name[40];			// CHAR(40) NOT NULL,			/* �i�� */
	int   kensaki_no;				// TINYINT NOT NULL,			/* �����@�ԍ� */
	int   part_name_length;			// TINYINT NOT NULL,			/* �i���̕����� */
	int   kiban_no;					// TINYINT NOT NULL,			/* CAP��̖��� */
	int   kensa_kasho;				// TINYINT NOT NULL,			/* �������� */
	int   data_packet_length;		// TINYINT NOT NULL,			/* �f�[�^�p�P�b�g�̃o�C�g�� */
	int   summary_cnt;				// TINYINT NOT NULL,			/* �W�v����X�L������ */
	char  shigyo_master_name[16];	// CHAR(16) NOT NULL,			/* �n�Ɠ_���Ɏg���T���v���̖��O�B�n�Ɠ_���T���v���̕ύX���A���̃e�[�u���ɐV���ȃ��R�[�h��ǉ����āA�܂�A�V���� kensaki_id ��o�^����B */
	char  comment[40];				// CHAR(40) NOT NULL,			/* ������Ƃ������������������o�^���邽�߂̗\���I�ȃt�B�[���h�B�K�i�ύX�����̃e�[�u���ɐV���ȃ��R�[�h��ǉ����āA�܂�A�V���� kensaki_id ��o�^����B */
} kensaki_t;

// KENSA_SPEC �e�[�u��
// �����ӏ����ƂɁA�f�[�^�̏��݂�A�����K�i��ۊǂ���B
typedef struct {
	char registered_date[22];	// DATETIME NOT NULL,				/* ���̃��R�[�h��o�^�������� */
	int kensaki_id;				// INT NOT NULL,					/* PK�B */
	int button_no;				// TINYINT NOT NULL,				/* PK�B�����ӏ��������ԍ��B�P�`�Q�T�T�̐����B */
	char button_name[16];		// CHAR(16) NOT NULL,				/* �����ӏ��̖��O�B"A" �Ƃ��A"STOP-KEY" �Ƃ��B */
	int kiban_no;				// TINYINT NOT NULL,				/* ���̌����ӏ��̃f�[�^���A�ǂ�CAP�����̃f�[�^�Ɋ܂܂�Ă��邩 */
	int packet_position;		// TINYINT NOT NULL,				/* ���̌����ӏ��̃f�[�^���A�f�[�^�p�P�b�g�̂ǂ��ɂ��邩 */
	int raw_peak_peak_upper;	// INT NOT NULL,					/* RawPeakPeak �̏�� */
	int raw_peak_peak_lower;	// INT NOT NULL,					/* RawPeakPeak �̉��� */
	int baseline_upper;			// INT NOT NULL,					/* Baseline �̏�� */
	int baseline_lower;			// INT NOT NULL,					/* Baseline �̉��� */
	int diff_upper;				// INT NOT NULL,					/* Diff �̏�� */
	int diff_lower;				// INT NOT NULL,					/* Diff �̉��� */
} kensa_spec_t;
	
// KENSA_RESULT �e�[�u��
// �T���v�����P���������閈�ɁA��{�I�ȏ���ۑ�����B
typedef struct {
	int kensa_id;			//	BIGINT NOT NULL AUTO_INCREMENT,	/* PK, AUTO_INCREMENT �F ���ꂾ�� BIGINT �^ */
	int kensaki_id;			//	INT NOT NULL,					/* �i���ƌ����@�ԍ� */
	char kensa_date[22];	//	DATETIME NOT NULL,				/* �������� */
	int lot_id;				//	INT NOT NULL,					/* ���b�g�ԍ������� */
	int shigyo_id;			//	INT NOT NULL,					/* �n�Ƃ̏�� */
	int total_judge;		//	TINYINT NOT NULL,				/* ���̌����ΏۃT���v���̑������茋�� */
	char ip[15];			//	CHAR(15) NOT NULL,				/* ���̌��������{�������̃��Y�p�C�̂h�o�A�h���X */

	float temperature;		//	CHAR(10) NOT NULL,	/* �C���A�f�[�^�^�͂Ƃ肠�����A������ */ �f�[�^�x�[�X�͕����񂾂������ł�float�Ƃ���
	float humidity;			//	CHAR(10) NOT NULL,	/* ���x�A�f�[�^�^�͂Ƃ肠�����A������ */ �f�[�^�x�[�X�͕����񂾂������ł�float�Ƃ���

//	char temperature[11];	//	CHAR(10) NOT NULL,				/* �C���A�f�[�^�^�͂Ƃ肠�����A������ */
//	char humidity[11];		//	CHAR(10) NOT NULL,				/* ���x�A�f�[�^�^�͂Ƃ肠�����A������ */
} kensa_result_t;

// KENSA_RESULT_DETAIL �e�[�u��
// �T���v���������������ʂ̏ڍ׏���ۑ�����B
typedef struct {
	int   kensa_id;			//	BIGINT NOT NULL,				/* PK, BIGINT �^ */
	int   button_no;		//	TINYINT NOT NULL,				/* PK�B�����ӏ��������ԍ��B�P�`�Q�T�T�̐����B */
	float raw_result;		//	FLOAT NOT NULL,					/* RawPeakPeak �̒l */
	int   raw_judge;		//	TINYINT NOT NULL,				/* RawPeakPeak �̔��� */
	float baseline_result;	//	FLOAT NOT NULL,					/* Baseline �̒l */
	int   baseline_judge;	//	TINYINT NOT NULL,				/* Baseline �̔��� */
	float diff_result;		//	FLOAT NOT NULL,					/* Diff �̒l */
	int   diff_judge;		//	TINYINT NOT NULL,				/* Diff �̔��� */
} kensa_result_detail_t;

// SHIGYO_SPEC �e�[�u��
// �����ӏ����ƂɁA�n�Ɠ_���̍ۂ̌����K�i��ۊǂ���B�f�[�^�̏��݂́A�n�Ɠ_���ł��AKENSA_SPEC �e�[�u�����Q�Ƃ���B
typedef struct {
	char registered_date[22];	//DATETIME NOT NULL,				/* ���̃��R�[�h��o�^�������� */
	int kensaki_id;				//INT NOT NULL,					/* PK�B */
	int button_no;				//TINYINT NOT NULL,				/* PK�B�����ӏ��������ԍ��B�P�`�Q�T�T�̐����B */
	int raw_peak_peak_upper;	//INT NOT NULL,					/* RawPeakPeak �̏�� */
	int raw_peak_peak_lower;	//INT NOT NULL,					/* RawPeakPeak �̉��� */
	int baseline_upper;			//INT NOT NULL,					/* Baseline �̏�� */
	int baseline_lower;			//INT NOT NULL,					/* Baseline �̉��� */
	int diff_upper;				//INT NOT NULL,					/* Diff �̏�� */
	int diff_lower;				//INT NOT NULL,					/* Diff �̉��� */
} shigyo_spec_t;

// SHIGYO_RESULT �e�[�u��
// �T���v�����P���n�Ɠ_�����閈�ɁA��{�I�ȏ���ۑ�����B
typedef struct {
	int shigyo_id;			//	INT NOT NULL AUTO_INCREMENT,	/* PK �^ AUTO_INCREMENT */
	int kensaki_id;			//	INT NOT NULL,					/* �i���ƌ����@�ԍ� */
	char kensa_date[22];	//	DATETIME NOT NULL,				/* �������� */
	int total_judge;		//	TINYINT NOT NULL,				/* ���̎n�Ɠ_���̑������茋�� */
	char ip[15];			//	CHAR(15) NOT NULL,				/* ���̎n�Ɠ_�������{�������̃��Y�p�C�̂h�o�A�h���X */

	float temperature;		//	CHAR(10) NOT NULL,	/* �C���A�f�[�^�^�͂Ƃ肠�����A������ */ �f�[�^�x�[�X�͕����񂾂������ł�float�Ƃ���
	float humidity;			//	CHAR(10) NOT NULL,	/* ���x�A�f�[�^�^�͂Ƃ肠�����A������ */ �f�[�^�x�[�X�͕����񂾂������ł�float�Ƃ���

} shigyo_result_t;

// SHIGYO_RESULT_DETAIL �e�[�u��
// �n�Ɠ_���������ʂ̏ڍ׏���ۑ�����B
typedef   struct {
	int   shigyo_id;		// INT NOT NULL,		/* PK�B */
	int   button_no;		// TINYINT NOT NULL,	/* PK�B�����ӏ��������ԍ��B�P�`�Q�T�T�̐����B */
	float raw_result;		// FLOAT NOT NULL,		/* RawPeakPeak �̒l */
	int   raw_judge;		// TINYINT NOT NULL,	/* RawPeakPeak �̔��� */
	float baseline_result;	// FLOAT NOT NULL,		/* Baseline �̒l */
	int   baseline_judge;	// TINYINT NOT NULL,	/* Baseline �̔��� */
	float diff_result;		// FLOAT NOT NULL,		/* Diff �̒l */
	int   diff_judge;		// TINYINT NOT NULL,	/* Diff �̔��� */
} shigyo_result_detail_t;
	
// LOT �e�[�u��
// ���b�g�ԍ���ۑ�����B
typedef struct {
	int  lot_id;				// INT NOT NULL AUTO_INCREMENT,	/* PK �^ AUTO_INCREMENT */
	char registered_date[22];	// DATETIME NOT NULL,		/* ���̃��R�[�h��o�^�������� */
	int  kensaki_id;			// INT NOT NULL,			/* */
	char lot_no[20];			// CHAR(20) NOT NULL,		/* ���b�g�ԍ��̖{�́B������B */
} lot_t;

	// ERROR �e�[�u��
//�G���[�̏���ۑ�����B
typedef struct {
	char registered_date[22];	// DATETIME NOT NULL,				/* PK �^ �G���[���肵������ */
	int  kensaki_id;			// INT NOT NULL,					/* PK */
	char ip[15];				// CHAR(15) NOT NULL,				/* ���̌��������{�������̃��Y�p�C�̂h�o�A�h���X */
	int  error_code;			// INT NOT NULL,					/* �G���[�R�[�h */
	char error_message[40];		// CHAR(40) NOT NULL,				/* �G���[��������镶���� */
	
	bool flg_admin;				// Administrator mode
	bool flg_stpsvc;			// Auto test service stop
	int  capid_eno;				// Cap data read end ID numbor
	char ofname[16];			// Special, file name of redirection file name
} error_t;

