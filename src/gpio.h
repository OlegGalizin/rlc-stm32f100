#if !defined(__GPIP_H__)
#define __GPIP_H__
#include "stm32f10x.h"
// определение пина должно выглядить
//#define PIN_NAME PORT, PIN, OUT_MODE_SPEED, SET
// PORT - A, B, C, D и тд - порт ВВ
// PIN - номер бита в порте
// OUT_MODE_SPEED - режим выхода и скорость 
// SET - значение после инициализации для выхода или подтяжка для входа
// скорость. Имеет смысл только для выхода. Объединяется | с режимом
#define OUT10MHZ GPIO_CRL_MODE0_0  // режим выхода среднескоростной - в поле MODE
#define OUT2MHZ  GPIO_CRL_MODE0_1  // низкоскоростной
#define OUT50MHZ (GPIO_CRL_MODE0_1|GPIO_CRL_MODE0_0)  // высокоскоростной

// режимы
#define OUT_PUSHPULL 0 // двутактный выход
#define AOUT_PUSHPULL GPIO_CRL_CNF0_1 //альтернативный двутактный
#define OUT_OPENDRAIN GPIO_CRL_CNF0_0 // открытый сток
#define AUOT_OPENDRAIN (GPIO_CRL_CNF0_0|GPIO_CRL_CNF0_1) //альтернативный открытый сток
//
#define AINPUT 0  //аналоговый вход
#define INPUT GPIO_CRL_CNF0_0  // вход цифровой 
#define INPUT_PULL GPIO_CRL_CNF0_1 //вход цифровой с подтяжкой

//подтяжка
#define PULL_DOWN 0 // подтяжка к земле - необходимо присвоение регистра устновки-сброса
#define PULL_UP   1 // подтяжка к питанию


#define _TO_GPIO_CRL(DEST_PORT, PORT, PIN, MODE, OUT) \
  ((&GPIO##DEST_PORT->CRL == &GPIO##PORT->CRL)*(PIN<8)*(MODE))<<(PIN*4)
#define _TO_GPIO_CRH(DEST_PORT, PORT, PIN, MODE, OUT) \
  ((&GPIO##DEST_PORT->CRH == &GPIO##PORT->CRH)*(PIN>=8)*(MODE))<<((PIN-8)*4)
//макросы используются для присвоения управляющему младшему или старшему регистрам. Макросы должны объединяться через |
#define TO_GPIO_CRL(DEST_PORT, PORT_DESC) _TO_GPIO_CRL(DEST_PORT, PORT_DESC)
#define TO_GPIO_CRH(DEST_PORT, PORT_DESC) _TO_GPIO_CRH(DEST_PORT, PORT_DESC)

#define _TO_GPIO_BSRR(DEST_PORT, PORT, PIN, MODE, OUT) /*((OUT)==0)?0:*/((&GPIO##DEST_PORT->BSRR == &GPIO##PORT->BSRR)*(OUT<<PIN))
//Присваивается регатру установки-сброса. Используется если нужна инициализация или подтяжка
#define TO_GPIO_BSRR(DEST_PORT, PORT_DESC) _TO_GPIO_BSRR(DEST_PORT, PORT_DESC)

#define _GPIO_SET(PORT, PIN, MODE, OUT) GPIO##PORT->BSRR = (1 << PIN)
#define _GPIO_ISSET(PORT, PIN, MODE, OUT) (GPIO##PORT->IDR & (1 << PIN))
#define _GPIO_RESET(PORT, PIN, MODE, OUT) GPIO##PORT->BRR = (1 << PIN)
#define _GPIO_TOGGLE(PORT, PIN, MODE, OUT) GPIO##PORT->ODR ^= (1 << PIN)


//соотв макросы работы с портом
#define GPIO_SET(PIN_DESC) _GPIO_SET(PIN_DESC)
#define GPIO_RESET(PIN_DESC) _GPIO_RESET(PIN_DESC)
#define GPIO_TOGGLE(PIN_DESC) _GPIO_TOGGLE(PIN_DESC)
#define GPIO_ISSET(PIN_DESC) _GPIO_ISSET(PIN_DESC)

#define _GPIO_BB(PORT, PIN, MODE, OUT) (*(uint32_t*)(PERIPH_BB_BASE + ((uint32_t)&(GPIO##PORT->ODR) -  PERIPH_BASE)*32 + PIN*4))
// макрос работы с портом через bit-bang
#define GPIO_BB(PIN_DESC) _GPIO_BB(PIN_DESC)
#endif /* __GPIP_H__ */
