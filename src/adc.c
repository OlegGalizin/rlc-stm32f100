#include <stdlib.h>
#include "adc.h"
#include "gpio.h"
#include "io.h"
#include "event.h"
#include "calcz.h"
#include "main.h"
#include "n1202.h"

#include "adc_handlers.inc"

int16_t ADCResults[ARRAY_SIZE];
int16_t DACInput[ARRAY_SIZE];
uint8_t CurrentFreq;
int8_t CurrentDiapason; // Текущий диапазон измерения

const FrenqSetup_t FrenqSetup[5] = {
// Init function                    DacPrescaler, AdcPrescaler,   DacSource, 
  {InitForCalcualtingWithDacFill,    149,          149,           DACInput,   
//DacSize                                  SinCosSize                                    Sin          SinCos
  SLOW_ARRAY_SIZE                           ,sizeof(SinArray100)/sizeof(SinArray100[0]), SinArray100, NULL,
  SLOW_ARRAY_SIZE, 32 }, // 100 Hz

// Init function                    DacPrescaler, AdcPrescaler,   DacSource, 
  {InitForCalcualtingWithDacFill,    149,          149,           DACInput,   
//DacSize                                  SinCosSize                                      Sin CosSinArray
  SLOW_ARRAY_SIZE                           ,sizeof(SinArray1)/sizeof(SinArray1[0]), SinArray1, NULL,
  SLOW_ARRAY_SIZE, 32 }, // 1KHz

// Init function                    DacPrescaler, AdcPrescaler,   DacSource, 
  {InitForCalcualtingWithoutDacFill, 43,            87,          (int16_t*)&DacSinArray10[0],   
//DacSize                                  SinCosSize                                      Sin      CosSin
  sizeof(DacSinArray10)/sizeof(DacSinArray10[0]), sizeof(SinCosArray10)/sizeof(SinCosArray10[0]), NULL, SinCosArray10,
  ADC_ARRAY_SIZE, 55 }, // 9.917 KHz

// Init function                    DacPrescaler, AdcPrescaler,   DacSource, 
  {InitForCalcualtingWithoutDacFill, 43,            87,          (int16_t*)&DacSinArray50[0],   
//DacSize                                  SinCosSize                                            Sin   CosSin
  sizeof(DacSinArray50)/sizeof(DacSinArray50[0]),sizeof(SinCosArray50)/sizeof(SinCosArray50[0]), NULL, SinCosArray50,
  ADC_ARRAY_SIZE, 55 }, // 49.587kHz

// Init function                    DacPrescaler, AdcPrescaler,   DacSource, 
  {InitForCalcualtingWithoutDacFill, 43,            87,          (int16_t*)&DacArrayDC,   
//DacSize                                  SinCosSize                                      Sin   SinCos
  sizeof(DacArrayDC)/sizeof(DacArrayDC/*[0]*/),sizeof(SinCosArray50)/sizeof(SinCosArray50[0]), NULL, &SinCosArray50[0],
  ADC_ARRAY_SIZE, 55 } // 0 Hz
}; /* DC - 4, 100Hz - 0, 1kHz - 1, 10kHz - 2, 50kHz - 3 */


void SetTheFrenq(uint8_t Frenq)
{
  const FrenqSetup_t* FSetup = &FrenqSetup[Frenq];
  LcdInit();
  CurrentFreq = Frenq;

  //Reset Tim3 & tim2
  RCC->APB1RSTR |=  (RCC_APB1RSTR_TIM2RST | RCC_APB1RSTR_TIM3RST);
  RCC->APB1RSTR &=  ~(RCC_APB1RSTR_TIM2RST | RCC_APB1RSTR_TIM3RST);

  FSetup->InitFunc(FSetup);/* Init finction for the frenq */
  NVIC_EnableIRQ(DMA1_Channel3_IRQn); /* Enable DMA interrupt */

  /* Reset ADC1 */
  ADC1->CR2 &= ~ADC_CR2_ADON;

  ADC1->CR2 = ADC_CR2_TSVREFE|ADC_CR2_EXTTRIG|ADC_CR2_EXTSEL_2; /* External trigger by T3 TRGO, DMA. reverence voltage on */
  ADC1->CR2 |= ADC_CR2_ADON; /* On the ADC */

// Calibration 
  ADC1->CR2 |= ADC_CR2_RSTCAL;
  while ( ADC1->CR2 & ADC_CR2_RSTCAL )
    ; /* BLANK */
  ADC1->CR2 |= ADC_CR2_CAL;
  while ( ADC1->CR2 & ADC_CR2_CAL )
    ; /* BLANK */

  /* ADC DMA chennel */
  DMA1_Channel3->CCR = DMA_CCR1_PL_1|DMA_CCR1_MSIZE_0|DMA_CCR1_PSIZE_0| /*Hight pry, 16 bit mem, 16 bit pereph */
          DMA_CCR1_MINC|DMA_CCR1_CIRC|DMA_CCR1_HTIE|DMA_CCR1_TCIE; /*  mem inc, circular, enterrupts by Half and End of conv */
  DMA1_Channel3->CNDTR = FSetup->OptimalADCSize;
  DMA1_Channel3->CPAR = (uint32_t)&ADC1->DR;
  DMA1_Channel3->CMAR = (uint32_t)&ADCResults[0];

  /* DAC DMA Channel */
  DMA1_Channel4->CCR = DMA_CCR3_PL|DMA_CCR3_MSIZE_0|DMA_CCR3_PSIZE_0| /* VEry Hight pry, 16 bit mem, 16 bit pereph */
          DMA_CCR3_MINC|DMA_CCR3_CIRC| DMA_CCR3_DIR; /*  mem inc, circular, from memory*/
  DMA1_Channel4->CNDTR = FSetup->DacSize; /* Data Size */
  DMA1_Channel4->CPAR = (uint32_t)&DAC->DHR12R2; /* Data holding register for 2 DAC channel */
  DMA1_Channel4->CMAR = (uint32_t)DACInput;

  DMA1_Channel3->CCR |= DMA_CCR1_EN; /* Enable DMA */
  DMA1_Channel4->CCR |= DMA_CCR3_EN; /* Enable DMA */

  TIM3->ARR = FSetup->AdcPrescaler; /* Clock for ADC */
//  TIM3->CR1 |= TIM_CR1_CEN;    /* start TIM3 */  
  /* Tim3 is slave; tim2 is master. Tim3 starts from tim2  so it no need to be enabled */
  TIM3->SMCR = TIM_SMCR_TS_0 |  /* TIM2 is TRGI */
                      TIM_SMCR_SMS_1|TIM_SMCR_SMS_2 /* Slave Trigger mode - start counting */;
  TIM3->CR2 = TIM_CR2_MMS_1; /* UPDATE  as TRGO. It is used to start ADC*/
  
  TIM3->CCMR2 = TIM_CCMR2_OC4M_0 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2; /* pwm mode 2 inactive while TIMx_CNT<TIMx_CCR4 */ 
  TIM3->CCR4 = 81; /* It need 20 ADC tick (40 CPU tick) for main adc conversion. After that we run injected 1 conversion */
  TIM3->CCER = TIM_CCER_CC4E; /* On the PWM output */
  TIM3->DIER = TIM_DIER_CC4DE; /* DMA request */

  TIM2->ARR = FSetup->DacPrescaler; /* Clock for DAC */
  TIM2->CR2 = TIM_CR2_MMS_1; /* UPDATE  as TRGO. It user to start TIM3*/
  TIM2->DIER = TIM_DIER_UDE;
//  TIM2->CR1 |= TIM_CR1_CEN;    /* start TIM2. Tim3 will be started by this TRGOUT */  
  TIM2->SMCR = TIM_SMCR_SMS_1|TIM_SMCR_SMS_2;  /* Internal trigger 0 - TIM1 is TRGI waiting start */
  ResetZm();
  AdcDelay(5);
  OperativeCalibrationFlag = 0; // Предыдущая О-Ш калибровка недействительна
}



// 17 диапазонов устанавливают коэффициенты умножения и сопротивление в i-v преобразователе
// K1 - коэф предусилителя инструментального усилителя. 1 или 10
// K2 - коэф диф усилителя инструментального усилителя. 1 или 10 - инвертирован.
// K1 - коэф оконечного усилителя переменного тока. 4 или 12
// R_SWITCH - 2 бита определяют выбор резистора 
const Diapason_t Diapasons[18] = 
{
  {K1_SWITCH|K2_SWITCH|K3_SWITCH|R_100,R_100|I_SWITCH}, // 0 - K1V-10,K2V-10,K3V-12, R=100
  {K1_SWITCH|K2_SWITCH|R_100, R_100|I_SWITCH}, //1- K1V-10,K2V-10, R=100
  {K1_SWITCH|K3_SWITCH|R_100, R_100|I_SWITCH}, //2- K1V-10,K3V-12, R=100
  {K1_SWITCH|R_100, R_100|I_SWITCH}, //3- K1V-10, R=100
  {K3_SWITCH|R_100, R_100|I_SWITCH}, //4 - K3V-12, R=100
  {R_100,R_100|I_SWITCH},// 5 - R=100
  {R_100,K3_SWITCH|R_100|I_SWITCH}, // 6 - K3I = 12, R =100
  {R_1K, R_1K|I_SWITCH}, // 7 - R=1K
  {R_1K, K3_SWITCH|R_1K|I_SWITCH}, // 8 -  K3I=12, R=1K
  {R_10K,R_10K|I_SWITCH}, // 9 - R = 10K
  {R_10K, R_10K|K3_SWITCH|I_SWITCH}, // 10 - K3I = 12, R=10K
  {R_100K, R_100K|I_SWITCH}, //11 - R=100K
  {R_100K, R_100K|K3_SWITCH|I_SWITCH}, //12 - K3I = 12, R=100K
  {R_100K, R_100K|K1_SWITCH|I_SWITCH}, //13- K1I=10, R=100K
  {R_100K, R_100K|K1_SWITCH|K3_SWITCH|I_SWITCH}, //14 - K1I=10, K2I=12, R=100K
  {R_100K, R_100K|K1_SWITCH|K2_SWITCH|I_SWITCH}, //15 - K1I=10, K2I=10, R=100K
  {R_100K, R_100K|K1_SWITCH|K2_SWITCH|K3_SWITCH|I_SWITCH}, //16 - K1I=10, K2I=10, K3I=12, R=100K
  {R_100|K1_SWITCH|K2_SWITCH, R_100|K1_SWITCH|K2_SWITCH|I_SWITCH} //17 - K1I=10, K2I=10, R=100 для калибровки
};


void SetSwitches(uint8_t Switches)
{
  GPIO_BB(I_V_MEAS) = ((I_SWITCH & Switches) != 0 ); // V measure
  GPIO_BB(K1) = ((Switches&K1_SWITCH) != 0 );
  GPIO_BB(K2) = ((Switches&K2_SWITCH) == 0 ); // Inverted !!!!
  GPIO_BB(K3) = ((Switches&K3_SWITCH) != 0 );
  GPIO_BB(IV1) = ((Switches&R_SWITCH1) != 0 );
  GPIO_BB(IV2) = ((Switches&R_SWITCH2) != 0 );

  AdcDelay(2);
}

void SystemInit(void)
{};

void Init(void)
{
  SCB->VTOR = FLASH_BASE;

  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |RCC_APB2ENR_IOPBEN |RCC_APB2ENR_AFIOEN|
                  RCC_APB2ENR_USART1EN|RCC_APB2ENR_TIM1EN|RCC_APB2ENR_ADC1EN|RCC_APB2ENR_TIM17EN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN|RCC_APB1ENR_TIM2EN|RCC_APB1ENR_TIM7EN|
                  RCC_APB1ENR_BKPEN|RCC_APB1ENR_PWREN|RCC_APB1ENR_DACEN; 
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;
   
  //Настройка портов ввода вывода
  GPIOB->ODR = 0; /* PB4 is pull up default !!!! */
  GPIOA->CRL = TO_GPIO_CRL(A, PWR_ON )|TO_GPIO_CRL(A, K3)|TO_GPIO_CRL(A, K2)|TO_GPIO_CRL(A, ADC )|
               TO_GPIO_CRL(A, INSTR_OUT)|TO_GPIO_CRL(A, DAC_OFF )|TO_GPIO_CRL(A, DAC_M  );
  GPIOA->CRH = TO_GPIO_CRH(A, LCD_CLK)|TO_GPIO_CRH(A, LCD_DA)|TO_GPIO_CRH(A, LCD_CE)|
               TO_GPIO_CRH(A, LCD_RESET)|TO_GPIO_CRH(A, LCD_LED);
  GPIOB->CRL = TO_GPIO_CRL(B, BUT1)|TO_GPIO_CRL(B, BUT2)|TO_GPIO_CRL(B, V_BAT)|
               TO_GPIO_CRL(B, PROBE1)|TO_GPIO_CRL(B, PROBE2)|TO_GPIO_CRL(B, IV1)|TO_GPIO_CRL(B, IV2);
  GPIOB->CRH = TO_GPIO_CRH(B, K1)|TO_GPIO_CRH(B, I_V_MEAS)|TO_GPIO_CRH(B, CP0)|TO_GPIO_CRH(B, CP1)|
               TO_GPIO_CRH(B, BUT3)|TO_GPIO_CRH(B, PWM);
  GPIOB->BSRR = TO_GPIO_BSRR(B, BUT1)|TO_GPIO_BSRR(B, BUT2)|
                TO_GPIO_BSRR(B, BUT3)|TO_GPIO_BSRR(B, PROBE1)|TO_GPIO_BSRR(B, PROBE2)|TO_GPIO_BSRR(B, K1);
  GPIOA->BSRR = TO_GPIO_BSRR(A, K2)|TO_GPIO_BSRR(A, PWR_ON);
  AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE; /* JTAG as GPIO only SWD */

  //Запуск кварца
  RCC->CR |= RCC_CR_HSEON;
  while((RCC->CR & RCC_CR_HSERDY) == 0)
    ; /* BLANK */

  //запуск PLL
  RCC->CFGR = RCC_CFGR_PLLXTPRE|RCC_CFGR_PLLMULL4|RCC_CFGR_PLLSRC /* 12 MHz/2*4 from prediv  = 24 MHz*/
    |RCC_CFGR_ADCPRE_DIV4; // 24/4=6Mhz ADC clock
  RCC->CR |= RCC_CR_PLLON;
  while((RCC->CR & RCC_CR_PLLRDY) == 0)
    ; /* BLANK */

  // переключение на PLL
  RCC->CFGR |= RCC_CFGR_SW_PLL;

  // Включение зарадового насоса 3.3 -> -6.6. Форма сигнала:
  //_|-|_|-|_|-|
  //___|---|___|
  TIM1->PSC = 99; //240kHz
  TIM1->ARR = 239; //1kHz
  TIM1->CCR1 = TIM1->CCR2 = 119;
  TIM1->CCMR1 = TIM_CCMR1_OC1M_1|TIM_CCMR1_OC1M_2|TIM_CCMR1_OC2M_1|TIM_CCMR1_OC2M_0; /* 1- PWM1, 2 - toggle*/
  TIM1->CCER = TIM_CCER_CC2NE|TIM_CCER_CC1NE; /* ENA 1NE 2NE out */
  TIM1->BDTR = TIM_BDTR_MOE;
  TIM1->CR1 |= TIM_CR1_CEN;
  TIM1->CR2 |= TIM_CR2_MMS_1; // update as out trigger


  /* On the DACs */
  DAC->CR &= ~(DAC_CR_DMAEN2 | DAC_CR_BOFF2 | DAC_CR_TEN2 | DAC_CR_TSEL2_2|DAC_CR_EN2);
  //2ЦАП без буфера тактируется от TIM2/TRG0 использует DMA
  DAC->CR = DAC_CR_DMAEN2 | DAC_CR_BOFF2 | DAC_CR_TEN2 | DAC_CR_TSEL2_2;
  if ((DAC->CR & DAC_CR_EN2) == 0 ) // Init the dac
  {
     DAC->DHR12R1 = 2048; // Some init value
     DAC->DHR12R2 = 2048; // Some init value
#if !defined(SIM)
     TIM17->ARR = 4095;
     TIM17->CCR1 = 2048;
     TIM17->CCMR1 = (TIM_CCMR1_OC1M_1|TIM_CCMR1_OC1M_2); // PWM mode 1
     TIM17->CCER = TIM_CCER_CC1E; /* Enable output */
     TIM17->BDTR = TIM_BDTR_MOE;
     TIM17->CR1 |= TIM_CR1_CEN;
#endif
  }
  DAC->CR |= (DAC_CR_EN1|DAC_CR_EN2); /* Enable the dac channel 2 for trigger and channel 1*/
}


#if defined(TEST_ADC)
#include "event.h"
#include "menu.h"
#include "n1202.h"
#include "itoa.h"
#include "display_float.h"
#if defined(TEST_CALCZ)
#include "calcz.h"
#endif

uint32_t HandlerTicks;

#if defined(TEST_CALIBR)
#include "calibration.h"
#endif

int64_t EndSinSumm = 0;
int64_t EndCosSumm = 0;
int64_t DisplaySinSumm = 0;
int64_t DisplayCosSumm = 0;

void TestMenu(void);
void HandlerTime(void);

void TestAdcMenu(void)
{
  if ( Event == EV_FUNC_FIRST )
  {
    MenuCounter = 6;
    LcdClear();
    goto redraw;
  }

  if ( (Event & EV_MASK) == EV_KEY_PRESSED ||
        (Event & EV_MASK) == EV_KEY_REPEATE)
  {
    switch (Event & KEY_MASK)
    {
      case KEY_DOWN:
        MenuCounter += 2;
      case KEY_UP:
        MenuCounter--;
        if (MenuCounter > 200 )
          MenuCounter = 6;
        if (MenuCounter > 6 )
          MenuCounter = 0;
        goto redraw;
      case KEY_ENTER:
        switch(MenuCounter)
        {
          case 0: /* Test */
            CurrentFunc(TestMenu);
            return;
          case 1: 
            goto redraw;
          case 2: /* Light */
            GPIO_TOGGLE(LCD_LED);
            return;
          case 3: /* Power off */
            GPIO_RESET(PWR_ON);
            return;
          case 4: /* Handler time */
#if defined(TEST_TICK)
            CurrentFunc(HandlerTime);
#endif
            return;
          case 5:/*calibration dc */
#if defined(TEST_CALIBR)
            CurrentFunc(CalibrationStart);
#endif
            return;
          case 6:
#if defined(TEST_CALCZ)
            CurrentFunc(CalculationZ);
#endif
            return;
        }
    }
  }
  return;
   
redraw:
  LcdChr(X_POSITION*0+Y_POSITION*0+16 + (0==MenuCounter)*INVERSE, "Test");
//  LcdChr(X_POSITION*0+Y_POSITION*1+16 + (1==MenuCounter)*INVERSE, "MSE");
  LcdChr(X_POSITION*0+Y_POSITION*2+16 + (2==MenuCounter)*INVERSE, "Light toggle");
  LcdChr(X_POSITION*0+Y_POSITION*3+16 + (3==MenuCounter)*INVERSE, "Power off");
#if defined(TEST_TICK)
  LcdChr(X_POSITION*0+Y_POSITION*4+16 + (4==MenuCounter)*INVERSE, "Ticks");
#endif
#if defined(TEST_CALIBR)
  LcdChr(X_POSITION*0+Y_POSITION*5+16 + (5==MenuCounter)*INVERSE, "CalibrationDC");
#endif
#if defined(TEST_CALCZ)
  LcdChr(X_POSITION*0+Y_POSITION*6+16 + (6==MenuCounter)*INVERSE, "CalculatZ");
#endif  
}


void TestMenu(void)
{
  if ( Event == 0 )
    return;

  if ( Event == EV_FUNC_FIRST )
  {
    MenuCounter = 8; 
    MenuMode = 0;
    LcdClear();
    goto redraw;
  }

  if ( (Event & EV_MASK) == EV_KEY_PRESSED || // нажата клавиша
        (Event & EV_MASK) == EV_KEY_REPEATE)  // автонажатие
  {
    uint16_t Inc = 0; 
    uint16_t Value;

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
          case 0: /* K1 */
            GPIO_TOGGLE(K1);
            goto redraw;
          case 1: /* K2 */
            GPIO_TOGGLE(K2);
            goto redraw;
          case 2: /* K3 */
            GPIO_TOGGLE(K3);
            goto redraw;
          case 3: /* IV Converter */
          {
            uint8_t CurrentValue = 0;

            if ( GPIO_ISSET(IV1) )
              CurrentValue = 1; /* First bit */
            if ( GPIO_ISSET(IV2) )
              CurrentValue |= 2; /* Second Bit */

            Value = CurrentValue + 1; /* Next value */
            Value &= 0x3;

            if ( ((CurrentValue & 0x1) != 0 ) ^ ( (Value & 0x1) != 0) ) /* Next value bit is not equel current */
              GPIO_TOGGLE(IV1);
            if ( ((CurrentValue & 0x2) != 0 ) ^ ((Value & 0x2) != 0 ) ) 
              GPIO_TOGGLE(IV2);
            goto redraw;
          }
          case 4: /* IV switch */
            GPIO_TOGGLE(I_V_MEAS);
            goto redraw;

          case 6: // I/V Diapason toggle
            if ( GPIO_ISSET(I_V_MEAS) )
              SetSwitches(Diapasons[CurrentDiapason].VV);
            else
              SetSwitches(Diapasons[CurrentDiapason].II);
            goto redraw;

          case 7: /* Back to main menu */
            CurrentFunc(TestAdcMenu);
            return;
          case 8: /* F */
          {
            CurrentFreq++;
            if ( CurrentFreq > FRENQ0)
              CurrentFreq = FRENQ100;
            SetAdcInput(MAIN_OUT);
            SetTheFrenq(CurrentFreq);
            goto redraw;
          }
          case 9: /* Adc connection */
          {
            if ( ADC1->SQR3 == MAIN_OUT ) /* Channel 2 - main out */
            {
              SetAdcInput(INST_OUT); //Измерение вых инстр ус
            }
            else 
            {
              SetAdcInput(MAIN_OUT);  //Измерение главного выхода
            }
            goto redraw;
          }
          case 10: // instrum dac
          case 11: // input offset dac
          case 5: // Diapason
            MenuMode = MenuMode ^ 1; // Blink
            goto redraw;
        }
        goto redraw;
      case KEY_ADC:
        AdcEvCounter++;
        EndSinSumm += ResultSinSum;
        EndCosSumm += ResultCosSum;
        if ( AdcEvCounter == 5 )
        {
          AdcEvCounter = 0;
          DisplaySinSumm = EndSinSumm;
          DisplayCosSumm = EndCosSumm;
          EndSinSumm = 0;
          EndCosSumm = 0;
          goto RedrawSumms;
        }
        return;
    } /* Switch */
    if ( (MenuMode & 0x1) == 0 ) // режим беганья по меню
    {
      MenuCounter -= Inc;
      if ( MenuCounter > 200)
        MenuCounter = 11;
      if (MenuCounter > 11 )
        MenuCounter = 0;
      goto redraw;
    }

    switch(MenuCounter) // Режим изменения текущего значения
    {
      case 10: // регулировка смещения инструментального усилителя
        Value = DAC->DHR12R1;        
        break;
      case 11: // коррекция входного сигнала по постоянному току
        Value = TIM17->CCR1;
        break;
      case 5: //  смена диапазона
        CurrentDiapason += Inc;
        if (CurrentDiapason  < 0 )
          CurrentDiapason = 0;
        if ( CurrentDiapason > 17 )
          CurrentDiapason = 17;
        if ( GPIO_ISSET(I_V_MEAS) )
          SetSwitches(Diapasons[CurrentDiapason].II);
        else
          SetSwitches(Diapasons[CurrentDiapason].VV);
        goto redraw;
    }
    Value = Value + Inc;
    if (Value > 4095)
      Value = Value - Inc; /* Enter-down Enter-up - decrease - increase value */
    if (Inc == 0 )
      Value = 2048; /* up-down - normal value */
    switch(MenuCounter)
    {
      case 10: // Установить новое значение DAC смещения инстр ус
        DAC->DHR12R1 = Value;
        break;
      case 11: // Установить новое значение PWM смещения входного сигнала
        TIM17->CCR1 = Value;
        break;
    }
    goto ADCRedraw;
  } /* If */
  return;
   
redraw:
  {
    char Buf[9];
    char* Out;


    /* K1*/
    if ( GPIO_ISSET(K1) )
      Out = "10";
    else
      Out = "1";
    LcdChr(X_POSITION*0+Y_POSITION*0+2 + (0==MenuCounter)*INVERSE, "1K");
    LcdChr(X_POSITION*2+Y_POSITION*0+2 + (0==MenuCounter)*INVERSE, Out);

    /* K2*/
    if ( !GPIO_ISSET(K2) )
      Out = "10";
    else
      Out = "1";
    LcdChr(X_POSITION*5+Y_POSITION*0+2 + (1==MenuCounter)*INVERSE, "2K");
    LcdChr(X_POSITION*7+Y_POSITION*0+2 + (1==MenuCounter)*INVERSE, Out);

    /* K3 */
    if ( GPIO_ISSET(K3) )
      Out = "12";
    else
      Out = "4";
    LcdChr(X_POSITION*10+Y_POSITION*0+2 + (2==MenuCounter)*INVERSE, "3K");
    LcdChr(X_POSITION*12+Y_POSITION*0+2 + (2==MenuCounter)*INVERSE, Out);

    /* I/V converter */
    if ( GPIO_ISSET(IV1) )
    {
      if ( GPIO_ISSET(IV2) )
        Out = "100K";
      else
        Out = "1K";
    }
    else
    {
      if ( GPIO_ISSET(IV2) )
        Out = "10K";
      else
        Out = "100";
    }
    LcdChr(X_POSITION*0+Y_POSITION*1+1 + (3==MenuCounter)*INVERSE, "R");
    LcdChr(X_POSITION*1+Y_POSITION*1+4 + (3==MenuCounter)*INVERSE, Out);

    /* MUX */
    if ( GPIO_ISSET(I_V_MEAS) )
      Out = "I";
    else
      Out = "V";
    LcdChr(X_POSITION*6+Y_POSITION*1+2 + (4==MenuCounter)*INVERSE, Out);

    /* Diapason */
    itoa(Buf, CurrentDiapason);
    LcdChr(X_POSITION*8+Y_POSITION*1+2 + (5==MenuCounter)*INVERSE, Buf);
    LcdChr(X_POSITION*10+Y_POSITION*1+1 + (6==MenuCounter)*INVERSE, "T");

    LcdChr(X_POSITION*12+Y_POSITION*1+4 + (7==MenuCounter)*INVERSE, "Back");

    { // отображение частоты
      char* F;
      switch (CurrentFreq)
      {
        case FRENQ0  :
          F = "DC";
          break;
        case FRENQ100: 
          F = "100";
          break;
        case FRENQ1K : 
          F = "1K";
          break;
        case FRENQ10K: 
          F = "10K";
          break;
        case FRENQ50K:
          F = "50K";
          break;
        default:
          F = "?";
      }
      LcdChr(X_POSITION*0+Y_POSITION*2+2 + (8==MenuCounter)*INVERSE, "F:");
      LcdChr(X_POSITION*2+Y_POSITION*2+4 + (8==MenuCounter)*INVERSE, F);
    }

    if ( ADC1->SQR3 == MAIN_OUT ) /* Channel 2 - main out */
      LcdChr(X_POSITION*8+Y_POSITION*2+8 + (9==MenuCounter)*INVERSE, "MAIN_ADC");
    else
      LcdChr(X_POSITION*8+Y_POSITION*2+8 + (9==MenuCounter)*INVERSE, "INST_ADC");

#if PWRMEAS
    /* Power input voltage ADC value */
    LcdChr(X_POSITION*0+Y_POSITION*4+3 , "PWR");
    itoa(Buf, ResultPowerAdc);
    LcdChr(X_POSITION*3+Y_POSITION*4+4 , Buf);

    /* Reference ADC value */
    LcdChr(X_POSITION*8+Y_POSITION*4+3 , "REF");
    itoa(Buf, ResultRefAdc);
    LcdChr(X_POSITION*11+Y_POSITION*4+4 , Buf);
#endif 

ADCRedraw:
    /* MAIN DAC value */
    LcdChr(X_POSITION*0+Y_POSITION*5+2 , "DA");
    itoa(Buf,  DAC->DOR2);
    LcdChr(X_POSITION*2+Y_POSITION*5+4 , Buf);

    /* MAIN ADC value */
    LcdChr(X_POSITION*0+Y_POSITION*6+2 , "AD");
    itoa(Buf, ADC1->DR);
    LcdChr(X_POSITION*2+Y_POSITION*6+4 , Buf);

    return;
RedrawSumms:

    MenuMode = MenuMode ^ 0x2; // Обеспечивает режим мигания

    /* Dac смещения инструментального ус */
    LcdChr(X_POSITION*0+Y_POSITION*3+3 + (10==MenuCounter)*INVERSE, "IFS");
    itoa(Buf, DAC->DOR1);
    LcdChr(X_POSITION*3+Y_POSITION*3+4 + ((10==MenuCounter) && (MenuMode != 0x3))*INVERSE, Buf);
    
    /* PWM смещения выходного сигнала */
    LcdChr(X_POSITION*8+Y_POSITION*3+3 + (11==MenuCounter)*INVERSE, "OFS");
#if !defined(SIM)
    itoa(Buf, TIM17->CCR1);
#endif
    LcdChr(X_POSITION*12+Y_POSITION*3+4 + ((11==MenuCounter) &&(MenuMode != 0x3))*INVERSE, Buf);

    itoa(Buf, ResultMin + 2048);
    LcdChr(X_POSITION*0+Y_POSITION*7+4, Buf);
    itoa(Buf, ResultAverage+2048);
    LcdChr(X_POSITION*5+Y_POSITION*7+4, Buf);
    itoa(Buf, ResultMax + 2048);
    LcdChr(X_POSITION*11+Y_POSITION*7+4, Buf);
 
    DisplayFloat(Buf, DisplaySinSumm);
    LcdChr(X_POSITION*7+Y_POSITION*5+9, Buf);
    DisplayFloat(Buf, DisplayCosSumm);
    LcdChr(X_POSITION*7+Y_POSITION*6+9, Buf);
  }
}

#if defined(TEST_TICK)
void HandlerTime(void)
{
  if ( Event == EV_FUNC_FIRST )
  {
    LcdClear();
    SCB_DEMCR |= 0x01000000;
    DWT_CONTROL |= 1; // enable the counter
    DWT_CYCCNT = 0;
    HandlerTicks = 0;
    goto redraw;
  }

  if ( (Event & EV_MASK) == EV_KEY_PRESSED || // нажата клавиша
        (Event & EV_MASK) == EV_KEY_REPEATE)  // автонажатие
  {
    switch (Event & KEY_MASK)
    {
      case KEY_DOWN:
      case KEY_UP:
        DWT_CONTROL = 0;
        SCB_DEMCR = 0;
        CurrentFunc(TestAdcMenu);
        return;
      case KEY_ENTER:
       goto redraw;
    }
  }
return;

redraw:
  {
    char Buf[9];
    DisplayFloat(Buf, (float)DWT_CYCCNT);
    LcdChr(X_POSITION*0+Y_POSITION*1+9, Buf);
    DisplayFloat(Buf, (float)HandlerTicks);
    LcdChr(X_POSITION*0+Y_POSITION*3+9, Buf);
  }
}
#endif


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


  SetAdcInput(MAIN_OUT);
  SetTheFrenq(FRENQ1K);
  CurrentFunc(TestAdcMenu); 
   while(1)
   {
#if !defined(SIM)
   __WFI();
#endif
    EventCheck();
   }
}
#endif

