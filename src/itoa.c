#include <string.h>
#include "itoa.h"
char* itoa(char* Buf, int16_t in)
{
  char c;
  int Div = 1000;
  int i;
  int j;

  memset(Buf, 0, 4);
  Buf[0] = '0';
  if ( in > 9999  || in < 0 )
  {
    memset(Buf, '*', 4);
    return Buf;
  }
  j=0;
  for(i = 0; i < 4; i++ )
  {
    c = in/Div;
    if (c != 0 || j != 0 )
    {
      c = c + '0';
      Buf[j] = c;
      j++;
    }
    in = in % Div;
    Div = Div/10;
  }
  return Buf;
}
