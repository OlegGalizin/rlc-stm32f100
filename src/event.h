#if !defined(_EVENT_H_)
#define _EVENT_H_

#include <stm32f10x.h>

#define EV_KEY_TOUCH    0x0000
#define EV_KEY_PRESSED  0x0002 
#define EV_KEY_LONG     0x0003
#define EV_KEY_REPEATE  0x0004
#define EV_KEY_REALIZED 0x0005
#define EV_KEY_DOUBLE   0x0006
#define EV_FUNC_FIRST   0x0007
#define EV_MASK         0x0007

extern uint16_t Event;  /* 0xKKKKKAAA */
/* AAA - it is event key. Event can be defined by & operation with EV_MASK*/
/* KKKKKK - keys - one bit fpr one key. Up to 5 keys */
volatile extern uint16_t EventQueue; /* for setting user event */


extern volatile uint8_t EvCounter; // Press delay counter
typedef void (*MenuFunction_t)(void);
extern MenuFunction_t CurrentFunction;  // Current function
#define CurrentFunc(MenuFunc) CurrentFunction = MenuFunc

extern void EventInit(void);
extern void EventKeys(void); /* This function is periodicaly called (e.g. from ISR) */
extern void EventCheck(void); /* This function should be called to handele the event */
extern void EventIdle(void);

/* Event detect delays (unit is check intervals) */
#define KEY_PRESSED_VALUE 4 // Press event delay
#define KEY_LONG_VALUE    140  // Long press event delay
#define KEY_REPEATE_VALUE 160 // Repeate event delay
#define KEY_REALIZE_VALUE 8 //  Realize event detect delay


//#define KEYPORT GPIOB->IDR


#define KEY1  GPIO_IDR_IDR7
#define KEY2  GPIO_IDR_IDR6
#define KEY3  GPIO_IDR_IDR8
//#define KEY4  GPIO_IDR_IDR1
//#define KEY5  GPIO_IDR_IDR2
#define KEY6  GPIO_IDR_IDR15
#define KEY_MASK_SYS (KEY1|KEY2|KEY3)
#define KEY_MASK (KEY1|KEY2|KEY3|KEY6)

#define KEY_ENTER 	KEY1
#define KEY_DOWN 		KEY2
#define KEY_UP 	    KEY3
#define KEY_ADC     KEY6 /* Pseudo key */

#endif /* _EVENT_H_ */
