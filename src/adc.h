#if !defined(__SIGNALS_H__)
#define __SIGNALS_H__
#include "stm32f10x.h"

struct FrenqSetup_t;
// Функция инициализации генерации и измерения вызывается по указателю
typedef void (*InitFrenq_t) (const struct FrenqSetup_t*);
void InitForCalcualtingWithDacFill(const struct FrenqSetup_t* FSetup);
void InitForCalcualtingWithoutDacFill(const struct FrenqSetup_t* FSetup);

void Init(void); // init the measure system. Do once at start

// Структура инициализации
typedef struct FrenqSetup_t
{
  InitFrenq_t InitFunc;   // Функция инициализации
  int16_t  DacPrescaler;  //Прескалер для ЦАП счетчика
  int16_t  AdcPrescaler;  // Прескалер для АЦП счетцика
  int16_t*  DacSource; /* Source for DAC DMA. pointer as integer */
  int16_t  DacSize;   /* Size for DAC DMA */
  int16_t  RealTableSize; /* Size of the real sin table */
  const int16_t*  Sin; /* Table for fourier factor for 1/4 period */
  const int32_t*  CosSin; /* Table for fourier factors (sin and cos) for 1 period */
  uint16_t OptimalADCSize; // Optimal size for easy geting 10Hz frenq for measure
  uint16_t OptimalCallCount; // Optimal interrupt count for easy geting 10Hz frenq for measure
}
FrenqSetup_t;

void CalcualtingWithDacFill(int16_t Cur); // Функция медленной обработки результата. Она заполняет также массив для ЦАП
void CalcualtingWithoutDacFill(int16_t Cur); // Массив для ЦАП не заполняется - лежит во флеше

#define ADC_ARRAY_SIZE 496
#define SLOW_ARRAY_SIZE 500
#define ARRAY_SIZE 500
extern int16_t ADCResults[ARRAY_SIZE];
extern int16_t DACInput[ARRAY_SIZE];


#define FRENQ0   4
#define FRENQ100 0
#define FRENQ1K  1
#define FRENQ10K 2
#define FRENQ50K 3
extern uint8_t CurrentFreq; // Ткущая частота см макросы
void SetTheFrenq(uint8_t Frenq); //Функция установки частоты. Также подключается текущий вход. См ниже

// Массивы синусов-косинусов для генерации и умножения
extern const int16_t DacArrayDC; // dac input dor DC
extern const int16_t DacSinArray50[11]; // dac input 50K
extern const int32_t SinCosArray50[11]; // Sin Cos array 50K
extern const int16_t DacSinArray10[55]; // Dac input 10K
extern const int32_t SinCosArray10[55]; // Sin Cos array 10K

#define BEGIN_FLAG 0x8000
#define END_FLAG 0x4000
extern const int16_t SinArray1[41];  // 1/4 period Sin array 1K for sin cos and dac calculation
extern const int16_t SinArray100[401]; // 1/4 period Sin array 100Hz for sin cos and dac calculation

// Аналоговые входы
#define INST_OUT ADC_SQR3_SQ1_0|ADC_SQR3_SQ1_1  // 3 канал
#define MAIN_OUT ADC_SQR3_SQ1_1 // 2 канал
#define PWR_OUT  ADC_SQR3_SQ1_3 // 8 канал
#define REF_OUT  ADC_SQR3_SQ1_4|ADC_SQR3_SQ1_0 // 17 канал
#define SetAdcInput(Inp)   ADC1->SQR3 = Inp;
//extern uint32_t AdcInput; // Текущий вход. 

// Переменные возврата результата измерения
#if defined(PWRMEAS)
extern uint16_t ResultPowerAdc; // Питание
extern uint16_t ResultRefAdc;  // Опора
#endif
extern int16_t ResultMin; // Минимальное значение АЦП -- используется ри выборе диапазона
extern int16_t ResultMax;  // Максимальное значение АЦП
extern int32_t ResultAverage; // Среднее значение измерения - для измерения постоянной составляющей
extern int64_t ResultCosSum; // Сумма произведений на косинус
extern int64_t ResultSinSum; // Сумма произведений на синус
//extern uint16_t ResultPowerAdc; //Результат измерения питания 3.3В
//extern uint16_t ResultRefAdc;  //результат измерения внутренней опоры


//Дефайны выбора коэфициентов для диапазонов измерения
#define K1_SWITCH 0x01
#define K2_SWITCH 0x02
#define K3_SWITCH 0x04
#define R_SWITCH1 0x08
#define R_SWITCH2 0x10
#define I_SWITCH  0x80 // Старший бит говорит  что нужно измерять ток а не напряжение

#define R_100 0
#define R_1K  R_SWITCH1
#define R_10K R_SWITCH2
#define R_100K (R_SWITCH2|R_SWITCH1)
void SetSwitches(uint8_t Switches); // Установка ключей. Передаются макросы выше

typedef struct Diapason_t
{
  uint8_t VV;
  uint8_t II;
}Diapason_t; // Структура определения диапазона. Поля - включенные свитчи для тока и напряжения
extern const Diapason_t Diapasons[18];

extern int8_t CurrentDiapason; // Текущий диапазон измерения
void AdcDelay(int8_t Count); // Пропустить count циклов измерения

#if defined(TEST_ADC)
void TestMenu(void);
void TestAdcMenu(void);
extern uint32_t HandlerTicks;
#define    DWT_CYCCNT    *(volatile unsigned long *)0xE0001004
#define    DWT_CONTROL   *(volatile unsigned long *)0xE0001000
#define    SCB_DEMCR     *(volatile unsigned long *)0xE000EDFC
#endif


#endif /* __SIGNALS_H__*/

