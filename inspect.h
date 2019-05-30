#ifndef INSPECT_H
#define INSPECT_H

//void get_info(int fd, int rd_size);
//void get_cap(int fd, int rd_size);
//unsigned short b2s( char upper, char lower );

bool inspect_info(INFO_DATA*, error_t*);
bool inspect_main(INFO_DATA*, bool, error_t*);

bool inspect_judgeA( INFO_DATA*, KENSA_SPEC_DATA*, bool, error_t*);
bool inspect_judgeB( INFO_DATA*, KENSA_SPEC_DATA*, bool, error_t*);

bool shigyo_judgeA( INFO_DATA*, SHIGYO_SPEC_DATA*, bool, error_t*);
bool shigyo_judgeB( INFO_DATA*, SHIGYO_SPEC_DATA*, bool, error_t*);

#endif
