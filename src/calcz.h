#if !defined(__CALCZ_H__)
#define __CALCZ_H__
#include <math.h>
#include <stdlib.h>
#include "compl.h"
#include "adc.h"
#include "event.h"
extern uint8_t FixDiapason; // Отключение функции автовыбора диапазона
extern uint8_t OverloadFlag; // Флаг перегрузки. Проверять после вызова CalculationZm

void ResetZm(void); // Функция принудительного сброса намерянного
void SetDiapason(void);// Функция установки измерения на выбранном диапазоне. Подсчет начинается с 0 при следующем измерении. Сброс требуется при смене тестируемого элемента

// Вычисляет на основании ResultCosSum ResultSinSum текущее Zm. На основании ResultMax ResultMin выбирает диапазон если это включено
uint8_t CalculationZm(void); // Функция измерения реактивного сопротивления в попугаях в след переменных

extern Complex_t ZmSum; //Результат - Сумма результатов измерения Z 
#if defined(MSE)
extern float SqZmSumReal; //Рузультат - сумма квадратов действительной части Z
extern float SqZmSumImag;//Рузультат - сумма квадратов мнимой части Z
#endif
extern uint32_t ZmMeasCount;//Результат - количество измерений Z

#if defined(TEST_CALCZ)
void CalculationZ(void);
#endif

extern uint8_t AdcEvCounter; //номер измерения


#endif /* __CALCZ_H__ */
