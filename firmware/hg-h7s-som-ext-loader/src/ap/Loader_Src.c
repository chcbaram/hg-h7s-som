#include "Loader_Src.h"
#include "hw.h"





/**
  * @brief   Program memory.
  * @param   Address: page address
  * @param   Size   : size of data
  * @param   buffer : pointer to data buffer
  * @retval  1      : Operation succeeded
  * @retval  0      : Operation failed
  */
int Init()
{
  int ret = 0;

  if (hwInit() == true)
  {
    ledToggle(_DEF_LED1);
    ret = 1;
  }

  return ret;
}

KeepInCompilation 
int Read (uint32_t Address, uint32_t Size, uint8_t* Buffer)
{
  int ret = 1;
  uint32_t read_addr;


  ledToggle(_DEF_LED1);

  read_addr = Address - qspiGetAddr();

  if (qspiRead(read_addr, Buffer, Size) == true)
  {
    ret = 1;
  }
	return ret;
} 

/**
  * @brief   Program memory.
  * @param   Address: page address
  * @param   Size   : size of data
  * @param   buffer : pointer to data buffer
  * @retval  1      : Operation succeeded
  * @retval  0      : Operation failed
  */
KeepInCompilation 
int Write (uint32_t Address, uint32_t Size, uint8_t* buffer)
{
  int ret = 0;
  uint32_t write_addr;

  ledToggle(_DEF_LED1);

  write_addr = Address - qspiGetAddr();

  if (qspiWrite(write_addr, buffer, Size) == true)
  {
    ret = 1;
  }

  return ret;
}

/**
  * @brief   Sector erase.
  * @param   EraseStartAddress :  erase start address
  * @param   EraseEndAddress   :  erase end address
  * @retval  1      : Operation succeeded
  * @retval  0      : Operation failed
  */
KeepInCompilation 
int SectorErase (uint32_t EraseStartAddress ,uint32_t EraseEndAddress)
{
  int ret = 1;
  uint32_t BlockAddr;
  EraseStartAddress = EraseStartAddress -  EraseStartAddress % 0x00010000;

  ledToggle(_DEF_LED1);
// return 1;
  while (EraseEndAddress >= EraseStartAddress)
  {
    BlockAddr = EraseStartAddress & 0x0FFFFFFF;
    if (qspiEraseSector(BlockAddr) != true)
    {
      ret = 0;
      break;
    }
    ledToggle(_DEF_LED1);
    EraseStartAddress += 0x00010000;
  }

 	return ret;  
}

/**
  * Description :
  * Calculates checksum value of the memory zone
  * Inputs    :
  *      StartAddress  : Flash start address
  *      Size          : Size (in WORD)  
  *      InitVal       : Initial CRC value
  * outputs   :
  *     R0             : Checksum value
  * Note: Optional for all types of device
  */
uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal)
{
  uint8_t missalignementAddress = StartAddress%4;
  uint8_t missalignementSize = Size ;
  int cnt;
  uint32_t Val;
	
  StartAddress-=StartAddress%4;
  Size += (Size%4==0)?0:4-(Size%4);
  
  for(cnt=0; cnt<Size ; cnt+=4)
  {
    Val = *(uint32_t*)StartAddress;
    if(missalignementAddress)
    {
      switch (missalignementAddress)
      {
        case 1:
          InitVal += (uint8_t) (Val>>8 & 0xff);
          InitVal += (uint8_t) (Val>>16 & 0xff);
          InitVal += (uint8_t) (Val>>24 & 0xff);
          missalignementAddress-=1;
          break;
        case 2:
          InitVal += (uint8_t) (Val>>16 & 0xff);
          InitVal += (uint8_t) (Val>>24 & 0xff);
          missalignementAddress-=2;
          break;
        case 3:   
          InitVal += (uint8_t) (Val>>24 & 0xff);
          missalignementAddress-=3;
          break;
      }  
    }
    else if((Size-missalignementSize)%4 && (Size-cnt) <=4)
    {
      switch (Size-missalignementSize)
      {
        case 1:
          InitVal += (uint8_t) Val;
          InitVal += (uint8_t) (Val>>8 & 0xff);
          InitVal += (uint8_t) (Val>>16 & 0xff);
          missalignementSize-=1;
          break;
        case 2:
          InitVal += (uint8_t) Val;
          InitVal += (uint8_t) (Val>>8 & 0xff);
          missalignementSize-=2;
          break;
        case 3:   
          InitVal += (uint8_t) Val;
          missalignementSize-=3;
          break;
      } 
    }
    else
    {
      InitVal += (uint8_t) Val;
      InitVal += (uint8_t) (Val>>8 & 0xff);
      InitVal += (uint8_t) (Val>>16 & 0xff);
      InitVal += (uint8_t) (Val>>24 & 0xff);
    }
    StartAddress+=4;
  }
  
  return (InitVal);
}

/**
  * Description :
  * Verify flash memory with RAM buffer and calculates checksum value of
  * the programmed memory
  * Inputs    :
  *      FlashAddr     : Flash address
  *      RAMBufferAddr : RAM buffer address
  *      Size          : Size (in WORD)  
  *      InitVal       : Initial CRC value
  * outputs   :
  *     R0             : Operation failed (address of failure)
  *     R1             : Checksum value
  * Note: Optional for all types of device
  */
KeepInCompilation 
uint64_t Verify (uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size, uint32_t missalignement)
{
  uint32_t VerifiedData = 0, InitVal = 0;
  uint64_t checksum;
  Size*=4;

  ledToggle(_DEF_LED1);

  qspiEnableMemoryMappedMode();

  checksum = CheckSum((uint32_t)MemoryAddr + (missalignement & 0xf), Size - ((missalignement >> 16) & 0xF), InitVal);
  while (Size>VerifiedData)
  {
    if ( *(uint8_t*)MemoryAddr++ != *((uint8_t*)RAMBufferAddr + VerifiedData))
      return ((checksum<<32) + (MemoryAddr + VerifiedData));  
   
    VerifiedData++;  
  }
        
  return (checksum<<32);
}

/**
  * @brief 	 Full erase of the device 						
  * @param 	 Parallelism : 0 																		
  * @retval  1           : Operation succeeded
  * @retval  0           : Operation failed											
  */
KeepInCompilation 
int MassErase (uint32_t Parallelism )
{
  int ret = 0;

  ledToggle(_DEF_LED1);

  if (qspiEraseChip() == true)
  {
    ret = 1;
  }
  return ret;
}
