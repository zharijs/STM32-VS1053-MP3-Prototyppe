/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sdcard.h"		/* Example: Header file of existing MMC/SDC contorl module */
#include "stm32f10x.h"
#include <stdio.h>

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/*-----------------------------------------------------------------------*/

#define FS_PRINTF_EN	1		/* 1 means print diagnostic printf */

#if FS_PRINTF_EN == 1
    #define fs_printf(...)	printf(__VA_ARGS__)
#else
    #define fs_printf(...)
#endif	


#define SECTOR_SIZE		512

uint32_t Mass_Block_Size,Mass_Block_Count;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
  SD_CardInfo mSDCardInfo;
  uint32_t DeviceSizeMul = 0, NumberOfBlocks = 0;	
  uint16_t Status;

	Status = SD_Init();
	if (Status != SD_OK)	
	{
		fs_printf("SD_Init() fail (%d) : file %s on line %d\r\n", Status, __FILE__, __LINE__);
		goto retfail;
	}

	SD_GetCardInfo(&mSDCardInfo);
	SD_SelectDeselect((uint32_t) (mSDCardInfo.RCA << 16));
	DeviceSizeMul = (mSDCardInfo.SD_csd.DeviceSizeMul + 2);
	
	if (mSDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		Mass_Block_Count = (mSDCardInfo.SD_csd.DeviceSize + 1) * 1024;
	}
	else
	{
		NumberOfBlocks  = ((1 << (mSDCardInfo.SD_csd.RdBlockLen)) / 512);
		Mass_Block_Count = ((mSDCardInfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks/2));
	}
	
	Status = SD_SelectDeselect((uint32_t) (mSDCardInfo.RCA << 16)); 
	Status = SD_EnableWideBusOperation(SDIO_BusWide_1b);  //Amended to slower speed to make it work
	if (Status != SD_OK)
	{
		fs_printf("SD_EnableWideBusOperation(SDIO_BusWide_4b) Fail (%d)\r\n", Status);
		goto retfail;
	}
	
	Status = SD_SetDeviceMode(SD_DMA_MODE);   
	if (Status != SD_OK)
	{
		fs_printf("SD_SetDeviceMode(SD_DMA_MODE) Fail (%d)\r\n", Status);				
		goto retfail;
	} 

	Mass_Block_Size  = 512;
	if (mSDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{	
		fs_printf("SDHC Card, Memory Size = %uMB\r\n", Mass_Block_Count / (1024 * 4));
	}
	else
	{
		fs_printf("Normal Card, Memory Size = %uMB\r\n", (Mass_Block_Count * Mass_Block_Size) /(1024*1024));
	}
			
	return RES_OK;
retfail:	
	return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	SD_Error Status = SD_OK;

	if (count == 1) 
	{
                //printf("Reading Single Block\n");
		Status = SD_ReadBlock(sector << 9 , (uint32_t*)buff, SECTOR_SIZE);
	} 
	else 
	{
                //printf("OMG! Reading multiple blocks!\n");
		Status = SD_ReadMultiBlocks(sector<<9 , (uint32_t*)buff, SECTOR_SIZE, count);
	}

	if (Status == SD_OK) 
	{
		return RES_OK;
	} 
	else
	{
		printf("Err: SD_ReadMultiBlocks(,%d,%d)\r\n",sector,count);
		return RES_ERROR;
	}
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	SD_Error Status = SD_OK;
	
#if 1
	while (count--)
	{
		Status = SD_WriteBlock(sector << 9 ,(uint32_t *)buff, SECTOR_SIZE);
		if (Status != SD_OK)
		{
			break;
		}
	}
#else		/* SD_WriteMultiBlocks() Å¼¶û»áÖ´ÐÐ³ö´í */	
	if (count == 1) 
	{
		Status = SD_WriteBlock(sector << 9 ,(uint32_t *)buff, SECTOR_SIZE);
	} 
	else 
	{
		Status = SD_WriteMultiBlocks(sector << 9 ,(uint32_t *)buff, SECTOR_SIZE, count);	
	}
#endif	

	if (Status == SD_OK) 
	{
		return RES_OK;
	} 
	else
	{
		printf("Err: SD_WriteBlocks(,%d,%d)\r\n",sector,count);
		return RES_ERROR;
	}
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_OK;
}
#endif

unsigned short get_fattime(void){  
    return 0;  
}