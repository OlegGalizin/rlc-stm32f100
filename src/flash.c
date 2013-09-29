#include "stm32f10x.h"
#include "flash.h"

#define RDP_Key                  ((uint16_t)0x00A5)
#define FLASH_KEY1               ((uint32_t)0x45670123)
#define FLASH_KEY2               ((uint32_t)0xCDEF89AB)


void WriteFlash(void* Src, void* Dst, int Len)
{
  uint16_t* SrcW = (uint16_t*)Src;
  volatile uint16_t* DstW = (uint16_t*)Dst;

  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;

  FLASH->CR |= FLASH_CR_PG; /* Programm the flash */
  while (Len)
  {
    *DstW = *SrcW;
    while ((FLASH->SR & FLASH_SR_BSY) != 0 )
      ;
    if (*DstW != *SrcW )
    {
      break;
    }
    DstW++;
    SrcW++;
    Len = Len - sizeof(uint16_t);
  }

  FLASH->CR &= ~FLASH_CR_PG; /* Reset the flag back !!!! */
  FLASH->CR |= FLASH_CR_LOCK; /* Lock the flash back */
}
void ErasePage(void* Addr)
{
  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;

  FLASH->CR |= FLASH_CR_PER; /* Page erase */
  FLASH->AR = (uint32_t)Addr; 
  FLASH->CR|= FLASH_CR_STRT; /* Start erase */
  while ((FLASH->SR & FLASH_SR_BSY) != 0 ) /* Wait end of eraze */
    ;
  FLASH->CR &= ~FLASH_CR_PER; /* Page erase end */
}
