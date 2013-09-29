#if !defined(__IO_H__)
#define __IO_H__

#include "gpio.h"

#define PWR_ON    A, 1, OUT_PUSHPULL|OUT2MHZ, 1  // Включение-удержание питания
#define K3        A, 6, OUT_PUSHPULL|OUT2MHZ, 0  // Коэф усиления усилителя перем тока
#define K2        A, 7, OUT_PUSHPULL|OUT2MHZ, 1  // Коэф усиления диф усилителя
#define ADC       A, 2, AINPUT, 0 // Входной измеряемый сигнал - 2 аналоговый вход
#define INSTR_OUT A, 3, AINPUT, 0 // Сигнал после инстр усил - используется для настройки смещений по постоянному току - 3 аналог вход
#define DAC_OFF   A, 4, AINPUT, 0 // Смещение инструм усилителя - ЦАП 1 канал
#define DAC_M     A, 5, AINPUT, 0 // Выходной сигнал синуса - ЦАП 2 канал

#define V_BAT     B, 0, AINPUT, 0 // Напряжение питания после делителя  - 8 аналоговый вход
#define BUT1      B, 7, INPUT_PULL, PULL_UP // Первая кнопка
#define BUT2      B, 6, INPUT_PULL, PULL_UP // Вторая кнопка
#define PROBE1    B, 5, INPUT_PULL, PULL_UP // Индикатор подключенного щупа
#define PROBE2    B, 4, INPUT_PULL, PULL_UP // Индикатор подключенного щупа
#define IV1       B, 1, OUT_PUSHPULL|OUT2MHZ, 0 // Резистор в iv преобразователе
#define IV2       B, 2, OUT_PUSHPULL|OUT2MHZ, 0 // Резистор в iv преобразователе

#define LCD_LED   A, 12, OUT_PUSHPULL|OUT2MHZ, 0 // Подсветка
#if defined(LCD_USART)
#define LCD_CLK   A, 8, AOUT_PUSHPULL|OUT2MHZ, 0 // Тактовый сигнал дисплея
#define LCD_DA    A, 9, AOUT_PUSHPULL|OUT2MHZ, 0 // Данные дисплея
#else
#define LCD_CLK   A, 8, OUT_PUSHPULL|OUT2MHZ, 0
#define LCD_DA    A, 9, OUT_PUSHPULL|OUT2MHZ, 0
#endif
#define LCD_CE    A, 10, OUT_PUSHPULL|OUT2MHZ, 0 // Выбор кристалла дисплея
#define LCD_RESET A, 11, OUT_PUSHPULL|OUT2MHZ, 0 // Сброс дисплея

#define I_V_MEAS  B, 11, OUT_PUSHPULL|OUT2MHZ, 0 // Мультиплексор измерения тока или напряжения
#define K1        B, 10, OUT_PUSHPULL|OUT2MHZ, 0 // Коэв усиления предусилителя инстр усилителя
#define PWM       B, 9, AOUT_PUSHPULL|OUT2MHZ, 0 // Смещение усилителя мощности
#define BUT3      B, 8, INPUT_PULL, PULL_UP // третья кнопка
#define CP0       B, 13, AOUT_PUSHPULL|OUT10MHZ, 0 // Сигнал на зарядовый насос
#define CP1       B, 14, AOUT_PUSHPULL|OUT10MHZ, 0 // Сигнал на зарядовый насос

#endif /* __IO_H__ */
