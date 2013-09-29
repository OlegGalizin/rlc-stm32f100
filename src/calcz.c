#include "calcz.h"
#include "calibration.h"
#include "itoa.h"
#include "n1202.h"
#include "main.h"


uint8_t FixDiapason; // Признак отключения автовыбора диапазона

#define ZARRAYSIZE 6 // Размер массивю Время измерения = (ZARRAYSIZE*2 + 2)*0.1сек
static Complex_t VArray[ZARRAYSIZE]; // Массив для хранения измерений напряжения
static Complex_t IArray[ZARRAYSIZE]; // Массив для хранения измерений тока
uint8_t AdcEvCounter; //номер измерения
uint8_t Step; //Шаг диапазона

Complex_t ZmSum; //Результат - Сумма результатов измерения Z 
#if defined(MSE)
float SqZmSumReal; //Рузультат - сумма квадратов действительной части Z
float SqZmSumImag;//Рузультат - сумма квадратов мнимой части Z
#endif
uint32_t ZmMeasCount;//Результат - количество измерений Z
uint8_t OverloadFlag;// Результат - флаг перегрузки.

#define MFLAG_V 0x1 // Бит выставлен в случае измерения напряжения
#define MFLAG_NO_SWITCH 0x2 // Не нужно переключаться на измерение след параметра. Можно продолжить измерение этого. Выставляетс после смены изм величины
#define MFLAG_M 0x4 //Начато измерение
#define MFLAG_SLOW 0x8 // Медленная смена диапазона
#define MFLAG_I_OK 0x10 // Медленная смена диапазона
#define MFLAG_V_OK 0x20 // Медленная смена диапазона
static uint8_t MFlags;  // Флаги измерения

static int8_t CheckDiapason(void); // Функция проверки диапазона. Возвращает 0 в случае правильного выбора. Иначе возвращает число для относительной корректировки диапазона.
void SetDiapason(void) // Функция установки измерения тока на выбранном диапазоне. Подсчет начинается с 0 при следующем измерении. Сброс требуется при смене тестируемого элемента
{
  uint8_t Switches;
  if ( MFlags & MFLAG_V ) // напряжение
    Switches = Diapasons[CurrentDiapason].VV;
  else
    Switches = Diapasons[CurrentDiapason].II;
  SetSwitches( Switches ); 
  ResetZm();
}

// Функция выбирает диапазон измерения, измеряет 6 значений напряжений и тока делит их друг на друга и результат
// добавляет в  ZmSum, такде добавляются квадраты действит и мнимых частей SqZmSumReal  SqZmSumImag
// Функцию необходимо вызывать при готовности следующего измерения напряжения или тока
// Функция возвращает 0, если произведен подсчет комплексного сопротивления
// Иначе возвращается знaчение !=0
uint8_t CalculationZm(void)
{
  int8_t DeltaDiapason;
  int8_t NewDiapason = CurrentDiapason;

  DeltaDiapason = CheckDiapason(); // Проверка диапазона
#if 0 // Отладочный вызов результата 
  {
  char Buf[5];
  itoa(Buf, DeltaDiapason);
  LcdChr(X_POSITION*8+Y_POSITION*7+4, Buf);
  }
#endif

  if ( ( FixDiapason == 0) && // Автоматическая смена диапазона включена
       ( DeltaDiapason != 0) ) // Если требуется смена диапазона 
  {
     NewDiapason = CurrentDiapason + DeltaDiapason; // Новый диапазон относительно текущего

    if ( NewDiapason < 0 ) // Требуется не сущетсвующий диапазон усиления по напряжению
      NewDiapason = 0;
    if ( NewDiapason > 16) // Требуется несуществующий диапазон по току
      NewDiapason = 16;

#if !defined(TEST_ADC) && !defined(AAA)
    while ( Calibration.InvalidFlag == 0 && 
         *((uint32_t*)&Calibration.Data.Z.stdm[NewDiapason + 17 * CurrentFreq]) == 0xFFFFFFFF )
    {// Диапазон без проведенной калибровки например по причине перегрузки
      if ( NewDiapason > 5 )
        NewDiapason--;
      else
        NewDiapason++;
    }
#endif
  }

  if ( NewDiapason != CurrentDiapason) // Требуется смена диапазона.
  {
    ResetZm(); // Предыдущие измерения недействительны
    CurrentDiapason = NewDiapason; // Меняем диапазон
    if (MFlags & MFLAG_SLOW) // Медленное изменение уже было включено но не помоглою Включаем быстрое.
    {
      Step = 6;
      MFlags &= MFLAG_V;
      return 6;
    }
    if (MFlags & MFLAG_V) // измерение напряжения
    {
      SetSwitches(Diapasons[NewDiapason].VV); // корректируем диапазон напряжения
    }
    else
    {
      SetSwitches(Diapasons[NewDiapason].II); // Корекция токового значения
    }
    AdcDelay(1); // Стандартная задержка в SetSwitches - 2 слишком велика - здесь не нужно точности
    return 1;// Пока выбираем диапазон. Результат не готов
  }
  else // Диапазон менять не нужно
  {
    if ( MFlags & (MFLAG_I_OK|MFLAG_V_OK) ) // Пока еще подстраиваемся
    {
      // Переключаемся на другой параметр
      if ( MFlags & MFLAG_V ) // Было измерение напряжения
      {
        SetSwitches(Diapasons[CurrentDiapason].II); // Измеряем ток
        MFlags ^= (MFLAG_V|MFLAG_V_OK); // Один сбросить другой установить
      }
      else
      {
        SetSwitches(Diapasons[CurrentDiapason].VV); // Измеряем напряжение
        MFlags |= (MFLAG_V|MFLAG_I_OK);
      }
      AdcDelay(1); // Стандартная задержка в SetSwitches - 2 слишком велика - здесь не нужно точности
      return 2; // Преключение с напряжения на ток или наоборот
    }
    //Начинаем измерять 
    if ( (MFlags & MFLAG_M) == 0)
    {
      // Дополнительная задержка перед началом измерения
      AdcDelay(2);
      MFlags |= MFLAG_M;
      return 3; // Задержка перед измерением
    }
    // Продолжаем измерять
  }
  
  { // Собираем данные в массив
    Complex_t* Item;
    if ( MFlags & MFLAG_V ) // напряжение
    {
      Item = &VArray[AdcEvCounter];
      Item->Re =  -ResultCosSum;
      Item->Im = ResultSinSum; // сохранили измерение
    }
    else
    {
      Item = &IArray[AdcEvCounter];
    
      Item->Re =  ResultCosSum;
      Item->Im = -ResultSinSum;
    }
  }

  if (AdcEvCounter == (ZARRAYSIZE-1)) // массив заполнен
  {
    if ( MFlags & MFLAG_NO_SWITCH ) // Не нужно переключаться. Можно выводить результат
    {
      goto GetResult; // Все массивы заполнены. подсчитать результат
    }
    
    // Переключаемся на другую величину 
    if ( MFlags & MFLAG_V ) // напряжение
    {
      SetSwitches(Diapasons[CurrentDiapason].II); // Переключаемся на измерение тока
      MFlags &= ~MFLAG_V;
    }
    else
    {
      SetSwitches(Diapasons[CurrentDiapason].VV); // Переключаемся на измерение напряжения
      MFlags |= MFLAG_V;
    }
    AdcEvCounter = 0;
    MFlags |= MFLAG_NO_SWITCH;
    return 4; // Закончено измерение напряжения  или тока
  }
  AdcEvCounter++; // Слудующий раз записать следующий элемент массива
  return 5;
      
GetResult:
  { // Закончено измерение тока или напряжения. Все данные собраны
    uint8_t j; 

    AdcEvCounter = 0;
    MFlags &= ~MFLAG_NO_SWITCH;

    for (j=0; j<ZARRAYSIZE; j++) // Расчет среднего знач и квадратичного отклонения
    {
      Complex_t In;

      if (IArray[j].Re == 0.0f && IArray[j].Im == 0.0f) // На 0 делить нельзя
        continue;
      In = ComplexDiv(VArray[j],IArray[j]);

      ZmSum.Re = ZmSum.Re +  In.Re; // Здесь накапливается среднее значение
      ZmSum.Im = ZmSum.Im +  In.Im;
#if defined(MSE)
      {
      float Tmp;
      Tmp = In.Re;
      SqZmSumReal += Tmp*Tmp; // Нужно для подсчета отклонения
      Tmp = In.Im;
      SqZmSumImag += Tmp*Tmp; // Нужно для подсчета отклонения
      }
#endif
      ZmMeasCount += 1; // Количество обсчитанных точек увеличивается на 1
    }
  }
  MFlags |= MFLAG_SLOW;
  return 0; // Подсчитано новое значение 
}

int8_t CheckDiapason(void)
{
  int8_t Ret = 0;
  
  if ( Step == 0 )
    Step++;
  if (MFlags & MFLAG_SLOW)
    Step = 1;

  // Шаг может быть уменьшен до 0 что бы не увеличивать чувствительность и не перегружать
  if ( ResultMin < 200 - 2048 || ResultMax > 3800 - 2048) // Перегрузка АЦП 
  {
    Ret = Step; // При перегрузке нужно менять диапазон обязательно
    OverloadFlag = 15; // Полный цикл измерения 12 тактов. Использовать побольше для надежности
  }
  else 
  {
    uint16_t Delta =  ResultMax - ResultMin;

    if ( Delta < 900 ) // Недогрузка АЦП
    {
        if (Delta < 20 )
          Ret = -Step;
        else if ( Delta < 90)
          Ret = -3;
        else if (Delta < 250 )
          Ret = -2;
        else  
          Ret = -1;
    }
    if ( OverloadFlag > 0 ) // Произошло нормальное измерение - можно сбросить флаг перегрузки
      OverloadFlag--;
  }
  if ( Step > 1  )
    Step--;

  if ((MFlags & MFLAG_V) == 0)  // Ток смещает диапазон в другую сторону
	{
    Ret = Ret * -1;
  }
  return Ret;
}

void ResetZm(void)
{
  ZmSum.Re = 0;
  ZmSum.Im = 0;
#if defined(MSE)
  SqZmSumReal = 0;
  SqZmSumImag = 0;
#endif
  ZmMeasCount = 0;
  AdcEvCounter = 0;
}


#if defined(TEST_CALCZ) && defined(TEST_ADC)
#include "n1202.h"
#include "out_float.h"
#include "itoa.h"

#if  defined(AUTO_RESET)
float PrevImMse;
float PrevReMse;
#endif

void CalculationZ(void)
{
  int8_t rr;

  if ( Event == EV_FUNC_FIRST )
  {
    ResetZm();
    LcdClear();
		FixDiapason = 0;
    return;
  }
  if ((Event & EV_MASK) == EV_KEY_PRESSED) 
  {
    switch (Event & KEY_MASK)
    {
      case KEY_ADC:
        goto redraw;
      case KEY_UP:
        if ( CurrentDiapason < 16 )
        {
					FixDiapason = 1;
          CurrentDiapason++;
          ResetZm();
          LcdClear();
        }
        return;
      case KEY_DOWN:
        if ( CurrentDiapason > 0 )
        {
					FixDiapason = 1;
          CurrentDiapason--;
          ResetZm();
          LcdClear();
        }
        return;
          
      default:
        CurrentFunc(TestAdcMenu);
        return;
    }
  }
  return;

redraw:
  rr = CalculationZm();
  if ( rr == 0)
  {
    float Av;
#if defined(MSE)
    float mse;
#if  defined(AUTO_RESET)
    uint8_t ResetFlag = 0;
#endif
#endif

    Av = ZmSum.Re/ZmMeasCount; // Расчет среднего действит части
    OutFloat(X_POSITION*0+Y_POSITION*2+MUL2, Av , ' '); // вывод лействит части
#if defined(MSE)
    mse = sqrtf((SqZmSumReal/ZmMeasCount - Av*Av)/ZmMeasCount/(ZmMeasCount - 1)); // Расчет отклонения действит части
    Av = mse/Av*1000; // Отклонение в процентах
    if ( Av < 0)
      Av = Av * -1;
    OutFloat(X_POSITION*5+Y_POSITION*4, Av , '%'); // Вывод относительного отклонения действит значения
#if  defined(AUTO_RESET)
    if (ZmMeasCount > 12 && Av > 1e-2f )
    {
      if (Av > PrevReMse ) // Ошибка увеличилась
        ResetFlag++; // Начать счет заново
      PrevReMse = Av; // Сохранить предыдущее отклонение
    }
#endif
#endif

    Av = ZmSum.Im/ZmMeasCount;  // Расчет мнимого среднего
    OutFloat(X_POSITION*0+Y_POSITION*5+MUL2, Av , ' '); // Вывод мнимой части
#if defined(MSE)
    mse = sqrtf((SqZmSumImag/ZmMeasCount - Av*Av)/ZmMeasCount/(ZmMeasCount - 1)); // Расчет отклоненря мнимой части
    Av = mse/Av*1000; // Отклонение в процентах
    if ( Av < 0)
      Av = Av * -1;
    OutFloat(X_POSITION*5+Y_POSITION*7, Av , '%'); // Вывод относит отлония мнимой части
#if  defined(AUTO_RESET)
    if (ZmMeasCount > 12 && Av > 1e-2f )
    {
      if (Av > PrevImMse ) // Ошибка увеличилась
        ResetFlag++; // Начать счет заново
      PrevImMse = Av; // Сохранить предыдущее отклонение
    }
    if ( ResetFlag )
    {
      ResetZm();
      PrevReMse = PrevImMse = 10000.0f;
    }
#endif
#endif
  }

  {
  char Buf[4];

  ShowDiapason(); //Диапазон измерения
  ShowFrenq();
  ShowOverload(); // Перегрузка

  itoa(Buf, rr); // То что вернула  CalculationZm 
  LcdChr(X_POSITION*0+Y_POSITION*1+4, Buf);
 
  itoa(Buf, ZmMeasCount); // Количество измерений
  LcdChr(X_POSITION*12+Y_POSITION*1+4, Buf);
  }
}
#endif
