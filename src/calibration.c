#include <math.h>
#include "event.h"
#include "n1202.h"
#include "adc.h"
#include "calcz.h"
#include "flash.h"
#include "itoa.h"
#include "calibration.h"
#include "out_float.h"
#include "main.h"

static void CalibrationInstr(void);
static void CalibrationOut(void);
static void CalibrationZ(void);
static void LoadCalibrationStart(void);
static uint8_t SavedFreq;


void CalibrationStart(void)
{
  if ( Event == EV_FUNC_FIRST )
  {
    LcdClear();
    LcdChr( Y_POSITION*2+X_POSITION*0 + 16, "Please short");
    LcdChr( Y_POSITION*3+X_POSITION*0 + 16, "the probes and" );  
    LcdChr( Y_POSITION*4+X_POSITION*0 + 16, "press any key" ); 
    return;
  }
  if ( (Event & EV_MASK) == EV_KEY_PRESSED )
    if ( (Event & KEY_MASK_SYS) )
  {
    SavedFreq = CurrentFreq;
    CurrentFunc(CalibrationInstr);
    return;
  }
}

#define CALIBRATION_MODE_SHORT 1
#define CALIBRATION_MODE_OPEN  2
#define CALIBRATION_MODE_LOAD  4
uint8_t CalibrationMode;

static void CalibrationInstr(void)
{  // Смещение инструментального усилителя
  if ( Event == EV_FUNC_FIRST )
  {
    SetSwitches(Diapasons[17].VV);
    SetAdcInput(INST_OUT);
    SetTheFrenq(FRENQ0); //Измерение вых инстр ус
    AdcEvCounter = 12; // Переиспользуеи 
    LcdClear();
    return;
  }
  if ( ((Event & EV_MASK) == EV_KEY_PRESSED) &&
       ((Event & KEY_MASK) == KEY_ADC ))
  {
    if ( AdcEvCounter-- )
    {
      char Buf[9];

      DAC->DHR12R1 = DAC->DOR1 + (ResultAverage+2048-1862) * 6;
      itoa(Buf, DAC->DOR1);
      LcdChr(X_POSITION*0+Y_POSITION*3+4 , Buf);
      itoa(Buf, ResultAverage+2048);
      LcdChr(X_POSITION*0+Y_POSITION*4+4 , Buf);
    }
    else
    {  
#if !defined(TEST_ADC)
      if (Calibration.InvalidFlag)
      {
        uint16_t Offset = DAC->DHR12R1;

        WriteFlash(&Offset, (void*)&Calibration.InstrOffset, sizeof(uint16_t));
      }
#endif
      CurrentFunc(CalibrationOut);     
    }

    return;
  }
}

static void CalibrationOut(void)
{ // Смещение усилителя DAC
  if ( Event == EV_FUNC_FIRST )
  {
    SetSwitches(Diapasons[17].II);
    SetTheFrenq(FRENQ0); //Измерение вых инстр ус
    AdcEvCounter = 64;
    LcdClear();
    return;
  }
  if ( ((Event & EV_MASK) == EV_KEY_PRESSED) &&
       ((Event & KEY_MASK) == KEY_ADC ))
  {
    if ( AdcEvCounter-- )
    {
      char Buf[9];

      TIM17->CCR1 = TIM17->CCR1 - (ResultAverage+2048-1862) / 3;
      itoa(Buf, TIM17->CCR1);
      LcdChr(X_POSITION*0+Y_POSITION*3+4 , Buf);
      itoa(Buf, ResultAverage+2048);
      LcdChr(X_POSITION*0+Y_POSITION*4+4 , Buf);
    }
    else
    {
      if (Calibration.InvalidFlag) // Полная калибровка
      {
#if !defined(TEST_ADC)
        uint16_t Offset = TIM17->CCR1;

        /* Save offset */
        WriteFlash(&Offset, (void*)&Calibration.OutOffset, sizeof(uint16_t));
#endif
        SetTheFrenq(FRENQ100); 
      }
      else
        SetTheFrenq(SavedFreq);// В случае О-Ш калибровки восстанавливаем частоту

      /* Next function is short calibration */
      CalibrationMode = CALIBRATION_MODE_SHORT;
//      CalibrationMode = CALIBRATION_MODE_LOAD;
      CurrentDiapason = 0;
      SetDiapason();
      AutoDiapason = 0;
      SetAdcInput(MAIN_OUT);
#if !defined(TEST_ADC)
      CurrentFunc(CalibrationZ);
//      CurrentFunc(LoadCalibrationStart);
#else      
      CurrentFunc(TestAdcMenu);
#endif
    }
    return;
  }
}

const float CFrenq[4] = {
100*2*M_PI, 
1000*2*M_PI,
9.917e3f*2*M_PI,
49.587e3f*2*M_PI
};

float RMult(int8_t diapason)
{
  uint32_t ret = 1;
  int8_t i;
  int8_t exp = diapason/2;

  for (i = 0; i < exp; i++)
  {
    ret = ret * 10;
  }

  return ret * 1e-4f;
}

#if !defined(CONST_CALIBRATION)
const Calibration_t Calibration __attribute__ ((at(FLASH_BASE+1024*14))) = 
{
#include "zstd.h"
  {
#include "1088ff.h"
  },
  0xFFFF,
  0xFFFF,
  0xFFFF
};
#else // CONST_CALIBRATION
const Calibration_t Calibration /*__attribute__ ((at(FLASH_BASE+1024*15))) */= 
{
  {
#include "zstd.h"
  },
  {
#include "1088calibr.h"
  },
  0x620,
  0x6e3,
  0x0
};
#endif // CONST_CALIBRATION

void OpenCalibrationStart(void)
{
  if ( Event == EV_FUNC_FIRST )
  {
    LcdClear();
    LcdChr( Y_POSITION*2+X_POSITION*0 + 16, "Please open");
    LcdChr( Y_POSITION*3+X_POSITION*0 + 16, "the probes and" );  
    LcdChr( Y_POSITION*4+X_POSITION*0 + 16, "press any key" ); 
    return;
  }
  if ( (Event & EV_MASK) == EV_KEY_PRESSED )
    if ( (Event & KEY_MASK_SYS) )
  {
    SetAdcInput(MAIN_OUT);
    CurrentDiapason = 6;
    SetDiapason();
    if (Calibration.InvalidFlag)  // Начальная калибровка
      SetTheFrenq(FRENQ100);
    CalibrationMode = CALIBRATION_MODE_OPEN;
    CurrentFunc(CalibrationZ);
    return;
  }
}

#if defined(MSE)
float GetPercent(float MeanValue, float SumOfSquare)
{
  float mse = sqrtf((SumOfSquare/ZmMeasCount - MeanValue*MeanValue)/ZmMeasCount/(ZmMeasCount - 1)); // Расчет отклоненря
  float Av = mse/MeanValue*1000; // Относительное отклонение
  if ( Av < 0)
    Av = Av * -1;
  return Av;
}
#endif


void LoadCalibrationStart(void)
{
  if ( Event == EV_FUNC_FIRST )
  {
    float Resistance;
    
    LcdClear();

    if ( CurrentDiapason > 16)   
    {
      uint16_t InvalidFlag = 0; /* 2 byte */

      WriteFlash(&InvalidFlag, (void*)&Calibration.InvalidFlag,  sizeof(InvalidFlag));
#if defined(TEST_ADC)
      CurrentFunc(TestAdcMenu);
#else
      CurrentFunc(MainForm);
#endif
      return;
    }

    LcdChr( Y_POSITION*0+X_POSITION*0 + 16, "Please insert");
    LcdChr( Y_POSITION*1+X_POSITION*0 + 16, "the resistor" );  
    Resistance = Calibration.Rstd[CurrentDiapason]*RMult(CurrentDiapason);
    OutFloat(X_POSITION*0+Y_POSITION*3+MUL2, Resistance , Omega);
    LcdChr( Y_POSITION*5+X_POSITION*0 + 16, "& press any key" ); 
    return;
  }
  if ( (Event & EV_MASK) == EV_KEY_PRESSED &&
       (Event & KEY_MASK_SYS) )
  {
    SetTheFrenq(FRENQ100);
    CalibrationMode = CALIBRATION_MODE_LOAD;
    CurrentFunc(CalibrationZ);
    return;
  }
}


static void CalibrationZ()
{
  uint8_t Ret;

  if ( Event == EV_FUNC_FIRST )
  {
    LcdClear();
    ResetZm();
    return;
  }

  if ((Event & EV_MASK) == EV_KEY_PRESSED) 
  {
    switch (Event & KEY_MASK)
    {
      case KEY_ADC:
          goto redraw;
      case KEY_ENTER:
        ResetZm();
        return;
          
      case KEY_UP:
      case KEY_DOWN:
        if ( ZmMeasCount < 24 )
          return;
        {
          Complex_t Result = ComplexDivToFloat(ZmSum, ZmMeasCount);
          const char* Pointer;
          uint16_t Index = CurrentDiapason + 17 * CurrentFreq;
          
          if (CalibrationMode == CALIBRATION_MODE_LOAD)
            Pointer = (const char*)&Calibration.Data.Z.stdm[Index];
          else
            Pointer = (const char*)&Calibration.Data.Z.osm[Index];

          if (OverloadFlag == 0) //Без перегрузки 
          {
            if (Calibration.InvalidFlag) // Полная калибровка
            {
              WriteFlash(&Result, (void*)Pointer, sizeof(Result));
            }
            else // О-Ш калибровка
            {
              if (CalibrationMode == CALIBRATION_MODE_OPEN)
              {
                Yopen = ComplexDivFloat(1.0f, CalculateZ(0) );
                OperativeCalibrationFlag = 1;
                CurrentFunc(MainForm);  // End of O/S calibration
              }
              else
              {
                Zshort = CalculateZ(0);
                CalibrationMode = CALIBRATION_MODE_OPEN;
                AutoDiapason = AUTO_DIAPASON_ON;
                CurrentFunc(OpenCalibrationStart);  // End of short calibration
              }
              return;
            }
          }
        }

        CurrentFreq++;
        if ( CurrentFreq > FRENQ50K ) // All frenq
        {
          CurrentDiapason++;
          SetDiapason();
          CurrentFreq = FRENQ100;
          if ( CalibrationMode == CALIBRATION_MODE_LOAD)
          {
            CurrentFunc(LoadCalibrationStart);
            return;
          }
        }
        SetTheFrenq(CurrentFreq);

        if ( CurrentDiapason > 5 && CalibrationMode == CALIBRATION_MODE_SHORT )
        {
          CurrentFunc(OpenCalibrationStart);
          return;
        }

        if ( CurrentDiapason > 16 && CalibrationMode == CALIBRATION_MODE_OPEN )
        {
          CurrentFunc(LoadCalibrationStart);
          CurrentDiapason = 0;
          SetDiapason();
          return;
        }
    }
  }
  return;

redraw:
  Ret = CalculationZm();
  switch ( Ret )
  {
    case 0: // Подсчитано новое значение
    {
    float Av;

    if (Calibration.InvalidFlag == 0)
    {
      ShowResult(CalculateZ(0));
      return;
    }

    Av = ZmSum.Re/ZmMeasCount; // Расчет среднего действит части
    OutFloat(X_POSITION*0+Y_POSITION*2+MUL2, Av , ' '); // вывод лействит части
#if defined(MSE)
    Av = GetPercent(Av, SqZmSumReal);
    OutFloat(X_POSITION*5+Y_POSITION*4, Av , '%'); // Вывод относительного отклонения действит значения
#endif

    Av = ZmSum.Im/ZmMeasCount;  // Расчет мнимого среднего
    OutFloat(X_POSITION*0+Y_POSITION*5+MUL2, Av , ' '); // Вывод мнимой части
#if defined(MSE)
    Av = GetPercent(Av, SqZmSumImag);
    OutFloat(X_POSITION*5+Y_POSITION*7, Av , '%'); // Вывод относит отлония мнимой части
#endif
    }

  }
  ShowDiapason(); //Диапазон измерения
  ShowFrenq();
  ShowOverload(); // Перегрузка
}


