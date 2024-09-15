#include "Dev_Inf.h"



struct StorageInfo const StorageInfo  =  
{
   "W25Q256JV_HG-H7S-SOM", 	  					          // Device Name 
   SPI_FLASH,                   					        // Device Type
   0x70000000,                						        // Device Start Address
   0x02000000,              						          // Device Size in 32 MBytes
   0x100,                     						        // Programming Page Size 256 Bytes
   0xFF,                       						        // Initial Content of Erased Memory
// Specify Size and Address of Sectors (view example below)
   {
    {0x00000200, 0x00010000},     				 		    // Sector Num : 512 ,Sector Size: 64 KBytes
    {0x00000000, 0x00000000},
   }      
};