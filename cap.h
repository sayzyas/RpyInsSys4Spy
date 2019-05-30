#ifndef CAP_H
#define CAP_H

#define CAP_RESET_CONDITION() do { \
	gpio_output(CAP_INFO, OFF); \
	usleep(1000); \
	gpio_output(CAP_RESET, ON); \
	usleep(1000); \
} while(0)

#define CAP_READ_INFO_CONDITION() do { \
	gpio_output(CAP_RESET, OFF); \
	usleep(1000); \
	gpio_output(CAP_INFO, ON); \
	usleep(1000); \
} while(0)

#define CAP_READ_DATA_CONDITION() do { \
	gpio_output(CAP_RESET, OFF); \
	usleep(1000); \
	gpio_output(CAP_INFO, OFF); \
	usleep(1000); \
} while(0)

unsigned short b2s( unsigned char upper, unsigned char lower );
bool read_capinfo(int fd, INFO_DATA* info, int rd_size, error_t* err );
bool read_capdata(int cap_no, int fd, float* raw_data, int* max_data, int* min_data, int rd_size, int sum_cnt, error_t* err, bool finger, int* end_status, int* id );


#endif
