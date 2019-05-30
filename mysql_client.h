#ifndef MYSQL_CLIENT_H
#define MYSQL_CLIENT_H

// SQL table name

#define ID_KENSAKI 				"kensaki"
#define ID_KENSA_SPEC 			"kensa_spec"
#define ID_KENSA_RESULT 		"kensa_result"
#define ID_KENSA_RESULT_DETAIL 	"kensa_result_detail"
#define ID_ID_SHIGYO_SPEC 		"shigyo_spec"
#define ID_SHIGYO_RESULT 		"shigyo_result"
#define ID_SHIGYO_RESULT_DETAIL "shigyo_result_detail"
#define ID_LOT 					"lot"
#define ID_ERROR				"error"

#define SIZE_OF_KENSAKI 				11
#define SIZE_OF_KENSA_SPEC 				12
#define SIZE_OF_KENSA_RESULT 			9
#define SIZE_OF_KENSA_RESULT_DETAIL 	8
#define SIZE_OF_SHIGYO_SPEC 			9
#define SIZE_OF_SHIGYO_RESULT 			7
#define SIZE_OF_SHIGYO_RESULT_DETAIL 	8
#define SIZE_OF_LOT 					4
#define SIZE_OF_ERROR					5

class db_ctrl
{
private:
	MYSQL 			*conn;
	MYSQL_RES 		*resp;
	MYSQL_ROW 		row;


public:
	const char* 	sql_serv;
	const char* 	user;
	const char* 	passwd;
	const char* 	db_name;
	
	char date_time[24];	// 208-12-24 12:00:00
	char date[11];		// 2012-12-24
	char time[9];		// 12:00:00
	
	db_ctrl();
	~db_ctrl();
	
	bool init(error_t*);
	bool connect(error_t*);
	bool close();
	int queryCtrl( const char*, error_t* );

	bool readTable( const char* );
	bool readQueryData( const char*, char*, error_t* );
	bool readTable_shigyo_spec(SHIGYO_SPEC_DATA*, KENSA_SPEC_DATA*, int, error_t*);
	bool readTable_kensa_spec(KENSA_SPEC_DATA*, int, error_t*);
	int readTable_kensaki( kensaki_t*, INFO_DATA*, error_t* );
	int readTable_lot( lot_t*, INFO_DATA*, int, error_t* );

	bool writeTable_shigyo_result( shigyo_result_t*, error_t* );
	bool writeTable_shigyo_result_detail( shigyo_result_detail_t*, error_t* );
	bool writeTable_kensa_result( kensa_result_t*, error_t* );
	bool writeTable_kensa_result_detail( kensa_result_detail_t*, error_t* );
	bool writeTable_error( error_t* );
	
	
	bool writeTable( void );
	

	bool get_cntDateTime(error_t*);
	bool chkTable_shigyo_result( kensa_result_t*, error_t*);

	bool setTable_kensaki(void);
	bool setTable_shigyo_result(error_t*);
	bool setTable_lot(error_t*);
	bool setTable_shigyo_spec(void);
	bool setTable_kensa_spec(void);
	
	
	

};

#endif
