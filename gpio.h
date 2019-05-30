#ifndef GPIO_H
#define GPIO_H

// RaspberryPie pin assign
/*
 +-----------+-------+----+----+-------+-------+-------+----+----+-------+----------+
 | Assign    |       |BCM |wPi | Name  |   Pi  | Name  |wPi |BCN |       | Assign   |
 +-----------+-------+----+----+-------+-------+-------+----+----+-------+----------+
 | --        |       |    |    |  3.3v | 01 02 |    5v |    |    |       | --       |
 |           | SDA   | 02 | 08 | SDA.1 | 03 04 |    5v |    |    |       | --       |
 |           | SCL   | 03 | 09 | SCL.1 | 05 06 |   GND |    |    | GND   | --       |
 |           | IN_7  | 04 | 07 | IO.07 | 07 08 |   TxD | 15 | 14 | OUT_4 | JUDGE_OK |
 | --        | GND   |    |    |   GND | 09 10 |   RxD | 16 | 15 | IN_4  | STOP     |
 | CAP_INFO  | OUT_7 | 17 | 00 | IO.00 | 11 12 | IO.01 | 01 | 18 | OUT_0 | RUNNING  |
 | ID19      | OUT_5 | 27 | 02 | IO.02 | 13 14 |   GND |    |    | GND   | --       |
 | CAP_RESET | OUT_6 | 22 | 03 | IO.03 | 15 16 | IO.04 | 04 | 23 | IN_0  | START    |
 | --        | 3.3v  |    |    |  3.3v | 17 18 | IO.05 | 05 | 24 | OUT_1 | CONTNUE  |
 | ERROR     | OUT_3 | 10 | 12 |  MOSI | 19 20 |   GND |    |    | GND   | --       |
 |           | LED   | 09 | 13 |  MISO | 21 22 | IO.06 | 06 | 25 | IN_1  | SHIGYO   |
 | SHUTDOWN  | IN_3  | 11 | 14 |  SCLK | 23 24 |   CE0 | 10 | 08 | OUT_2 | JUDGE_NG |
 | --        | GND   |    |    |   GND | 25 26 |   CE1 | 11 | 07 | IN_2  | RE-START |
 | N.C.      |       | 00 | 30 | SDA.0 | 27 28 | SCL.0 | 31 | 01 |       | N.C.     |
 | N.C.      |       | 05 | 21 | IO.21 | 29 30 |   GND |    |    |       | N.C.     |
 | N.C.      |       | 06 | 22 | IO.22 | 31 32 | IO.26 | 26 | 12 |       | N.C.     |
 | N.C.      |       | 13 | 23 | IO.23 | 33 34 |   GND |    |    |       | N.C.     |
 | N.C.      |       | 19 | 24 | IO.24 | 35 36 | IO.27 | 27 | 16 |       | N.C.     |
 | N.C.      |       | 26 | 25 | IO.25 | 37 38 | IO.28 | 28 | 20 |       | N.C.     |
 | N.C.      |       |    |    |   GND | 39 40 | IO.29 | 29 | 21 |       | N.C.     |
 +-----------+-------+----+----+-------+-------+-------+----+----+-------+----------+
 
*/
// IN
#define START 			23		// IN_0: START from PLC
#define SHIGYO 			25		// IN_1: SHIGYO
#define RSTART 			7		// IN_2: RE-START
#define SHUTDOWN 		11		// IN_3: RaspberyPi SHUTDOWN
#define STOP 			15		// IN_4: STOP
// OUT
#define RUNNING 		18		// OUT_0: PROGRAM RUNNING
#define CONTINUE 		24		// OUT_1: INSPECTION continue to PLC
#define JudgeNG			8		// OUT_2: JUDGEMENT NG signal to PLC
#define ERROR 			10		// OUT_3: ERROR signal
#define JudgeOK			14		// OUT_4: JUDGEMENT OK signal to PLC

#define CAP_RESET 		22		// OUT_6: CAP Reset
#define CAP_INFO 		17		// OUT_7: CAP Info

// 2019.02.27 added
#define ID19			27		// OUT: When low, read data until ID=19


#define IIC_ID_CAP0		0x10
#define IIC_ID_TMPHUM	0x45


// Prototype
void gpio_init(void);
int  gpio_input( int32_t pin );
void gpio_output( int32_t pin , int32_t val);
void gpio_cap_reset( void );
void gpio_cap_readInfo( void );
void gpio_cap_readData( void );
int i2c_init ( int );
void read_temphum( float*, float* );


#endif

