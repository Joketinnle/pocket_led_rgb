#ifndef __ir_h
#define __ir_h

#define IR_TIMEOUT_MS               100  /* the max blanking is 95ms (repeat data) */

#define ERROR_DIFF                  100 /* us */

#define HEADER_HIGH                 9000
#define HEADER_LOW                  4500
#define DATA_0_HIGH                 590
#define DATA_0_LOW                  565
#define DATA_1_HIGH                 1664         
#define DATA_1_LOW                  566



enum NEC_CODE_STATUE {
    HEADER = 0,
    ADDR,
    CMD,
    REPEAT,
    ABANDON
};


enum IR_CMD {
    


};



struct ir_data{
    

};



void ir_timer_callback_func(void);

#endif
