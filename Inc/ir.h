#ifndef __ir_h
#define __ir_h

#define IR_TIMEOUT_MS               100  /* the max blanking is 95ms (repeat data) */

#define ERROR_DIFF                  100 /* us */

#define HEADER_HIGH                 9000
#define HEADER_LOW                  4500
#define DATA_0_HIGH                 590
#define DATA_0_LOW                  565
#define DATA_1_HIGH                 566         
#define DATA_1_LOW                  1664

#define REPEAT_LOW                  9566

#define IR_ADDRESS                  0x00


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
