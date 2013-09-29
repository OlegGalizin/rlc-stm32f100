#if !defined(__CALIBRATION_H__)
#define __CALIBRATION_H__
#include "calcz.h"

typedef struct
{
  Complex_t osm[17*4];
  Complex_t stdm[17*4];
} ZArray_t;

float RMult(int8_t diapason);

typedef union
{
  char  Data[sizeof(ZArray_t)]; /*  8*9*4*2 + 8*4 = 608 */
  ZArray_t Z;
} Coeff_t;


typedef struct 
{
  uint16_t Rstd[17];
  uint16_t ParasiticL; /* pH for 0-5 diapasons */
  uint16_t ParasiticC; /* fF for 6-16 diapasons */
  uint16_t PowerFactor;
  Coeff_t Data;
  uint16_t InstrOffset;
  uint16_t OutOffset;
  volatile uint16_t InvalidFlag; /* 2 byte */
}Calibration_t;

extern const Calibration_t Calibration;
extern const float CFrenq[4];
#if !defined(M_PI)
#define M_PI 3.14159265358979323846f
#endif


extern void CalibrationStart(void);
#if defined(MSE)
extern float GetPercent(float MeanValue, float SumOfSquare);
#endif

#endif /* __CALIBRATION_H__ */

