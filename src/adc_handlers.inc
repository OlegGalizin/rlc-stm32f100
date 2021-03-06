#include "event.h"
#include "adc.h"


typedef void (*CurrentHandler_t)(int16_t Cur);
typedef union{
  const int32_t* CosSin;
  const int16_t* Sin;
  } PCosSin_t;


static CurrentHandler_t CurrentHandler; // ������� ������ ������� ���������� �� ���������� ���
static uint16_t          CountHandlersIn10Hz; // ���������� ������� ����������� ��� ������������ ������� 10 ��� � ���
static uint16_t          CountHandlers; // ������� ���������� ������� �����������
static int16_t HMin; // ������� ����������� ��������
static int16_t HMax;
static int32_t HAverage; // ����� ��� �������� ��������
static int64_t HCosSum; // ����� ������������ �� ����� �������
static int64_t HSinSum; // ����� ������������ �� �������
static const int32_t* CurrentCosSinPointer; // ������ ��������� � ������� �������-���������
static PCosSin_t PCosSin;
static const int16_t* SinPointer;   // ������� �������� � ������� �������
static int8_t   SinDirection; // ����������� �������� �� ������� ������� CalcualtingWithDacFill
static int8_t   SinFactor;    // ���� ������      see CalcualtingWithDacFill
static const int16_t* CosPointer;   // ������� �������� � ������� ������� ��� ��������� ��������
static int8_t   CosDirection; // direction for cos +1 or -1
static int8_t   CosFactor;    // factor for cos +1 or -1
static int16_t  DmaHalfSize; // ������ �������� �������

static volatile int8_t   SkipCount; // ���������� �������, ������� ����� ��������� ������ ������� �� �������� ����������� ������. ����� ����� ����� ������������ ��������� ������ ������� ����� ����������

// � ���� ���������� ������������ ���������� ����� ��������� �������
uint16_t ResultPowerAdc; // �������
uint16_t ResultRefAdc;  // �����
int16_t ResultMin; // ����������� ���������� ��������
int16_t ResultMax; // ����������� ���������� ��������
int32_t ResultAverage; // ������� ���������� ��������
int64_t ResultCosSum; // ��������� ������������ ������������ ��������� �� ������� - �������� ����� ���������
int64_t ResultSinSum; // -//- �� ����� - ������ �����

// ������� ������������� ��������� � ���������� ������� ���
void InitForCalcualtingWithDacFill(const struct FrenqSetup_t* FSetup)
{
  int i;

  CurrentHandler = CalcualtingWithDacFill; // ������� ��������� � ���������� ������ ���

  CountHandlersIn10Hz = FSetup->OptimalCallCount; // ���������� ������� ����������� ��� ��������� ������� ������� � 10 ����
  CountHandlers = CountHandlersIn10Hz; // ������� ���-�� ������� �����������
  DmaHalfSize = SLOW_ARRAY_SIZE/2; // �������� ������� ������� ���������
  PCosSin.Sin = FSetup->Sin; // ������� �������
  SinPointer = FSetup->Sin; // ��������� �� ������ ������� �������
  CosPointer = FSetup->Sin + FSetup->RealTableSize-1; // ������� - ��� �� ����� �� ��������� �� �������� �������
  SinFactor  = -1; // ����� ������� ���� ���� ����� �������������
  SinDirection = -1;// ����� ����� ����������� ����� ������ ������
  CosFactor  = 1; // ���� ����� �������������
  CosDirection = 1; // ����������� ����� �� ����� � ������
  //C ������� �������� ������� ��� - ������� �������� ������
  for (i=0; i<ADC_ARRAY_SIZE;i++)
  {
    DACInput[i] = 2048; // ��� ����� ��������� ���������� ����������
  }
}

void InitForCalcualtingWithoutDacFill(const struct FrenqSetup_t* FSetup)
{
  int i;

  CurrentHandler = CalcualtingWithoutDacFill;
  CountHandlersIn10Hz = FSetup->OptimalCallCount;
  CountHandlers = CountHandlersIn10Hz;
  DmaHalfSize = ADC_ARRAY_SIZE/2; // ������ ��� ������� �������
  PCosSin.CosSin = FSetup->CosSin;
  CurrentCosSinPointer = PCosSin.CosSin;
  for ( i = 0; i < FSetup->DacSize; i++)
  {
    DACInput[i] = FSetup->DacSource[i]; // ��� �������� ������������ �� �����
  }
}

void AdcDelay(int8_t Count)
{
  __disable_irq();
  SkipCount = Count;
//  SkipCount = 15;
  EventQueue = 0;   // �� ����� ������ ������ �������� ������� ������� �� ��� ���� �������� ���������� ���
  CountHandlers = 0;
  __enable_irq();

}

void DMA1_Channel3_IRQHandler(void) /* Read ADC values */
{
  int16_t Cur;

#if defined(TEST_ADC)
  uint32_t StartTime = DWT_CYCCNT;
#endif

  if (DMA1->ISR & DMA_ISR_HTIF3 && /*   Half Transfer complete */
       (DMA1_Channel3->CCR & DMA_CCR3_HTIE) /* Half traisfer interrupt enabled */
      ) 
  {
    Cur = 0; // ������ ��� ��������� � �������
    DMA1_Channel3->CCR &= ~DMA_CCR3_HTIE; /* disable Half traisfer interrupt !!!! */
  } else if (DMA1->ISR & DMA_ISR_TCIF3 ) /* transfer complete */
  {
    DMA1_Channel3->CCR |= DMA_CCR3_HTIE; /* enable Half traisfer interrupt  */

    Cur = DmaHalfSize; // ������ �������� ������
  }
  else
  {
    DMA1->IFCR |= DMA_IFCR_CGIF3; /* Clear all interrupt flags */
    return;
  }
  DMA1->IFCR |= DMA_IFCR_CGIF3; /* Clear all interrupt flags */
  CurrentHandler(Cur); // ����� ����������� ��� ������� ������� � �������

  if (EvCounter != 0 ) // any key was pressed
  {
	if ( SkipCount == 0 ) // ������ ����� ������������ ������� �� ������� ������
	{
      CountHandlers = 0;
	  SkipCount = 2;  // ������� ���������� ��������� ������� 
	}
  }

  if (CountHandlers == 0) // ���������� ������� ����������� ���������� ��� ������������ �������
  {
    // Generate end convertion event
    if ( SkipCount == 0 ) // ������� �� ������������ ��� ��� ��� ���� ������������� DAC � � ADC ����������� �����
    {
      if ( EventQueue == 0) //  ������� ������� �� ������
        EventQueue = EV_KEY_PRESSED|KEY_ADC;
    }
    else
    {
      SkipCount--;
    }
    ResultMin = HMin; // ��������� ����������� ����������
    ResultMax = HMax;
    ResultAverage = HAverage/DmaHalfSize/CountHandlersIn10Hz; // ������ ��������
    ResultCosSum = HCosSum/256; // ���������� �������� �����
    ResultSinSum = HSinSum/256; // ���������� ������ �����
    HCosSum = 0; // ��������� ������� ���������
    HSinSum = 0;
    HAverage = 0;
    HMin = 4096;
    HMax = -4096;
    CountHandlers = CountHandlersIn10Hz;
    // ������� EV_KEY_PRESSED|KEY_ADC ������������
    // ���������� �  ResultMin ResultMax  ResultAverage - ��������� �� ����������� ���� - ���� ��� � ������� ��������
    // ResultCosSum  ResultSinSum - ����� ����������� �������� �� ����� - ��������������� �������� � ������ ������ ���������
  }
  CountHandlers--; // ���������� ������� - ������ �������������� �� 2 ������
#if defined(PWRMEAS)
  {
  	static uint32_t AdcInput;

  	if ( CountHandlers == 8 && SkipCount == 2 ) // ��������� ���������� ���������� 
  	{
      AdcInput = ADC1->SQR3; // save original input
      ADC1->SQR3 = REF_OUT;  // Measure reference voltage
    }
    if ( AdcInput != 0 )
    {
      switch (CountHandlers)
      {
        case 4: // ������� �� ��������� ��������� ���������
          ADC1->SQR3 =  AdcInput;
          AdcInput = 0;
          ResultPowerAdc = HAverage/DmaHalfSize + 2048;
          break;
        case 5: // ����� �������� ��������
        case 7: 
          HAverage = 0;
          break;
        case 6: // ������������ �� ��������� � �������� �������
          ADC1->SQR3 = PWR_OUT;
          ResultRefAdc = HAverage/DmaHalfSize+ 2048; // ���������� ���������� ��������� �����
          break;
      }
    }
  }
#endif
#if defined(TEST_ADC)
  HandlerTicks =  HandlerTicks + (DWT_CYCCNT - StartTime);
#endif

}


#pragma O3
#pragma Otime
// ���������� ������ ������ � ���������� ������� SIN. � ���������� ������� � ����� ����� ��������� �� �������� �������
void CalcualtingWithDacFill(int16_t Cur) /* About 60 ticks for every point */
{
  int16_t Result;
  int16_t CosSin;
  int32_t CosSumLocal = 0;
  int32_t SinSumLocal = 0;
  int32_t AverageSum=0;
  int16_t End;

  End = Cur + SLOW_ARRAY_SIZE/2; // �������� ��������� �� ������ � �������

  do
  {
    Result = ADCResults[Cur] - 2048; // ����� ������� ������
    CosSin = *CosPointer; // ����� ������� �������� �������� �� �������
    if (CosSin & (BEGIN_FLAG|END_FLAG)) // ������ � ��������� ����� � �������
    {
      CosDirection *= -1; // �������� �����������
      SinDirection *= -1;
      if ( CosSin & BEGIN_FLAG )
        CosFactor *= -1; // �������� ���� � �������� � ������ �������� �������
      else
        SinFactor *= -1; // ���� ������ �������� � ����� ��������
      CosSin &= ~(BEGIN_FLAG|END_FLAG);
    }
    CosSumLocal += CosSin * Result * CosFactor; // ������������ �� �������
    CosPointer += CosDirection; // ����������� ������� ������ ��� �����

    CosSin = *SinPointer; // ����� ������ �������� ������
    CosSin &=  ~(BEGIN_FLAG|END_FLAG);
    CosSin *= SinFactor; // ������ ����
    DACInput[Cur] = CosSin + 2048; // ������������ ����� ��� ������
    SinSumLocal += CosSin * Result; // ������������ �� �����
    SinPointer += SinDirection; // ��������� ������� ������ � �������

    AverageSum += Result; // ����� ������� ��� �������� ��������
    if (Result > HMax )
      HMax = Result; // ������� ��������
    if (Result < HMin)
      HMin = Result; // ������� �������
    Cur++; // ���������� � ������� ��������
  } while ( Cur < End);

  DMA1->IFCR |= DMA_IFCR_CGIF1; /* Clear all interrupt flags */
  
  HCosSum += CosSumLocal; // ��������� �������� ���� � ���������� ����������
  HSinSum += SinSumLocal;
  HAverage += AverageSum;
}

#pragma O3
#pragma Otime
// ��������� ��������. ������� ������� � ��������� ������ �� ����� �������
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

    CosSin = *CosSinPointerLocal; // ��������� ������� �������� �� ������� �������-���������
    if ( CosSin & 0x1 ) /* ������� ���������� �������� � �������*/
    {
      CosSinPointerLocal = PCosSin.CosSin - 1; /* ���������� ��������� ����� ������ ��������� ������� �������-��������� */
    }
    CosSinPointerLocal += 1; // �������� ��������� �� ��������� ������� � ������� �������-���������
    Result = ADCResults[Cur] - 2048; // ��������� ���������
    LocalCosSum += (CosSin>>17) * Result; // 15 ������� ��� - �������. ���������� �� ���������
    LocalSinSum += (CosSin<<15>>16) * Result;  // 15 ������� ���, ����� ���������� - �����. ����� ������������ ����!!! - 2 �������� ������
    AverageSum += Result; // ������� ��������������
    if (Result > HMax )
      HMax = Result; // ��������
    if (Result < HMin)
      HMin = Result; // �������
    Cur++; // ��������� ������
  } while ( Cur < End);

  DMA1->IFCR |= DMA_IFCR_CGIF1; /* Clear all interrupt flags */
  
  // ���������� ����������� � ���������� ����������
  HCosSum += LocalCosSum;
  HSinSum += LocalSinSum;
  HAverage += AverageSum;
  CurrentCosSinPointer = CosSinPointerLocal; // ���������� ��������� � ������� ���������-�������
}
