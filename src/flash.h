#if !defined(__FLASH_H__)
#define __FLASH_H__
void WriteFlash(void* Src, void* Dst, int Len);
void ErasePage(void* Addr);
#endif /* __FLASH_H__ */
