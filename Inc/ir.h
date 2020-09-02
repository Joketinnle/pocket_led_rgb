#ifndef __ir_h
#define __ir_h

#define IR_TIMEOUT_MS               100  /* the max blanking is 95ms (repeat data) */

#define ERROR_DIFF                  180 /* us */

#define HEADER_HIGH                 9000
#define HEADER_LOW                  4500
#define DATA_0_HIGH                 640
#define DATA_0_LOW                  520
#define DATA_1_HIGH                 640         
#define DATA_1_LOW                  1595

#define REPEAT_LOW                  30061

#define IR_ADDRESS                  0x00


/* .......... remote button map .............*/
#define IR_CH_MINUS                     0x45
#define IR_CH_ENTER                     0x46
#define IR_CH_PLUS                      0x47
#define IR_PREV                         0x44
#define IR_NEXT                         0x40
#define IR_ONOFF                        0x43
#define IR_VOL_MINUS                    0x07
#define IR_VOL_PLUS                     0x15
#define IR_EQ                           0x09
#define IR_ZERO                         0x16
#define IR_100_PLUS                     0x19
#define IR_200_PLUS                     0x0D
#define IR_ONE                          0x0C
#define IR_TWO                          0x18
#define IR_THREE                        0x5E
#define IR_FOUR                         0x08           
#define IR_FIVE                         0x1C
#define IR_SIX                          0x5A
#define IR_SEVEN                        0x42
#define IR_EIGHT                        0x52
#define IR_NINE                         0x4A





enum NEC_CODE_STATUE {
    HEADER = 0,
    ADDR,
    CMD,
    REPEAT,
    ABANDON
};


//enum IR_CMD {
//    


//};



struct ir_data{
   uint8_t addr;
   uint8_t addr_inverse;
   uint8_t cmd;
   uint8_t cmd_inverse;
};



void ir_timer_callback_func(void);

#endif
