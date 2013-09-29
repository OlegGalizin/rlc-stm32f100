#if !defined(__COMPL_H__)
#define __COMPL_H__

typedef struct 
{
  float Re;
  float Im;
}Complex_t;

extern Complex_t ComplexDivToFloat(Complex_t Arg1, float Arg2);
extern Complex_t ComplexDivFloat(float Arg1, Complex_t Arg2);
extern Complex_t ComplexDiv(Complex_t Arg1, Complex_t Arg2);
extern Complex_t ComplexMul(Complex_t Arg1, Complex_t Arg2);

#endif /* __COMPL_H__ */
