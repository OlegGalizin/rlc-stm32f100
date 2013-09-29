#include "n1202.h"

/*--------------------------------------------------------------------------------------------------
                                Private function prototypes
--------------------------------------------------------------------------------------------------*/
//  Function prototypes are mandatory otherwise the compiler generates unreliable code.

const uint8_t FontLookup [] =
{
     0x00, 0x00, 0x00, 0x00, 0x00 ,  // sp
     0x00, 0x00, 0x2f, 0x00, 0x00 ,   // !
     0x00, 0x07, 0x00, 0x07, 0x00 ,   // "
     0x14, 0x7f, 0x14, 0x7f, 0x14 ,   // #
     0x24, 0x2a, 0x7f, 0x2a, 0x12 ,   // $
     0xc4, 0xc8, 0x10, 0x26, 0x46 ,   // %
     0x36, 0x49, 0x55, 0x22, 0x50 ,   // &
     0x00, 0x05, 0x03, 0x00, 0x00 ,   // '
     0x00, 0x1c, 0x22, 0x41, 0x00 ,   // (
     0x00, 0x41, 0x22, 0x1c, 0x00 ,   // )
     0x14, 0x08, 0x3E, 0x08, 0x14 ,   // *
     0x08, 0x08, 0x3E, 0x08, 0x08 ,   // +
     0x00, 0x00, 0x50, 0x30, 0x00 ,   // ,
     0x10, 0x10, 0x10, 0x10, 0x10 ,   // -
     0x00, 0x60, 0x60, 0x00, 0x00 ,   // .
     0x20, 0x10, 0x08, 0x04, 0x02 ,   // /
     0x3E, 0x51, 0x49, 0x45, 0x3E ,   // 0
     0x00, 0x42, 0x7F, 0x40, 0x00 ,   // 1
     0x42, 0x61, 0x51, 0x49, 0x46 ,   // 2
     0x21, 0x41, 0x45, 0x4B, 0x31 ,   // 3
     0x18, 0x14, 0x12, 0x7F, 0x10 ,   // 4
     0x27, 0x45, 0x45, 0x45, 0x39 ,   // 5
     0x3C, 0x4A, 0x49, 0x49, 0x30 ,   // 6
     0x01, 0x71, 0x09, 0x05, 0x03 ,   // 7
     0x36, 0x49, 0x49, 0x49, 0x36 ,   // 8
     0x06, 0x49, 0x49, 0x29, 0x1E ,   // 9
     0x00, 0x36, 0x36, 0x00, 0x00 ,   // :
     0x00, 0x56, 0x36, 0x00, 0x00 ,   // ;
     0x08, 0x14, 0x22, 0x41, 0x00 ,   // <
     0x14, 0x14, 0x14, 0x14, 0x14 ,   // =
     0x00, 0x41, 0x22, 0x14, 0x08 ,   // >
     0x02, 0x01, 0x51, 0x09, 0x06 ,   // ?
//     0x32, 0x49, 0x59, 0x51, 0x3E ,   // @
     0x5E, 0x61, 0x01, 0x61, 0x5E ,   // Omega
     0x7E, 0x11, 0x11, 0x11, 0x7E ,   // A
     0x7F, 0x49, 0x49, 0x49, 0x36 ,   // B
     0x3E, 0x41, 0x41, 0x41, 0x22 ,   // C
     0x7F, 0x41, 0x41, 0x22, 0x1C ,   // D
     0x7F, 0x49, 0x49, 0x49, 0x41 ,   // E
     0x7F, 0x09, 0x09, 0x09, 0x01 ,   // F
     0x3E, 0x41, 0x49, 0x49, 0x7A ,   // G
     0x7F, 0x08, 0x08, 0x08, 0x7F ,   // H
     0x00, 0x41, 0x7F, 0x41, 0x00 ,   // I
     0x20, 0x40, 0x41, 0x3F, 0x01 ,   // J
     0x7F, 0x08, 0x14, 0x22, 0x41 ,   // K
     0x7F, 0x40, 0x40, 0x40, 0x40 ,   // L
     0x7F, 0x02, 0x0C, 0x02, 0x7F ,   // M
     0x7F, 0x04, 0x08, 0x10, 0x7F ,   // N
     0x3E, 0x41, 0x41, 0x41, 0x3E ,   // O
     0x7F, 0x09, 0x09, 0x09, 0x06 ,   // P
     0x3E, 0x41, 0x51, 0x21, 0x5E ,   // Q
     0x7F, 0x09, 0x19, 0x29, 0x46 ,   // R
     0x46, 0x49, 0x49, 0x49, 0x31 ,   // S
     0x01, 0x01, 0x7F, 0x01, 0x01 ,   // T
     0x3F, 0x40, 0x40, 0x40, 0x3F ,   // U
     0x1F, 0x20, 0x40, 0x20, 0x1F ,   // V
     0x3F, 0x40, 0x38, 0x40, 0x3F ,   // W
     0x63, 0x14, 0x08, 0x14, 0x63 ,   // X
     0x07, 0x08, 0x70, 0x08, 0x07 ,   // Y
     0x61, 0x51, 0x49, 0x45, 0x43 ,   // Z
     0x00, 0x7F, 0x41, 0x41, 0x00 ,   // [
     0x00, 0xFC, 0x20, 0x3C, 0x20 ,   // Mju
     0x00, 0x41, 0x41, 0x7F, 0x00 ,   // ]
     0x04, 0x02, 0x01, 0x02, 0x04 ,   // ^
     0x40, 0x40, 0x40, 0x40, 0x40 ,   // _
     0x00, 0x01, 0x02, 0x04, 0x00 ,   // '
     0x20, 0x54, 0x54, 0x54, 0x78 ,   // a
     0x7F, 0x48, 0x44, 0x44, 0x38 ,   // b
     0x38, 0x44, 0x44, 0x44, 0x20 ,   // c
     0x38, 0x44, 0x44, 0x48, 0x7F ,   // d
     0x38, 0x54, 0x54, 0x54, 0x18 ,   // e
     0x08, 0x7E, 0x09, 0x01, 0x02 ,   // f
     0x0C, 0x52, 0x52, 0x52, 0x3E ,   // g
     0x7F, 0x08, 0x04, 0x04, 0x78 ,   // h
     0x00, 0x44, 0x7D, 0x40, 0x00 ,   // i
     0x20, 0x40, 0x44, 0x3D, 0x00 ,   // j
     0x7F, 0x10, 0x28, 0x44, 0x00 ,   // k
     0x00, 0x41, 0x7F, 0x40, 0x00 ,   // l
     0x7C, 0x04, 0x18, 0x04, 0x78 ,   // m
     0x7C, 0x08, 0x04, 0x04, 0x78 ,   // n
     0x38, 0x44, 0x44, 0x44, 0x38 ,   // o
     0x7C, 0x14, 0x14, 0x14, 0x08 ,   // p
     0x08, 0x14, 0x14, 0x18, 0x7C ,   // q
     0x7C, 0x08, 0x04, 0x04, 0x08 ,   // r
     0x48, 0x54, 0x54, 0x54, 0x20 ,   // s
     0x04, 0x3F, 0x44, 0x40, 0x20 ,   // t
     0x3C, 0x40, 0x40, 0x20, 0x7C ,   // u
     0x1C, 0x20, 0x40, 0x20, 0x1C ,   // v
     0x3C, 0x40, 0x30, 0x40, 0x3C ,   // w
     0x44, 0x28, 0x10, 0x28, 0x44 ,   // x
     0x0C, 0x50, 0x50, 0x50, 0x3C ,   // y
     0x44, 0x64, 0x54, 0x4C, 0x44     // z
};



/*--------------------------------------------------------------------------------------------------

  Name         :  LcdInit

  Description  :  Performs PINS & LCD controller initialization.

  Argument(s)  :  None.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdInit ()
{
#if 0
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN
#if defined(LCD_USART)  
                  |RCC_APB2ENR_USART1EN
#endif /* LCD_USART */ 
  ; /* Port B Clock */

#if !defined(LCD_USART)  
  GPIOA->CRH &= ~(((GPIO_CRL_MODE0|GPIO_CRL_CNF0)<<(LCD_CE_PIN-8)*4) |
                  ((GPIO_CRL_MODE0|GPIO_CRL_CNF0)<<(LCD_IN_PIN-8)*4) |
                  ((GPIO_CRL_MODE0|GPIO_CRL_CNF0)<<(LCD_CLK_PIN-8)*4)
#if !defined(LCD_NO_RESET)
                  |((GPIO_CRL_MODE0|GPIO_CRL_CNF0)<<(LCD_RST_PIN-8)*4) // Reset all pins
#endif
                   );
  GPIOA->CRH |=  (((GPIO_CRL_MODE0_0)<<(LCD_CE_PIN-8)*4) |
                  ((GPIO_CRL_MODE0_0)<<(LCD_IN_PIN-8)*4) |
                  ((GPIO_CRL_MODE0_0)<<(LCD_CLK_PIN-8)*4)
#if !defined(LCD_NO_RESET)
                  |((GPIO_CRL_MODE0_0)<<(LCD_RST_PIN-8)*4) // Pins in push-pull 10MHz
#endif     /* LCD_NO_RESET */
                   );
#else /* LCD_USART */
  GPIOA->CRH &= ~(((GPIO_CRL_MODE0|GPIO_CRL_CNF0)<<(LCD_CE_PIN-8)*4) |
                  ((GPIO_CRL_MODE0|GPIO_CRL_CNF0)<<(LCD_IN_PIN-8)*4) |
                  ((GPIO_CRL_MODE0|GPIO_CRL_CNF0)<<(LCD_CLK_PIN-8)*4)
#if !defined(LCD_NO_RESET)
                  |((GPIO_CRL_MODE0|GPIO_CRL_CNF0)<<(LCD_RST_PIN-8)*4) // Reset all pins
#endif  /* LCD_NO_RESET */
                   );
  GPIOA->CRH |=  (((GPIO_CRL_MODE0_0)<<(LCD_CE_PIN-8)*4) | /* PUSH-PULL */
                  ((GPIO_CRL_MODE0_0|GPIO_CRL_CNF0_1)<<(LCD_IN_PIN-8)*4) | /* Alternate push pull */
                  ((GPIO_CRL_MODE0_0|GPIO_CRL_CNF0_1)<<(LCD_CLK_PIN-8)*4)  /* Alternate push-pull */
#if !defined(LCD_NO_RESET)
                  |((GPIO_CRL_MODE0_0)<<(LCD_RST_PIN-8)*4) // Pins in push-pull 10MHz
#endif  /* LCD_NO_RESET */
                   );
  LCD_CE_PIN_HIGHT;
  USART1->BRR = 0x0040; /* MAX speed */
  USART1->CR2 = USART_CR2_STOP_0|USART_CR2_CLKEN|USART_CR2_CPOL|USART_CR2_CPHA|USART_CR2_LBCL; /* 0.5 stop bit, pib CLK ena, polatity and phaze, clock fpr last bit */
  USART1->CR1 = USART_CR1_OVER8|USART_CR1_M|USART_CR1_TE|USART_CR1_UE; /* oversampling 8, 9 bit, transmitter ena, usart ena */
#endif /*LCD_USART */
#endif /* 0 */

#if defined(LCD_USART)
  LCD_CE_PIN_HIGHT;
  USART1->BRR = 0x0200; /* MAX speed */
  USART1->CR2 = USART_CR2_STOP_0|USART_CR2_CLKEN|USART_CR2_CPOL|USART_CR2_CPHA|USART_CR2_LBCL; /* 0.5 stop bit, pib CLK ena, polatity and phaze, clock fpr last bit */
  USART1->CR1 = USART_CR1_OVER8|USART_CR1_M|USART_CR1_TE|USART_CR1_UE; /* oversampling 8, 9 bit, transmitter ena, usart ena */
#endif /*LCD_USART */

#if !defined(LCD_NO_RESET)
  GPIO_RESET(LCD_RESET);
#endif
  LCD_BIG_DELAY();
#if !defined(LCD_NO_RESET)
  GPIO_SET(LCD_RESET);
#endif
  LCD_BIG_DELAY();
//  LcdSend( 0xE2 , LCD_COMMAND|LCD_SET_CE|LCD_RESET_CE );  // Soft reset
//  LCD_BIG_DELAY();
  LcdSend( 0x3D , LCD_COMMAND|LCD_SET_CE );  // Charge pump
  LcdSend( 0x01 , LCD_COMMAND|LCD_RESET_CE);  // Charge pump = 4 (default 5 is too hight for 3.0 volt)
  LCD_BIG_DELAY();
  LcdSend( 0xA4,  LCD_COMMAND|LCD_SET_CE|LCD_RESET_CE );  // Power save OFF
  LCD_BIG_DELAY();
  LcdSend( 0x2F,  LCD_COMMAND|LCD_SET_CE|LCD_RESET_CE );  // Booster ON Voltage regulator ON Voltage follover ON
  LCD_BIG_DELAY();
  LcdSend( 0xAF,  LCD_COMMAND|LCD_SET_CE|LCD_RESET_CE );  // LCD on
  LCD_BIG_DELAY();
  LcdSend( 0xC8,  LCD_COMMAND|LCD_SET_CE|LCD_RESET_CE );  // reverse direction
  LCD_BIG_DELAY();
  LcdSend( 0xA1,  LCD_COMMAND|LCD_SET_CE|LCD_RESET_CE );  // reverse direction
  LCD_BIG_DELAY();

//  LcdSend( 0xE1 , LCD_COMMAND|LCD_SET_CE);  // Additional VOP for contrast increase
//  LcdSend( 0x20 , LCD_COMMAND|LCD_RESET_CE);  // from -127 to +127
//  LCD_BIG_DELAY();
}

//Clear display
void LcdClear()
{
	uint16_t j = 9;

	while ( j-- )
	{
    uint16_t i = 96;

    LcdGotoXY(0,j);

    while ( i-- )
    {
      LcdSend(0x00, LCD_DATA|LCD_SET_CE|LCD_RESET_CE);
    }  
	}
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdContrast

  Description  :  Set display contrast.

  Argument(s)  :  contrast -> Contrast value from 0x00 to 0x7F.

  Return value :  None.

  Notes        :  No change visible at ambient temperature.

--------------------------------------------------------------------------------------------------*/
void LcdContrast ( uint8_t Contrast ) /* 0 - 32 */
{
  LcdSend( 0x80|(Contrast&0x01F), LCD_COMMAND|LCD_SET_CE|LCD_RESET_CE );  // Electronic Volume
}

void LcdGotoXY(uint8_t X, uint8_t Y)
{
  LcdSend(0x10|(X>>4), LCD_COMMAND|LCD_SET_CE ); 
  LcdSend(0x00|(X&0x0F), 0);
  LcdSend(0xB0|Y, 0);
}


/*--------------------------------------------------------------------------------------------------

  Name         :  LcdChr

  Description  :  Displays a character at current cursor location and increment cursor location.

  Argument(s)  :  Ctrl = X*X_MUL+y*Y_MUL+ BIG1 + BIG2 + OutLen
                  NoInverse == 0 - inverse output
                  Str - out string

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
#if defined(BIG)  
static uint8_t AllBitsArray[3][4][5] = {
    {{0,1,2,3,0xFF},{4, 5, 6, 7, 0xFF}},
    {{0,1,2,0xFF},{2,3,4,5,0xFF}, {5,6,7,0xFF}},
    {{0,1,0xFF},{2,3,0xFF}, {4,5,0xFF}, {6,7, 0xFF}} };
#endif  /* BIG */

void LcdChr ( uint32_t Ctrl, const char* InStr )
{
  unsigned char ch;
  uint8_t Inverse;
#if defined(BIG)
  uint8_t Mul = (Ctrl>>30) + 1;
  uint8_t CurrentString;
#endif
  uint8_t X = (Ctrl/X_OFFSET)& 0xFF;
  uint8_t Y = (Ctrl/Y_POSITION) & 0x07;
  uint16_t Len;
  Inverse = ((Ctrl & INVERSE) != 0);

#if defined(BIG)  
  for (CurrentString = 0; CurrentString < Mul; CurrentString++)
#endif
  {
    const char* Str = InStr;

    LcdGotoXY(X,Y
#if defined(BIG)
                +CurrentString
#endif
              );
    Len = Ctrl & 0x03FF; /* Only Len - 10 bit */
    while ( Len != 0 )
    {
      const uint8_t* StartCh;
      int j;
      
      Len--;

      ch = *Str;
      if ( ch != 0)
        Str++;
      else
        ch = ' ';

      if ( (ch < 0x20) || (ch > 0x7b) )
      {
          //  Convert to a printable character.
          ch = 92;
      }
      ch = ch - 0x20;
  
      StartCh = &FontLookup[ch*5];
      for ( j = 0; j < 6; j++)
      {
        ch = *StartCh;
        StartCh++;
        if (j==5)
          ch = 0;

#if defined(BIG)
        if ( Mul == 1 )
#endif
        {
          if ( Inverse )
          {
            ch = ~ch;
          }
          LcdSend( ch, LCD_DATA );
        }
#if defined(BIG)
        else
        {
          uint32_t Out = 0;
          uint8_t i;
          uint8_t* BitsArray = AllBitsArray[Mul-2][CurrentString];
          uint8_t Bit;
  
          while ( Bit = *BitsArray, Bit != 0xFF)
          {
            for (i=0; i<Mul; i++)
            {
              Out |= (((uint32_t)ch) & (1<<Bit))<<(Bit*(Mul-1)+i);
            }
            BitsArray++;
          }
          Out = Out >> (CurrentString*8);
          
          if ( Inverse )
          {
            Out = ~Out;
          }
          for (i = 0; i < Mul; i++)
            LcdSend( Out, LCD_DATA );
        }
#endif /* BIG */
      }
    } /* Len != 0 */
  }
  LCD_CE_PIN_HIGHT;
}


/*--------------------------------------------------------------------------------------------------

  Name         :  LcdSend

  Description  :  Sends data to display controller.

  Argument(s)  :  data -> Data to be sent
                  cd   -> Command or data (see/use enum)

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdSend ( uint8_t Data, uint8_t Flags )
{
#if !defined(LCD_USART)  
  int i;
#endif
  uint32_t TmpData = Data;


  if ( (Flags & LCD_SET_CE) != 0)
  {
    LCD_CE_PIN_LOW;
  }

  if ( Flags & LCD_DATA ) /* If data */
  {
    TmpData |= 0x100;
  }
#if !defined(LCD_USART)  
  i=9;
  do
  {
    if ( TmpData & 0x100 )
      GPIO_SET(LCD_DA); /* IN pin to hight */
    else
      GPIO_RESET(LCD_DA); /* IN pin to low */
    LCD_DELAY();
    TmpData = TmpData<<1; /* Some additional delay */
    GPIO_SET(LCD_CLK); /* CLK pin to hight */
    LCD_DELAY();
    i--;
    GPIO_RESET(LCD_CLK); /* CLK pin to low */
  }while(i);
#else /* LCD_USART */
  TmpData = __RBIT(TmpData) >> 23;
  while ( (USART1->SR & USART_SR_TXE) == 0 ) /* Transmitter registar is not empty */
    ; /* BLANK */
  USART1->DR = TmpData;
#endif /* LCD_USART */
  if ( Flags & LCD_RESET_CE)
  {
    LCD_CE_PIN_HIGHT;
  }
}

/*--------------------------------------------------------------------------------------------------
                                     Character generator

             This table defines the standard ASCII characters in a 5x7 dot format.
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/



#if defined(LCDDEBUG)
void SystemInit()
{
}

int main(void)
{
  volatile unsigned i;
  int j;

  RCC->CFGR |= (RCC_CFGR_HPRE_1|RCC_CFGR_HPRE_3); /* div 8 */
  LcdInit();
  LcdClear();
  LcdChr ( Y_POSITION*0+X_POSITION*1+13, "Hello world" );
  LcdChr ( Y_POSITION*1+X_POSITION*1+13+INVERSE+X_OFFSET*3, "Hello world" );
  LcdChr ( Y_POSITION*2+X_POSITION*0+2, "12" );  
  LcdChr ( Y_POSITION*2+X_POSITION*3+2+MUL2, "34" );  
  LcdChr ( Y_POSITION*2+X_POSITION*8+2+MUL3, "56" );  
  LcdChr ( Y_POSITION*4+X_POSITION*0+2+MUL4, "78" );

  for (i=0; i != 100000; i++)
    ;
  LcdChr ( Y_POSITION*6+X_POSITION*8+8, "Volume" );  
  for (j = 0; j < 64; j++)
  {
    char Str[12] = {0};
    int k = j;
    for (i=0; i != 100000; i++)
      ;
    LcdContrast(j);
    Str[0] = k/10000 + '0';
    k = k%10000;
    Str[1] = k/1000 + '0';
    k = k%1000;
    Str[2] = k/100 + '0';
    k = k%100;
    Str[3] = j/10 + '0';
    k = k%10;
    Str[4] = k + '0';

    LcdChr ( Y_POSITION*7+X_POSITION*8+6, Str );  
  }

  LcdChr ( Y_POSITION*6+X_POSITION*6+10, "V0-Range = 25" );  
  LcdContrast(0);

  LcdSend(0x25, 0);
  for (j = 0; j < 64; j++)
  {
    char Str[12] = {0};
    int k = j;
    for (i=0; i != 100000; i++)
      ;
    LcdContrast(j);
    Str[0] = k/10000 + '0';
    k = k%10000;
    Str[1] = k/1000 + '0';
    k = k%1000;
    Str[2] = k/100 + '0';
    k = k%100;
    Str[3] = j/10 + '0';
    k = k%10;
    Str[4] = k + '0';

    LcdChr ( Y_POSITION*7+X_POSITION*8+6, Str );  
  }
  LcdContrast(16); //Default



  return 0;
}
#endif
