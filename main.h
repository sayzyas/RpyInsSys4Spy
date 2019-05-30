#ifndef MAIN_H
#define MAIN_H

class mode
{
private:
	INFO_DATA 	info;
	RESULT		rlt;

	kensaki_t				kensaki;
	lot_t					lot;
	shigyo_result_t			shigyo_result;
	shigyo_result_detail_t	shigyo_result_detail;
	kensa_result_t			kensa_result;
	kensa_result_detail_t	kensa_result_detail;
	
	db_ctrl 				db;
	SHIGYO_SPEC_DATA   		shigyo_spec_data;
	KENSA_SPEC_DATA   		kensa_spec_data;
	
	//bool					flg_stop;

public:
	error_t					err;

	mode();
	bool inspect(int);

};



#endif
