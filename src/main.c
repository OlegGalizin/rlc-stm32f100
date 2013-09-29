#include <math.h>
#include "n1202.h"
#include "event.h"
#include "adc.h"
#include "menu.h"
#include "calcz.h"
#include "main.h"
#include "itoa.h"
#include "out_float.h"

uint8_t EqualSchem;
void MainForm(void);

Complex_t MsePrev;
uint8_t LoadCalibrationFlag;
Complex_t Zshort;
Complex_t Yopen;

void ShowDiapason(void)
{
  char Buf[4];
  itoa(Buf, CurrentDiapason);
  LcdChr(X_POSITION*13+Y_POSITION*0+ 2 + (5==MenuCounter)*INVERSE, Buf);
}

void ShowOverload(void)
{
  char* Overload;

  if ( OverloadFlag )
    Overload = "!!!!!!!!";
  else
    Overload = "";
  LcdChr(X_POSITION*3+Y_POSITION*1 + 8 + (Overload[0]!=0)*INVERSE, Overload);
}


void ShowFrenq(void)
{ // отображение частоты
  char* F;
  switch (CurrentFreq)
  {
    case FRENQ100: 
      F = "100Hz";
      break;
    case FRENQ1K : 
      F = "1kHz";
      break;
    case FRENQ10K: 
      F = "10kHz";
      break;
    case FRENQ50K:
      F = "50kHz";
      break;
    default:
      F = "DC";
      break;
  }
  LcdChr(X_POSITION*3+Y_POSITION*0+ 5 + (2==MenuCounter)*INVERSE, F);
}


void ShowResult(Complex_t Z)
{
  float x;
  char c;

  if ( EqualSchem == 0 ) // serial
  {
    x = Z.Re;
  }
  else
  {
    // 1/re(1/z)
    Complex_t Arg = ComplexDivFloat(1.0f, Z);
    x = 1.0f/Arg.Re;
  }
  OutFloat(X_POSITION*0+Y_POSITION*2+MUL2, x , Omega);

  if ( EqualSchem == 0 ) // serial
  {
    x = Z.Im;
    if (x < 0) // capacitance
    {
      x = (-1.0f)/CFrenq[CurrentFreq]/x;
      c = 'F';
    }
    else
    {
      x = x/CFrenq[CurrentFreq];
      c = 'H';
    }
  }
  else // parallel
  {    
    Complex_t Arg = ComplexDivFloat(1.0f, Z);
    x = Arg.Im;
    if (x > 0) // capacitance
    {
      x = x / CFrenq[CurrentFreq];
      c = 'F';
    }
    else
    {
      x = (-1.0f)/x/CFrenq[CurrentFreq];
      c = 'H';
    }
  }
  OutFloat(X_POSITION*0+Y_POSITION*5+MUL2, x , c);
}

Complex_t CalculateZ(uint8_t NoPercent)
{
  Complex_t Z;
  Complex_t Zm;
#if defined(MSE)
  float Mse;
#endif
  uint16_t ArrayIndex;

  ArrayIndex = CurrentFreq * 17 + CurrentDiapason;

  Zm.Re = ZmSum.Re/ZmMeasCount;
  Zm.Im = ZmSum.Im/ZmMeasCount;

#if defined(MSE)
  if ( NoPercent == 0 )
  {
    Mse = GetPercent(Zm.Re, SqZmSumReal);
    if ( MsePrev.Re > Mse )
       MsePrev.Re = Mse;
    else
    {
      if ( Mse > 0.05f )
        ResetZm();
    }
    Mse = GetPercent(Zm.Im, SqZmSumImag);
    if ( MsePrev.Im > Mse )
       MsePrev.Im = Mse;
    else
    {
      if ( Mse > 0.05f )
        ResetZm();
    }
  }
#endif

  Z.Re = Calibration.Rstd[CurrentDiapason]*RMult(CurrentDiapason); // Z = Zstd

  if (CurrentDiapason <=5 ) // Последовательная схема при калибровке
  {
    Complex_t Tmp;
    const Complex_t* Zsm;
    const Complex_t* Zstdm;
 
    Z.Im = Calibration.ParasiticL * 1e-12f * CFrenq[CurrentFreq]; /* R + jOmegaL */
    
    // Zstd*(Zxm - Zsm)/(Zstdm-Zsm)
    Zsm = &Calibration.Data.Z.osm[ArrayIndex];
    Tmp.Re = Zm.Re - Zsm->Re;
    Tmp.Im = Zm.Im - Zsm->Im;

    Z = ComplexMul(Z, Tmp);
    Zstdm = &Calibration.Data.Z.stdm[ArrayIndex];
    Tmp.Re = Zstdm->Re - Zsm->Re;
    Tmp.Im = Zstdm->Im - Zsm->Im;
    Z = ComplexDiv(Z, Tmp);
  }
  else // parallel scheme
  {
    Complex_t Arg;
    const Complex_t* Zom;
    const Complex_t* Zstdm;

    Z.Re = 1.0f/Z.Re;
    Z.Im = Calibration.ParasiticC * 1e-15f * CFrenq[CurrentFreq]; /* Y + jOmegaC */
    Z = ComplexDivFloat(1.0f, Z);

    // Zstd (Zom - Zstdm)/(Zom-Zxm)/Zstdm*Zxm
    Zom = &Calibration.Data.Z.osm[ArrayIndex];
    Zstdm = &Calibration.Data.Z.stdm[ArrayIndex];
    Arg.Re = Zom->Re - Zstdm->Re;
    Arg.Im = Zom->Im - Zstdm->Im;
    
    Z = ComplexMul(Z, Arg);
    
    Arg.Re = Zom->Re - Zm.Re;
    Arg.Im = Zom->Im - Zm.Im;
    Z = ComplexDiv(Z, Arg);
    Z = ComplexMul(Z, Zm);
    Z = ComplexDiv(Z, *Zstdm);
  }
  return Z;
}


#if 0
void MainMenu(void)
{
  if ( Event == EV_FUNC_FIRST )
  {
    LcdClear();
    MenuCounter = 0;
    return;
  }
  if ( (Event & EV_MASK) == EV_KEY_PRESSED )
  {
    uint16_t Inc = 0; 

    switch (Event & KEY_MASK)
    {
      case KEY_DOWN:
        Inc = 0xFFFF;  // -1
        break;
      case KEY_UP:
        Inc = 1;
        break;
      case KEY_ENTER:
        switch(MenuCounter)
        {
          case 0:
            CurrentFunc(MainForm);
            return;
          case 1:
//            if ( Calibration.InvalidFlag != 0 )
              CurrentFunc(CalibrationStart);
            return;
        }
    } /* Switch */
    MenuCounter += Inc;
    if (MenuCounter > 10)
      MenuCounter = 1;
    if (MenuCounter > 1)
      MenuCounter = 0;
  } /*if */

  LcdChr(X_POSITION*0+Y_POSITION*0 + 16 + (0==MenuCounter)*INVERSE, "Back");
  {
    char* Text = "O/S calibr";
    if ( Calibration.InvalidFlag != 0 )
      Text = "Calibration";

    LcdChr(X_POSITION*0+Y_POSITION*1 + 16 + (1==MenuCounter)*INVERSE, Text);
  }
}
#endif

void MainForm(void)
{
  uint8_t Ret;
  Complex_t Z;

  if ( Event == 0 )
    return;

  if ( Event == EV_FUNC_FIRST )
  {
    LcdClear();
    MenuCounter = 0;
    MenuMode = 0;
    return;
  }

  if ( (Event & EV_MASK) == EV_KEY_PRESSED )
  {
    uint16_t Inc = 0; 

    switch (Event & KEY_MASK)
    {
      case KEY_DOWN:
        Inc = 0xFFFF;  // -1
        break;
      case KEY_UP:
        Inc = 1;
        break;
      case KEY_ENTER:
        switch(MenuCounter)
        {
          case 1: /* Parallel - serial */
            if ( EqualSchem == 0 )
              EqualSchem = SHUNT_SCHEM;
            else
              EqualSchem = 0;
            MenuCounter = 0;
            /* NO break !!! */
          case 0: /* Main form */
            ResetZm(); /* Start measure at 0 */
            goto redraw;
          case 4:
            CurrentFunc(CalibrationStart); // Калибровка
            return;
          case 5: /* diapason */
          case 2: /* frenq */
            if ( MenuMode == 0 )
              MenuMode = 1;
            else
            {
              MenuMode = 0;
              MenuCounter = 0;
            }  
            goto redraw;
          case 3: /* Light */
            GPIO_TOGGLE(LCD_LED);
            MenuCounter = 0;
            goto redraw;
          case 6: /* Auto-manual */
            if ( FixDiapason == 0)
              FixDiapason = 1;
            else
              FixDiapason = 0;
            MenuCounter = 0;
            goto redraw;
        }
        goto redraw;
      case KEY_ADC:
        goto Calc;
    } /* Switch */

    if (MenuMode == 0)
    {
      MenuCounter += Inc;
      if ( MenuCounter > 10 )
        MenuCounter = 6;
      if ( MenuCounter > 6 )
        MenuCounter = 0;
      goto redraw;
    }

    if (MenuCounter == 2) /* Frenq */
    {
      CurrentFreq += Inc;
      if ( CurrentFreq > FRENQ100 + 10 )
        CurrentFreq = FRENQ50K;
      if ( CurrentFreq > FRENQ50K)
        CurrentFreq = FRENQ100;
      SetTheFrenq(CurrentFreq);
      goto redraw;
    }

    if (MenuCounter == 5) /* Diapason */
    {
      FixDiapason = 1;
      CurrentDiapason += Inc;
      if ( CurrentDiapason > 18 )
        CurrentDiapason = 17;
      if ( CurrentDiapason > 17 )
        CurrentDiapason = 0;
      SetDiapason();
      goto redraw;
    }
  } /* If key pressed event */

  if ( (Event & EV_MASK) == EV_KEY_LONG )
  {
    switch (Event & KEY_MASK)
    {
      case KEY_ENTER:
        GPIO_RESET(PWR_ON);
        return;
    }
  }
  return;


   
redraw:
  {
    // Парраллельный или последовательный режим замещения
    {
      char* Mode;

      if ( EqualSchem == SHUNT_SCHEM)
        Mode = "==";
      else
        Mode = "--";
      LcdChr(X_POSITION*0+Y_POSITION*0 + 2 + (1==MenuCounter)*INVERSE, Mode);
    }


    ShowFrenq(); // отображение частоты

    // Подсветка
    LcdChr(X_POSITION*9+Y_POSITION*0 + 1 + (3==MenuCounter)*INVERSE, "*");

    //Калибровка
    LcdChr(X_POSITION*11+Y_POSITION*0 + 1 + (4==MenuCounter)*INVERSE, "C");
    
    ShowDiapason(); //Диапазон измерения

    //Автоматический - ручной выбор диапазона
    {
      char* ModeA;

      if ( FixDiapason )
        ModeA = "M";
      else
        ModeA = "A";
      LcdChr(X_POSITION*15+Y_POSITION*0 + 1 + (6==MenuCounter)*INVERSE, ModeA);
    }

    ShowOverload(); // Перегрузка

#if defined(PWRMEAS)
      // Renew power scale
    {
      uint16_t Voltage = ((uint32_t)ResultPowerAdc)*Calibration.PowerFactor/ResultRefAdc - 200;
#if 0
      uint16_t i;

      LcdGotoXY(0, 8);
      for (i = 0; i < 480; i += 5) // All 96 columns
      {
        uint8_t Str = 0;

        if ( i % 50 == 0 )
          Str += 0x2;
        if (i < Voltage)
          Str += 0x4+0x8;
        if ( i % 100 == 0 )
          Str += 0x1;
        LcdSend(Str, LCD_DATA|LCD_SET_CE|LCD_RESET_CE);
      }

#else
    itoa(Buf, ResultPowerAdc);
    LcdChr(X_POSITION*0+Y_POSITION*7+ 4, Buf);
    itoa(Buf, ResultRefAdc);
    LcdChr(X_POSITION*5+Y_POSITION*7+ 4, Buf);
#endif
    }
#endif
  }   
  return;

Calc:
  Ret = CalculationZm();

#if 0 // Отладочный вывод результата функции
  {
    extern uint8_t Step; //Шаг диапазона
    char Buf[5];
    itoa(Buf, Ret);
    LcdChr(X_POSITION*0+Y_POSITION*7+4, Buf);
    itoa(Buf, Step);
    LcdChr(X_POSITION*4+Y_POSITION*7+4, Buf);
  }
#endif

  if ( Ret != 0 )
    goto redraw;

  if ( Calibration.InvalidFlag != 0) // не проведена калибровка - не считать значения
    goto redraw;



  Z = CalculateZ(FixDiapason);  // В ручном режиме не сбрасываем автоматически измерения а всегда накапливаем

  // We have calculated Z in this place
  if (LoadCalibrationFlag)
  {
    Complex_t Tmp;

    Z.Re = Z.Re - Zshort.Re;
    Z.Im = Z.Im - Zshort.Im; /* Zm - Zs */
    Tmp = ComplexMul(Z, Yopen); /* (Zm-Zs)Yo */
    
    Tmp.Re = 1 - Tmp.Re; /* 1 - (Zm-Zs)/Yo */
    Tmp.Im = - Tmp.Im;

    Z = ComplexMul(Z, Tmp);
  }

  ShowResult(Z);

#if 0
  // Renew power scale
  {
    uint16_t Voltage = ((uint32_t)ResultPowerAdc)*Calibration.PowerFactor/ResultRefAdc;
    uint16_t i;

    LcdGotoXY(0, 7);
    for (i=0; i < 480; i++) // All 96 columns
    {
      uint8_t Str = 0x80;
      if ( i % 50 == 0 )
        Str += 0x20;
      if (i < Voltage)
        Str += 0x40;
      if ( i % 100 )
        Str += 0x10;
    }

    LcdSend(Str,LCD_DATA|LCD_SET_CE);
  }
#endif

  goto redraw;
}

static uint16_t WorkCounter;

void EventIdle(void)
{
  if (Event == 0)
    return;
  if (Event & KEY_MASK_SYS)
    WorkCounter=0;
  if (Event & KEY_ADC)
  {
    WorkCounter++;
    if ( WorkCounter > 10*60*10 )
        GPIO_RESET(PWR_ON); // Auto Power off     
  }
}

#if !defined(TEST_ADC)
void main()
{
  Init();

  //Зафиксировать питание включенным
  GPIO_SET(PWR_ON);
  //Включить подсветку
  GPIO_SET(LCD_LED);

  //Инициализация дисплея
  LcdInit();
  LcdClear();

  //Инициализайия событий кнопок
  EventInit();

  if (Calibration.InvalidFlag == 0 )
  {
    DAC->DHR12R1 = Calibration.InstrOffset;
    TIM17->CCR1 = Calibration.OutOffset;
  }
  SetAdcInput(MAIN_OUT);
  SetTheFrenq(FRENQ1K);
  SetDiapason();
  CurrentFunc(MainForm); 
   while(1)
   {
#if !defined(SIM)
   __WFI();
#endif
    EventCheck();
   }
}
#endif

