#include "compl.h"


Complex_t ComplexDivToFloat(Complex_t Arg1, float Arg2)
{
  Complex_t Res;
  
  Res.Re = Arg1.Re/Arg2;
  Res.Im = Arg1.Im/Arg2;
  return Res;
}

Complex_t ComplexDivFloat(float Arg1, Complex_t Arg2)
{
  Complex_t Ret;
  float Module2 = Arg2.Re*Arg2.Re + Arg2.Im*Arg2.Im;
  Ret.Re = Arg1 * Arg2.Re / Module2;
  Ret.Im = (-Arg1) * Arg2.Im / Module2;
  return Ret;
}
Complex_t ComplexDiv(Complex_t Arg1, Complex_t Arg2)
{
  Complex_t Ret;
  float Module2 = Arg2.Re*Arg2.Re + Arg2.Im*Arg2.Im;

  Ret.Re = (Arg1.Re * Arg2.Re + Arg1.Im * Arg2.Im)/ Module2;
  Ret.Im = (Arg1.Im * Arg2.Re - Arg1.Re * Arg2.Im)/ Module2;
  return Ret;
}
Complex_t ComplexMul(Complex_t Arg1, Complex_t Arg2)
{
  Complex_t Ret;

  Ret.Re = (Arg1.Re * Arg2.Re - Arg1.Im * Arg2.Im);
  Ret.Im = (Arg1.Re * Arg2.Im + Arg1.Im * Arg2.Re);
  return Ret;
}
