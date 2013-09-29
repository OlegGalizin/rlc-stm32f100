#include "event.h"
#include "adc.h"


typedef void (*CurrentHandler_t)(int16_t Cur);
typedef union{
  const int32_t* CosSin;
  const int16_t* Sin;
  } PCosSin_t;


static CurrentHandler_t CurrentHandler; // Текущая функия которая вызывается из обрабтчика ПДП
static uint16_t          CountHandlersIn10Hz; // Количество вызовов обработчика для формирвоания события 10 раз в сек
static uint16_t          CountHandlers; // Текущее количество вызовов обработчика
static int16_t HMin; // Текущее минимальное значение
static int16_t HMax;
static int32_t HAverage; // Сумма для подсчета среднего
static int64_t HCosSum; // Сумма произведения на синус Текущая
static int64_t HSinSum; // Сумма произведений на косинус
static const int32_t* CurrentCosSinPointer; // Ткущий указатель в таблице синусов-косинусов
static PCosSin_t PCosSin;
static const int16_t* SinPointer;   // Текущий уазатель в таблице синусов
static int8_t   SinDirection; // Направление движения по таблице синусов CalcualtingWithDacFill
static int8_t   SinFactor;    // Знак синуса      see CalcualtingWithDacFill
static const int16_t* CosPointer;   // Текущий уазатель в таблице синусов для генерации косинуса
static int8_t   CosDirection; // direction for cos +1 or -1
static int8_t   CosFactor;    // factor for cos +1 or -1
static int16_t  DmaHalfSize; // Размер половины массива

static volatile int8_t   SkipCount; // Количество событий, которые будут пропущены Первое событие не содержит достоверных данных. Также после смены коэффициента несколько первых событий нужно пропустить

// В этих перемынных складываются результаты после генерации события
uint16_t ResultPowerAdc; // Питание
uint16_t ResultRefAdc;  // Опора
int16_t ResultMin; // Минимальное измеренное значение
int16_t ResultMax; // Таксимально измеренное значение
int32_t ResultAverage; // Среднее измеренное значение
int64_t ResultCosSum; // Результат суммирования произведений измерения на косинус - реальная часть измерения
int64_t ResultSinSum; // -//- на синус - мнимая часть

// Функция инициализации измерений с заполением массива ЦАП
void InitForCalcualtingWithDacFill(const struct FrenqSetup_t* FSetup)
{
  int i;

  CurrentHandler = CalcualtingWithDacFill; // Функция обработки с заполением масива ЦАП

  CountHandlersIn10Hz = FSetup->OptimalCallCount; // Количество вызовов обработчика для получения частоты событий в 10 герц
  CountHandlers = CountHandlersIn10Hz; // текущее кол-во вызовов обработчика
  DmaHalfSize = SLOW_ARRAY_SIZE/2; // Половина размера таблицы измерений
  PCosSin.Sin = FSetup->Sin; // Таблица синусав
  SinPointer = FSetup->Sin; // Указатель на начало таблицы синусов
  CosPointer = FSetup->Sin + FSetup->RealTableSize-1; // Косинус - тот же синус но сдвинутый на четверть периода
  SinFactor  = -1; // После первого раза знак будет положительный
  SinDirection = -1;// После смены направления синус пойдет вперед
  CosFactor  = 1; // Знак будет отрицательный
  CosDirection = 1; // Направление будет от конца к началу
  //C Текущее значение таблицы ЦАП - нулевые значение выхода
  for (i=0; i<ADC_ARRAY_SIZE;i++)
  {
    DACInput[i] = 2048; // Они будут заполнены настоящими значениями
  }
}

void InitForCalcualtingWithoutDacFill(const struct FrenqSetup_t* FSetup)
{
  int i;

  CurrentHandler = CalcualtingWithoutDacFill;
  CountHandlersIn10Hz = FSetup->OptimalCallCount;
  CountHandlers = CountHandlersIn10Hz;
  DmaHalfSize = ADC_ARRAY_SIZE/2; // Массив уже другого размера
  PCosSin.CosSin = FSetup->CosSin;
  CurrentCosSinPointer = PCosSin.CosSin;
  for ( i = 0; i < FSetup->DacSize; i++)
  {
    DACInput[i] = FSetup->DacSource[i]; // Эти значения перетираться не будут
  }
}

void AdcDelay(int8_t Count)
{
  __disable_irq();
  SkipCount = Count;
//  SkipCount = 15;
  EventQueue = 0;   // Не очень хорошо терять например нажатие клавиши но еще хуже измерить неизвестно что
  CountHandlers = 0;
  __enable_irq();

}

void DMA1_Channel3_IRQHandler(void) /* Read ADC values */
{
  int16_t Cur;

  if (DMA1->ISR & DMA_ISR_HTIF3 && /*   Half Transfer complete */
       (DMA1_Channel3->CCR & DMA_CCR3_HTIE) /* Half traisfer interrupt enabled */
      ) 
  {
    Cur = 0; // Данные для обработки в массиве
    DMA1_Channel3->CCR &= ~DMA_CCR3_HTIE; /* disable Half traisfer interrupt !!!! */
  } else if (DMA1->ISR & DMA_ISR_TCIF3 ) /* transfer complete */
  {
    DMA1_Channel3->CCR |= DMA_CCR3_HTIE; /* enable Half traisfer interrupt  */

    Cur = DmaHalfSize; // вторая половина буфера
  }
  else
  {
    DMA1->IFCR |= DMA_IFCR_CGIF3; /* Clear all interrupt flags */
    return;
  }
  DMA1->IFCR |= DMA_IFCR_CGIF3; /* Clear all interrupt flags */
  CurrentHandler(Cur); // Вызов обработчика для данного массива и частоты

  if (EvCounter != 0 ) // any key was pressed
  {
	if ( SkipCount == 0 ) // Пришло время генерировать событие но клавиша нажата
	{
      CountHandlers = 0;
	  SkipCount = 2;  // Поэтому пропускаем генерацию события 
	}
  }

  if (CountHandlers == 0) // Количество вызовов обработчика достаточно для вормирования события
  {
    // Generate end convertion event
    if ( SkipCount == 0 ) // Событие не генерируется так как мог быть незаполненным DAC и в ADC содержаться мусор
    {
      if ( EventQueue == 0) //  Никакая клавиша на нажата
        EventQueue = EV_KEY_PRESSED|KEY_ADC;
    }
    else
    {
      SkipCount--;
    }
    ResultMin = HMin; // Сохранеие посчитанных параметров
    ResultMax = HMax;
    ResultAverage = HAverage/DmaHalfSize/CountHandlersIn10Hz; // Расчет среднего
    ResultCosSum = HCosSum/256; // Сохранение реальной части
    ResultSinSum = HSinSum/256; // Сохранение мнимой части
    HCosSum = 0; // Обнуление текущих счетчиков
    HSinSum = 0;
    HAverage = 0;
    HMin = 4096;
    HMax = -4096;
    CountHandlers = CountHandlersIn10Hz;
    // Событие EV_KEY_PRESSED|KEY_ADC сформировано
    // Результаты в  ResultMin ResultMax  ResultAverage - измерения по постоянному току - макс мин и среднее значение
    // ResultCosSum  ResultSinSum - суммы прозведений отсчетов на синус - пропорциональны реальной и мнимой частям измерения
  }
  CountHandlers--; // количество вызовов - массив обрабатывается за 2 вызова
#if defined(PWRMEAS)
  {
  	static uint32_t AdcInput;

  	if ( CountHandlers == 8 && SkipCount == 2 ) // Измерение постоянных напряжений 
  	{
      AdcInput = ADC1->SQR3; // save original input
      ADC1->SQR3 = REF_OUT;  // Measure reference voltage
    }
    if ( AdcInput != 0 )
    {
      switch (CountHandlers)
      {
        case 4: // Возврат на измерение основного параметра
          ADC1->SQR3 =  AdcInput;
          AdcInput = 0;
          ResultPowerAdc = HAverage/DmaHalfSize + 2048;
          break;
        case 5: // Сброс счетчика среднего
        case 7: 
          HAverage = 0;
          break;
        case 6: // Переключение на измерение с делитеоя питания
          ADC1->SQR3 = PWR_OUT;
          ResultRefAdc = HAverage/DmaHalfSize+ 2048; // Сохранения результата измерения опоры
          break;
      }
    }
  }
#endif
}


#pragma O3
#pragma Otime
// Обработчик низких частот с генерацией таблицы SIN. В результате таблицу в флеше можно уменьшить до четверти периода
void CalcualtingWithDacFill(int16_t Cur) /* About 60 ticks for every point */
{
  int16_t Result;
  int16_t CosSin;
  int32_t CosSumLocal = 0;
  int32_t SinSumLocal = 0;
  int32_t AverageSum=0;
  int16_t End;

  End = Cur + SLOW_ARRAY_SIZE/2; // Конечный указатель на данные в массиве

  do
  {
    Result = ADCResults[Cur] - 2048; // Взять текущий отсчет
    CosSin = *CosPointer; // Взять текущее значение сосинуса из массива
    if (CosSin & (BEGIN_FLAG|END_FLAG)) // Первая и последняя точка в массиве
    {
      CosDirection *= -1; // Изменить направление
      SinDirection *= -1;
      if ( CosSin & BEGIN_FLAG )
        CosFactor *= -1; // Изменить знак у косинуса в начале четверти периода
      else
        SinFactor *= -1; // Знак синуса меняется в конце четверти
      CosSin &= ~(BEGIN_FLAG|END_FLAG);
    }
    CosSumLocal += CosSin * Result * CosFactor; // Произведение на косинус
    CosPointer += CosDirection; // передвинуть текущий вперед или назад

    CosSin = *SinPointer; // Взять текуще значение синуса
    CosSin &=  ~(BEGIN_FLAG|END_FLAG);
    CosSin *= SinFactor; // Участь знак
    DACInput[Cur] = CosSin + 2048; // Сформировать точку для вывода
    SinSumLocal += CosSin * Result; // Произведение на синус
    SinPointer += SinDirection; // Следующий элемент синуса в массиве

    AverageSum += Result; // Сумма отчетов для подсчета среднего
    if (Result > HMax )
      HMax = Result; // Сменить максммум
    if (Result < HMin)
      HMin = Result; // Сменить минимум
    Cur++; // Сдвинуться в массиве отсчетов
  } while ( Cur < End);

  DMA1->IFCR |= DMA_IFCR_CGIF1; /* Clear all interrupt flags */
  
  HCosSum += CosSumLocal; // Сохранить значение сумм в глобальных переменных
  HSinSum += SinSumLocal;
  HAverage += AverageSum;
}

#pragma O3
#pragma Otime
// Обработка отсчетов. Таблица синусов и косинусов полная со всеми точками
void CalcualtingWithoutDacFill(int16_t Cur) /* About 36 ticks for every point */
{
  int32_t Result;
  int32_t LocalCosSum = 0;
  int32_t LocalSinSum = 0;
  int32_t AverageSum=0;
  int16_t End = Cur+ADC_ARRAY_SIZE/2;
  const int32_t* CosSinPointerLocal = CurrentCosSinPointer;

  do
  {
    int32_t CosSin;

    CosSin = *CosSinPointerLocal; // Прочитать текущее значение из таблицы синусов-косинусов
    if ( CosSin & 0x1 ) /* Признак последнего элемента в массиве*/
    {
      CosSinPointerLocal = PCosSin.CosSin - 1; /* Установить указатель перед первым элементом массива синусов-косинусов */
    }
    CosSinPointerLocal += 1; // Сдвинуть указатель на следующий элемент в массиве синусов-косинусов
    Result = ADCResults[Cur] - 2048; // Прочитать результат
    LocalCosSum += (CosSin>>17) * Result; // 15 старших бит - косинус. Умножается на результат
    LocalSinSum += (CosSin<<15>>16) * Result;  // 15 младших бит, кроме последнего - синус. Нужно восстановить знак!!! - 2 операции сдвига
    AverageSum += Result; // Среднее арифметическое
    if (Result > HMax )
      HMax = Result; // Максимум
    if (Result < HMin)
      HMin = Result; // Минимум
    Cur++; // Следующий отсчет
  } while ( Cur < End);

  DMA1->IFCR |= DMA_IFCR_CGIF1; /* Clear all interrupt flags */
  
  // Сохранение результатов в глобальных переменных
  HCosSum += LocalCosSum;
  HSinSum += LocalSinSum;
  HAverage += AverageSum;
  CurrentCosSinPointer = CosSinPointerLocal; // Сохранение указателя в массиве косинусов-синусов
}
