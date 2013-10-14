#include "display_float.h"


/* Buffer for 9 chars */
void DisplayFloat(char* DisplayBuf, float In)
{
  int16_t e    = 0;

  if (In < 0 )
  {
    In = In * -1;
    *DisplayBuf = '-';
  }
  else
    *DisplayBuf = ' ';;
  DisplayBuf++;

  if ( In != 0 )
  {
    /* Get exponent for 10 */
    while ( In > 10.0 )  /* More then 10 - increase exponent */
    {
      In = In / 10.0;
      e++;
    }

    while ( In < 1.0 )  /* Less then 1 - decrease exponent */
    {
      In = In * 10.0;
      e--;
    }
    /* After this 10 < In >= 1  */
  }

  {
    int16_t OutLen = 4;
    char Flags = 0;
 
    while ( OutLen )  /* Out mantissa */
    {
      char Digit = In;

//      if (Digit == 0 )
        //*DisplayBuf = ' ';
//      else
        *DisplayBuf = Digit + '0';
      DisplayBuf++;

#define ALREADYDOTED 2
      if ( (Flags & ALREADYDOTED) == 0)
      {
        if ( e%3 == 0 ) /* Engeneering format */
        {
          Flags |= ALREADYDOTED;
          *DisplayBuf = '.'; 
          DisplayBuf++;
        }
        else
          e--;
      }
    
      In = In - (float)(Digit); /* remove already outed part */

      In = In * 10.0;                 /* Next decimal digit */
      OutLen --;
    }
  }

  {
    /* Out sign of the exponent at next pos*/
    if ( e < 0 )
    {
      *DisplayBuf++ = '-'; 
      e = e * -1;
    }
    else
    {
      *DisplayBuf++ = ' '; // space
    }

    /* Out 2 digits of Exponent at 9 and 10 pos*/
    {
      char Dig = e/10;
      *DisplayBuf++ = Dig + '0';
    }

    {
      char Dig = e%10;
      *DisplayBuf++ = Dig+'0';
    }
  }
}
