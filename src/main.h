#if !defined(__MAIN_H__)
#define __MAIN_H__
#include "calcz.h"
#include "calibration.h"
#include "adc.h"
//STM32F10X_LD_VL,BIG,LCD_USART,TEST_ADC,TEST_CALCZ,PWRMEAS,MSE,AUTO_RESET,TEST_CALIBR,CONST_CALIBRATION
#define SHUNT_SCHEM 0x1
extern uint8_t EqualSchem;
extern const float CFrenq[4];
extern uint8_t LoadCalibrationFlag;
extern Complex_t Zshort;
extern Complex_t Yopen;
Complex_t CalculateZ(uint8_t NoPercent); // Подсчет комплексного сопротивления без учета O-S калибровки (только load )
void ShowResult(Complex_t Z);
void ShowFrenq(void);
void ShowOverload(void);
void ShowDiapason(void);


void MainForm(void);

#endif /* __MAIN_H__ */
