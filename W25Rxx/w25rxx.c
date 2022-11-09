
#include "w25rxxConf.h"
#include "w25rxx.h"

#if (_W25RXX_DEBUG == 1)
#include <stdio.h>
#endif

#define W25RXX_DUMMY_BYTE 0xA5

W25RXX_t W25RXX;
extern SPI_HandleTypeDef _W25RXX_SPI;
#if (_W25RXX_USE_FREERTOS == 1)
#define W25RXX_Delay(delay) osDelay(delay)
#include "cmsis_os.h"
#else
#define W25RXX_Delay(delay) HAL_Delay(delay)
#endif
//###################################################################################################################
uint8_t W25RXX_Spi(uint8_t Data)
{
	uint8_t ret;
	HAL_SPI_TransmitReceive(&_W25RXX_SPI, &Data, &ret, 1, 100);
	return ret;
}
//###################################################################################################################
uint32_t W25RXX_ReadID(void)
{
	uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	W25RXX_Spi(0x9F);
	Temp0 = W25RXX_Spi(W25RXX_DUMMY_BYTE);
	Temp1 = W25RXX_Spi(W25RXX_DUMMY_BYTE);
	Temp2 = W25RXX_Spi(W25RXX_DUMMY_BYTE);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
	return Temp;
}
//###################################################################################################################
void W25RXX_ReadUniqID(void)
{
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	W25RXX_Spi(0x4B);
	for (uint8_t i = 0; i < 4; i++)
		W25RXX_Spi(W25RXX_DUMMY_BYTE);
	for (uint8_t i = 0; i < 8; i++)
		W25RXX.UniqID[i] = W25RXX_Spi(W25RXX_DUMMY_BYTE);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
}
//###################################################################################################################
void W25RXX_WriteEnable(void)
{
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	W25RXX_Spi(0x06);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	W25RXX_Delay(1);
}
//###################################################################################################################
void W25RXX_WriteDisable(void)
{
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	W25RXX_Spi(0x04);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	W25RXX_Delay(1);
}
//###################################################################################################################
uint8_t W25RXX_ReadStatusRegister(uint8_t SelectStatusRegister_1_2_3)
{
	uint8_t status = 0;
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	if (SelectStatusRegister_1_2_3 == 1)
	{
		W25RXX_Spi(0x05);
		status = W25RXX_Spi(W25RXX_DUMMY_BYTE);
		W25RXX.StatusRegister1 = status;
	}
	else if (SelectStatusRegister_1_2_3 == 2)
	{
		W25RXX_Spi(0x35);
		status = W25RXX_Spi(W25RXX_DUMMY_BYTE);
		W25RXX.StatusRegister2 = status;
	}
	else
	{
		W25RXX_Spi(0x15);
		status = W25RXX_Spi(W25RXX_DUMMY_BYTE);
		W25RXX.StatusRegister3 = status;
	}
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	return status;
}
//###################################################################################################################
void W25RXX_WriteStatusRegister(uint8_t SelectStatusRegister_1_2_3, uint8_t Data)
{
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	if (SelectStatusRegister_1_2_3 == 1)
	{
		W25RXX_Spi(0x01);
		W25RXX.StatusRegister1 = Data;
	}
	else if (SelectStatusRegister_1_2_3 == 2)
	{
		W25RXX_Spi(0x31);
		W25RXX.StatusRegister2 = Data;
	}
	else
	{
		W25RXX_Spi(0x11);
		W25RXX.StatusRegister3 = Data;
	}
	W25RXX_Spi(Data);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
}
//###################################################################################################################
void W25RXX_WaitForWriteEnd(void)
{
	W25RXX_Delay(1);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	W25RXX_Spi(0x05);
	do
	{
		W25RXX.StatusRegister1 = W25RXX_Spi(W25RXX_DUMMY_BYTE);
		W25RXX_Delay(1);
	} while ((W25RXX.StatusRegister1 & 0x01) == 0x01);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
}
//###################################################################################################################
bool W25RXX_Init(void)
{
	W25RXX.Lock = 1;
	while (HAL_GetTick() < 100)
		W25RXX_Delay(1);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	W25RXX_Delay(100);
	uint32_t id;
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX Init Begin...\r\n");
#endif
	id = W25RXX_ReadID();

#if (_W25RXX_DEBUG == 1)
	printf("W25RXX ID:0x%X\r\n", id);
#endif
	switch (id & 0x000000FF)
	{
	case 0x20: // 	w25q512
		W25RXX.ID = W25Q512;
		W25RXX.BlockCount = 1024;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25q512\r\n");
#endif
		break;
	case 0x19: // 	w25q256
		W25RXX.ID = W25Q256;
		W25RXX.BlockCount = 512;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25q256\r\n");
#endif
		break;
	case 0x18: // 	w25q128
		W25RXX.ID = W25Q128;
		W25RXX.BlockCount = 256;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25q128\r\n");
#endif
		break;
	case 0x17: //	w25q64
		W25RXX.ID = W25Q64;
		W25RXX.BlockCount = 128;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25q64\r\n");
#endif
		break;
	case 0x16: //	w25q32
		W25RXX.ID = W25Q32;
		W25RXX.BlockCount = 64;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25q32\r\n");
#endif
		break;
	case 0x15: //	w25q16
		W25RXX.ID = W25Q16;
		W25RXX.BlockCount = 32;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25q16\r\n");
#endif
		break;
	case 0x14: //	w25q80
		W25RXX.ID = W25R80;
		W25RXX.BlockCount = 16;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25r80\r\n");
#endif
		break;
	case 0x13: //	w25q40
		W25RXX.ID = W25Q40;
		W25RXX.BlockCount = 8;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25q40\r\n");
#endif
		break;
	case 0x12: //	w25q20
		W25RXX.ID = W25Q20;
		W25RXX.BlockCount = 4;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25q20\r\n");
#endif
		break;
	case 0x11: //	w25q10
		W25RXX.ID = W25Q10;
		W25RXX.BlockCount = 2;
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Chip: w25q10\r\n");
#endif
		break;
	default:
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX Unknown ID\r\n");
#endif
		W25RXX.Lock = 0;
		return false;
	}
	W25RXX.PageSize = 256;
	W25RXX.SectorSize = 0x1000;
	W25RXX.SectorCount = W25RXX.BlockCount * 16;
	W25RXX.PageCount = (W25RXX.SectorCount * W25RXX.SectorSize) / W25RXX.PageSize;
	W25RXX.BlockSize = W25RXX.SectorSize * 16;
	W25RXX.CapacityInKiloByte = (W25RXX.SectorCount * W25RXX.SectorSize) / 1024;
	W25RXX_ReadUniqID();
	W25RXX_ReadStatusRegister(1);
	W25RXX_ReadStatusRegister(2);
	W25RXX_ReadStatusRegister(3);
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX Page Size: %d Bytes\r\n", W25RXX.PageSize);
	printf("W25RXX Page Count: %d\r\n", W25RXX.PageCount);
	printf("W25RXX Sector Size: %d Bytes\r\n", W25RXX.SectorSize);
	printf("W25RXX Sector Count: %d\r\n", W25RXX.SectorCount);
	printf("W25RXX Block Size: %d Bytes\r\n", W25RXX.BlockSize);
	printf("W25RXX Block Count: %d\r\n", W25RXX.BlockCount);
	printf("W25RXX Capacity: %d KiloBytes\r\n", W25RXX.CapacityInKiloByte);
	printf("W25RXX Init Done\r\n");
#endif
	W25RXX.Lock = 0;
	return true;
}
//###################################################################################################################
void W25RXX_EraseChip(void)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
#if (_W25RXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	printf("W25RXX EraseChip Begin...\r\n");
#endif
	W25RXX_WriteEnable();
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	W25RXX_Spi(0xC7);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	W25RXX_WaitForWriteEnd();
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX EraseBlock done after %d ms!\r\n", HAL_GetTick() - StartTime);
#endif
	W25RXX_Delay(10);
	W25RXX.Lock = 0;
}
//###################################################################################################################
void W25RXX_EraseSector(uint32_t SectorAddr)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
#if (_W25RXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	printf("W25RXX EraseSector %d Begin...\r\n", SectorAddr);
#endif
	W25RXX_WaitForWriteEnd();
	SectorAddr = SectorAddr * W25RXX.SectorSize;
	W25RXX_WriteEnable();
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	if (W25RXX.ID >= W25Q256)
	{
		W25RXX_Spi(0x21);
		W25RXX_Spi((SectorAddr & 0xFF000000) >> 24);
	}
	else
	{
		W25RXX_Spi(0x20);
	}
	W25RXX_Spi((SectorAddr & 0xFF0000) >> 16);
	W25RXX_Spi((SectorAddr & 0xFF00) >> 8);
	W25RXX_Spi(SectorAddr & 0xFF);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	W25RXX_WaitForWriteEnd();
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX EraseSector done after %d ms\r\n", HAL_GetTick() - StartTime);
#endif
	W25RXX_Delay(1);
	W25RXX.Lock = 0;
}
//###################################################################################################################
void W25RXX_EraseBlock(uint32_t BlockAddr)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX EraseBlock %d Begin...\r\n", BlockAddr);
	W25RXX_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	W25RXX_WaitForWriteEnd();
	BlockAddr = BlockAddr * W25RXX.SectorSize * 16;
	W25RXX_WriteEnable();
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	if (W25RXX.ID >= W25Q256)
	{
		W25RXX_Spi(0xDC);
		W25RXX_Spi((BlockAddr & 0xFF000000) >> 24);
	}
	else
	{
		W25RXX_Spi(0xD8);
	}
	W25RXX_Spi((BlockAddr & 0xFF0000) >> 16);
	W25RXX_Spi((BlockAddr & 0xFF00) >> 8);
	W25RXX_Spi(BlockAddr & 0xFF);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	W25RXX_WaitForWriteEnd();
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX EraseBlock done after %d ms\r\n", HAL_GetTick() - StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX_Delay(1);
	W25RXX.Lock = 0;
}
//###################################################################################################################
uint32_t W25RXX_PageToSector(uint32_t PageAddress)
{
	return ((PageAddress * W25RXX.PageSize) / W25RXX.SectorSize);
}
//###################################################################################################################
uint32_t W25RXX_PageToBlock(uint32_t PageAddress)
{
	return ((PageAddress * W25RXX.PageSize) / W25RXX.BlockSize);
}
//###################################################################################################################
uint32_t W25RXX_SectorToBlock(uint32_t SectorAddress)
{
	return ((SectorAddress * W25RXX.SectorSize) / W25RXX.BlockSize);
}
//###################################################################################################################
uint32_t W25RXX_SectorToPage(uint32_t SectorAddress)
{
	return (SectorAddress * W25RXX.SectorSize) / W25RXX.PageSize;
}
//###################################################################################################################
uint32_t W25RXX_BlockToPage(uint32_t BlockAddress)
{
	return (BlockAddress * W25RXX.BlockSize) / W25RXX.PageSize;
}
//###################################################################################################################
bool W25RXX_IsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
	if (((NumByteToCheck_up_to_PageSize + OffsetInByte) > W25RXX.PageSize) || (NumByteToCheck_up_to_PageSize == 0))
		NumByteToCheck_up_to_PageSize = W25RXX.PageSize - OffsetInByte;
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX CheckPage:%d, Offset:%d, Bytes:%d begin...\r\n", Page_Address, OffsetInByte, NumByteToCheck_up_to_PageSize);
	W25RXX_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	uint8_t pBuffer[32];
	uint32_t WorkAddress;
	uint32_t i;
	for (i = OffsetInByte; i < W25RXX.PageSize; i += sizeof(pBuffer))
	{
		HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
		WorkAddress = (i + Page_Address * W25RXX.PageSize);
		if (W25RXX.ID >= W25Q256)
		{
			W25RXX_Spi(0x0C);
			W25RXX_Spi((WorkAddress & 0xFF000000) >> 24);
		}
		else
		{
			W25RXX_Spi(0x0B);
		}
		W25RXX_Spi((WorkAddress & 0xFF0000) >> 16);
		W25RXX_Spi((WorkAddress & 0xFF00) >> 8);
		W25RXX_Spi(WorkAddress & 0xFF);
		W25RXX_Spi(0);
		HAL_SPI_Receive(&_W25RXX_SPI, pBuffer, sizeof(pBuffer), 100);
		HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
		for (uint8_t x = 0; x < sizeof(pBuffer); x++)
		{
			if (pBuffer[x] != 0xFF)
				goto NOT_EMPTY;
		}
	}
	if ((W25RXX.PageSize + OffsetInByte) % sizeof(pBuffer) != 0)
	{
		i -= sizeof(pBuffer);
		for (; i < W25RXX.PageSize; i++)
		{
			HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
			WorkAddress = (i + Page_Address * W25RXX.PageSize);
			W25RXX_Spi(0x0B);
			if (W25RXX.ID >= W25Q256)
			{
				W25RXX_Spi(0x0C);
				W25RXX_Spi((WorkAddress & 0xFF000000) >> 24);
			}
			else
			{
				W25RXX_Spi(0x0B);
			}
			W25RXX_Spi((WorkAddress & 0xFF0000) >> 16);
			W25RXX_Spi((WorkAddress & 0xFF00) >> 8);
			W25RXX_Spi(WorkAddress & 0xFF);
			W25RXX_Spi(0);
			HAL_SPI_Receive(&_W25RXX_SPI, pBuffer, 1, 100);
			HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
			if (pBuffer[0] != 0xFF)
				goto NOT_EMPTY;
		}
	}
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX CheckPage is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX.Lock = 0;
	return true;
NOT_EMPTY:
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX CheckPage is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX.Lock = 0;
	return false;
}
//###################################################################################################################
bool W25RXX_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
	if ((NumByteToCheck_up_to_SectorSize > W25RXX.SectorSize) || (NumByteToCheck_up_to_SectorSize == 0))
		NumByteToCheck_up_to_SectorSize = W25RXX.SectorSize;
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX CheckSector:%d, Offset:%d, Bytes:%d begin...\r\n", Sector_Address, OffsetInByte, NumByteToCheck_up_to_SectorSize);
	W25RXX_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	uint8_t pBuffer[32];
	uint32_t WorkAddress;
	uint32_t i;
	for (i = OffsetInByte; i < W25RXX.SectorSize; i += sizeof(pBuffer))
	{
		HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
		WorkAddress = (i + Sector_Address * W25RXX.SectorSize);
		if (W25RXX.ID >= W25Q256)
		{
			W25RXX_Spi(0x0C);
			W25RXX_Spi((WorkAddress & 0xFF000000) >> 24);
		}
		else
		{
			W25RXX_Spi(0x0B);
		}
		W25RXX_Spi((WorkAddress & 0xFF0000) >> 16);
		W25RXX_Spi((WorkAddress & 0xFF00) >> 8);
		W25RXX_Spi(WorkAddress & 0xFF);
		W25RXX_Spi(0);
		HAL_SPI_Receive(&_W25RXX_SPI, pBuffer, sizeof(pBuffer), 100);
		HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
		for (uint8_t x = 0; x < sizeof(pBuffer); x++)
		{
			if (pBuffer[x] != 0xFF)
				goto NOT_EMPTY;
		}
	}
	if ((W25RXX.SectorSize + OffsetInByte) % sizeof(pBuffer) != 0)
	{
		i -= sizeof(pBuffer);
		for (; i < W25RXX.SectorSize; i++)
		{
			HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
			WorkAddress = (i + Sector_Address * W25RXX.SectorSize);
			if (W25RXX.ID >= W25Q256)
			{
				W25RXX_Spi(0x0C);
				W25RXX_Spi((WorkAddress & 0xFF000000) >> 24);
			}
			else
			{
				W25RXX_Spi(0x0B);
			}
			W25RXX_Spi((WorkAddress & 0xFF0000) >> 16);
			W25RXX_Spi((WorkAddress & 0xFF00) >> 8);
			W25RXX_Spi(WorkAddress & 0xFF);
			W25RXX_Spi(0);
			HAL_SPI_Receive(&_W25RXX_SPI, pBuffer, 1, 100);
			HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
			if (pBuffer[0] != 0xFF)
				goto NOT_EMPTY;
		}
	}
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX CheckSector is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX.Lock = 0;
	return true;
NOT_EMPTY:
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX CheckSector is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX.Lock = 0;
	return false;
}
//###################################################################################################################
bool W25RXX_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
	if ((NumByteToCheck_up_to_BlockSize > W25RXX.BlockSize) || (NumByteToCheck_up_to_BlockSize == 0))
		NumByteToCheck_up_to_BlockSize = W25RXX.BlockSize;
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX CheckBlock:%d, Offset:%d, Bytes:%d begin...\r\n", Block_Address, OffsetInByte, NumByteToCheck_up_to_BlockSize);
	W25RXX_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	uint8_t pBuffer[32];
	uint32_t WorkAddress;
	uint32_t i;
	for (i = OffsetInByte; i < W25RXX.BlockSize; i += sizeof(pBuffer))
	{
		HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
		WorkAddress = (i + Block_Address * W25RXX.BlockSize);

		if (W25RXX.ID >= W25Q256)
		{
			W25RXX_Spi(0x0C);
			W25RXX_Spi((WorkAddress & 0xFF000000) >> 24);
		}
		else
		{
			W25RXX_Spi(0x0B);
		}
		W25RXX_Spi((WorkAddress & 0xFF0000) >> 16);
		W25RXX_Spi((WorkAddress & 0xFF00) >> 8);
		W25RXX_Spi(WorkAddress & 0xFF);
		W25RXX_Spi(0);
		HAL_SPI_Receive(&_W25RXX_SPI, pBuffer, sizeof(pBuffer), 100);
		HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
		for (uint8_t x = 0; x < sizeof(pBuffer); x++)
		{
			if (pBuffer[x] != 0xFF)
				goto NOT_EMPTY;
		}
	}
	if ((W25RXX.BlockSize + OffsetInByte) % sizeof(pBuffer) != 0)
	{
		i -= sizeof(pBuffer);
		for (; i < W25RXX.BlockSize; i++)
		{
			HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
			WorkAddress = (i + Block_Address * W25RXX.BlockSize);

			if (W25RXX.ID >= W25Q256)
			{
				W25RXX_Spi(0x0C);
				W25RXX_Spi((WorkAddress & 0xFF000000) >> 24);
			}
			else
			{
				W25RXX_Spi(0x0B);
			}
			W25RXX_Spi((WorkAddress & 0xFF0000) >> 16);
			W25RXX_Spi((WorkAddress & 0xFF00) >> 8);
			W25RXX_Spi(WorkAddress & 0xFF);
			W25RXX_Spi(0);
			HAL_SPI_Receive(&_W25RXX_SPI, pBuffer, 1, 100);
			HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
			if (pBuffer[0] != 0xFF)
				goto NOT_EMPTY;
		}
	}
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX CheckBlock is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX.Lock = 0;
	return true;
NOT_EMPTY:
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX CheckBlock is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX.Lock = 0;
	return false;
}
//###################################################################################################################
void W25RXX_WriteByte(uint8_t pBuffer, uint32_t WriteAddr_inBytes)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
#if (_W25RXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	printf("W25RXX WriteByte 0x%02X at address %d begin...", pBuffer, WriteAddr_inBytes);
#endif
	W25RXX_WaitForWriteEnd();
	W25RXX_WriteEnable();
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);

	if (W25RXX.ID >= W25Q256)
	{
		W25RXX_Spi(0x12);
		W25RXX_Spi((WriteAddr_inBytes & 0xFF000000) >> 24);
	}
	else
	{
		W25RXX_Spi(0x02);
	}
	W25RXX_Spi((WriteAddr_inBytes & 0xFF0000) >> 16);
	W25RXX_Spi((WriteAddr_inBytes & 0xFF00) >> 8);
	W25RXX_Spi(WriteAddr_inBytes & 0xFF);
	W25RXX_Spi(pBuffer);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	W25RXX_WaitForWriteEnd();
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX WriteByte done after %d ms\r\n", HAL_GetTick() - StartTime);
#endif
	W25RXX.Lock = 0;
}
//###################################################################################################################
void W25RXX_WritePage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
	if (((NumByteToWrite_up_to_PageSize + OffsetInByte) > W25RXX.PageSize) || (NumByteToWrite_up_to_PageSize == 0))
		NumByteToWrite_up_to_PageSize = W25RXX.PageSize - OffsetInByte;
	if ((OffsetInByte + NumByteToWrite_up_to_PageSize) > W25RXX.PageSize)
		NumByteToWrite_up_to_PageSize = W25RXX.PageSize - OffsetInByte;
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX WritePage:%d, Offset:%d ,Writes %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToWrite_up_to_PageSize);
	W25RXX_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	W25RXX_WaitForWriteEnd();
	W25RXX_WriteEnable();
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	Page_Address = (Page_Address * W25RXX.PageSize) + OffsetInByte;
	if (W25RXX.ID >= W25Q256)
	{
		W25RXX_Spi(0x12);
		W25RXX_Spi((Page_Address & 0xFF000000) >> 24);
	}
	else
	{
		W25RXX_Spi(0x02);
	}
	W25RXX_Spi((Page_Address & 0xFF0000) >> 16);
	W25RXX_Spi((Page_Address & 0xFF00) >> 8);
	W25RXX_Spi(Page_Address & 0xFF);
	HAL_SPI_Transmit(&_W25RXX_SPI, pBuffer, NumByteToWrite_up_to_PageSize, 100);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
	W25RXX_WaitForWriteEnd();
#if (_W25RXX_DEBUG == 1)
	StartTime = HAL_GetTick() - StartTime;
	for (uint32_t i = 0; i < NumByteToWrite_up_to_PageSize; i++)
	{
		if ((i % 8 == 0) && (i > 2))
		{
			printf("\r\n");
			W25RXX_Delay(10);
		}
		printf("0x%02X,", pBuffer[i]);
	}
	printf("\r\n");
	printf("W25RXX WritePage done after %d ms\r\n", StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX_Delay(1);
	W25RXX.Lock = 0;
}
//###################################################################################################################
void W25RXX_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize)
{
	if ((NumByteToWrite_up_to_SectorSize > W25RXX.SectorSize) || (NumByteToWrite_up_to_SectorSize == 0))
		NumByteToWrite_up_to_SectorSize = W25RXX.SectorSize;
#if (_W25RXX_DEBUG == 1)
	printf("+++W25RXX WriteSector:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToWrite_up_to_SectorSize);
	W25RXX_Delay(100);
#endif
	if (OffsetInByte >= W25RXX.SectorSize)
	{
#if (_W25RXX_DEBUG == 1)
		printf("---W25RXX WriteSector Faild!\r\n");
		W25RXX_Delay(100);
#endif
		return;
	}
	uint32_t StartPage;
	int32_t BytesToWrite;
	uint32_t LocalOffset;
	if ((OffsetInByte + NumByteToWrite_up_to_SectorSize) > W25RXX.SectorSize)
		BytesToWrite = W25RXX.SectorSize - OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_SectorSize;
	StartPage = W25RXX_SectorToPage(Sector_Address) + (OffsetInByte / W25RXX.PageSize);
	LocalOffset = OffsetInByte % W25RXX.PageSize;
	do
	{
		W25RXX_WritePage(pBuffer, StartPage, LocalOffset, BytesToWrite);
		StartPage++;
		BytesToWrite -= W25RXX.PageSize - LocalOffset;
		pBuffer += W25RXX.PageSize - LocalOffset;
		LocalOffset = 0;
	} while (BytesToWrite > 0);
#if (_W25RXX_DEBUG == 1)
	printf("---W25RXX WriteSector Done\r\n");
	W25RXX_Delay(100);
#endif
}
//###################################################################################################################
void W25RXX_WriteBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize)
{
	if ((NumByteToWrite_up_to_BlockSize > W25RXX.BlockSize) || (NumByteToWrite_up_to_BlockSize == 0))
		NumByteToWrite_up_to_BlockSize = W25RXX.BlockSize;
#if (_W25RXX_DEBUG == 1)
	printf("+++W25RXX WriteBlock:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToWrite_up_to_BlockSize);
	W25RXX_Delay(100);
#endif
	if (OffsetInByte >= W25RXX.BlockSize)
	{
#if (_W25RXX_DEBUG == 1)
		printf("---W25RXX WriteBlock Faild!\r\n");
		W25RXX_Delay(100);
#endif
		return;
	}
	uint32_t StartPage;
	int32_t BytesToWrite;
	uint32_t LocalOffset;
	if ((OffsetInByte + NumByteToWrite_up_to_BlockSize) > W25RXX.BlockSize)
		BytesToWrite = W25RXX.BlockSize - OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_BlockSize;
	StartPage = W25RXX_BlockToPage(Block_Address) + (OffsetInByte / W25RXX.PageSize);
	LocalOffset = OffsetInByte % W25RXX.PageSize;
	do
	{
		W25RXX_WritePage(pBuffer, StartPage, LocalOffset, BytesToWrite);
		StartPage++;
		BytesToWrite -= W25RXX.PageSize - LocalOffset;
		pBuffer += W25RXX.PageSize - LocalOffset;
		LocalOffset = 0;
	} while (BytesToWrite > 0);
#if (_W25RXX_DEBUG == 1)
	printf("---W25RXX WriteBlock Done\r\n");
	W25RXX_Delay(100);
#endif
}
//###################################################################################################################
void W25RXX_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
#if (_W25RXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	printf("W25RXX ReadByte at address %d begin...\r\n", Bytes_Address);
#endif
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);

	if (W25RXX.ID >= W25Q256)
	{
		W25RXX_Spi(0x0C);
		W25RXX_Spi((Bytes_Address & 0xFF000000) >> 24);
	}
	else
	{
		W25RXX_Spi(0x0B);
	}
	W25RXX_Spi((Bytes_Address & 0xFF0000) >> 16);
	W25RXX_Spi((Bytes_Address & 0xFF00) >> 8);
	W25RXX_Spi(Bytes_Address & 0xFF);
	W25RXX_Spi(0);
	*pBuffer = W25RXX_Spi(W25RXX_DUMMY_BYTE);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX ReadByte 0x%02X done after %d ms\r\n", *pBuffer, HAL_GetTick() - StartTime);
#endif
	W25RXX.Lock = 0;
}
//###################################################################################################################
void W25RXX_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
#if (_W25RXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	printf("W25RXX ReadBytes at Address:%d, %d Bytes  begin...\r\n", ReadAddr, NumByteToRead);
#endif
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);

	if (W25RXX.ID >= W25Q256)
	{
		W25RXX_Spi(0x0C);
		W25RXX_Spi((ReadAddr & 0xFF000000) >> 24);
	}
	else
	{
		W25RXX_Spi(0x0B);
	}
	W25RXX_Spi((ReadAddr & 0xFF0000) >> 16);
	W25RXX_Spi((ReadAddr & 0xFF00) >> 8);
	W25RXX_Spi(ReadAddr & 0xFF);
	W25RXX_Spi(0);
	HAL_SPI_Receive(&_W25RXX_SPI, pBuffer, NumByteToRead, 2000);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
#if (_W25RXX_DEBUG == 1)
	StartTime = HAL_GetTick() - StartTime;
	for (uint32_t i = 0; i < NumByteToRead; i++)
	{
		if ((i % 8 == 0) && (i > 2))
		{
			printf("\r\n");
			W25RXX_Delay(10);
		}
		printf("0x%02X,", pBuffer[i]);
	}
	printf("\r\n");
	printf("W25RXX ReadBytes done after %d ms\r\n", StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX_Delay(1);
	W25RXX.Lock = 0;
}
//###################################################################################################################
void W25RXX_ReadPage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize)
{
	while (W25RXX.Lock == 1)
		W25RXX_Delay(1);
	W25RXX.Lock = 1;
	if ((NumByteToRead_up_to_PageSize > W25RXX.PageSize) || (NumByteToRead_up_to_PageSize == 0))
		NumByteToRead_up_to_PageSize = W25RXX.PageSize;
	if ((OffsetInByte + NumByteToRead_up_to_PageSize) > W25RXX.PageSize)
		NumByteToRead_up_to_PageSize = W25RXX.PageSize - OffsetInByte;
#if (_W25RXX_DEBUG == 1)
	printf("W25RXX ReadPage:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToRead_up_to_PageSize);
	W25RXX_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	Page_Address = Page_Address * W25RXX.PageSize + OffsetInByte;
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_RESET);
	if (W25RXX.ID >= W25Q256)
	{
		W25RXX_Spi(0x0C);
		W25RXX_Spi((Page_Address & 0xFF000000) >> 24);
	}
	else
	{
		W25RXX_Spi(0x0B);
	}
	W25RXX_Spi((Page_Address & 0xFF0000) >> 16);
	W25RXX_Spi((Page_Address & 0xFF00) >> 8);
	W25RXX_Spi(Page_Address & 0xFF);
	W25RXX_Spi(0);
	HAL_SPI_Receive(&_W25RXX_SPI, pBuffer, NumByteToRead_up_to_PageSize, 100);
	HAL_GPIO_WritePin(_W25RXX_CS_GPIO, _W25RXX_CS_PIN, GPIO_PIN_SET);
#if (_W25RXX_DEBUG == 1)
	StartTime = HAL_GetTick() - StartTime;
	for (uint32_t i = 0; i < NumByteToRead_up_to_PageSize; i++)
	{
		if ((i % 8 == 0) && (i > 2))
		{
			printf("\r\n");
			W25RXX_Delay(10);
		}
		printf("0x%02X,", pBuffer[i]);
	}
	printf("\r\n");
	printf("W25RXX ReadPage done after %d ms\r\n", StartTime);
	W25RXX_Delay(100);
#endif
	W25RXX_Delay(1);
	W25RXX.Lock = 0;
}
//###################################################################################################################
void W25RXX_ReadSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize)
{
	if ((NumByteToRead_up_to_SectorSize > W25RXX.SectorSize) || (NumByteToRead_up_to_SectorSize == 0))
		NumByteToRead_up_to_SectorSize = W25RXX.SectorSize;
#if (_W25RXX_DEBUG == 1)
	printf("+++W25RXX ReadSector:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToRead_up_to_SectorSize);
	W25RXX_Delay(100);
#endif
	if (OffsetInByte >= W25RXX.SectorSize)
	{
#if (_W25RXX_DEBUG == 1)
		printf("---W25RXX ReadSector Faild!\r\n");
		W25RXX_Delay(100);
#endif
		return;
	}
	uint32_t StartPage;
	int32_t BytesToRead;
	uint32_t LocalOffset;
	if ((OffsetInByte + NumByteToRead_up_to_SectorSize) > W25RXX.SectorSize)
		BytesToRead = W25RXX.SectorSize - OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_SectorSize;
	StartPage = W25RXX_SectorToPage(Sector_Address) + (OffsetInByte / W25RXX.PageSize);
	LocalOffset = OffsetInByte % W25RXX.PageSize;
	do
	{
		W25RXX_ReadPage(pBuffer, StartPage, LocalOffset, BytesToRead);
		StartPage++;
		BytesToRead -= W25RXX.PageSize - LocalOffset;
		pBuffer += W25RXX.PageSize - LocalOffset;
		LocalOffset = 0;
	} while (BytesToRead > 0);
#if (_W25RXX_DEBUG == 1)
	printf("---W25RXX ReadSector Done\r\n");
	W25RXX_Delay(100);
#endif
}
//###################################################################################################################
void W25RXX_ReadBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize)
{
	if ((NumByteToRead_up_to_BlockSize > W25RXX.BlockSize) || (NumByteToRead_up_to_BlockSize == 0))
		NumByteToRead_up_to_BlockSize = W25RXX.BlockSize;
#if (_W25RXX_DEBUG == 1)
	printf("+++W25RXX ReadBlock:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToRead_up_to_BlockSize);
	W25RXX_Delay(100);
#endif
	if (OffsetInByte >= W25RXX.BlockSize)
	{
#if (_W25RXX_DEBUG == 1)
		printf("W25RXX ReadBlock Faild!\r\n");
		W25RXX_Delay(100);
#endif
		return;
	}
	uint32_t StartPage;
	int32_t BytesToRead;
	uint32_t LocalOffset;
	if ((OffsetInByte + NumByteToRead_up_to_BlockSize) > W25RXX.BlockSize)
		BytesToRead = W25RXX.BlockSize - OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_BlockSize;
	StartPage = W25RXX_BlockToPage(Block_Address) + (OffsetInByte / W25RXX.PageSize);
	LocalOffset = OffsetInByte % W25RXX.PageSize;
	do
	{
		W25RXX_ReadPage(pBuffer, StartPage, LocalOffset, BytesToRead);
		StartPage++;
		BytesToRead -= W25RXX.PageSize - LocalOffset;
		pBuffer += W25RXX.PageSize - LocalOffset;
		LocalOffset = 0;
	} while (BytesToRead > 0);
#if (_W25RXX_DEBUG == 1)
	printf("---W25RXX ReadBlock Done\r\n");
	W25RXX_Delay(100);
#endif
}
//###################################################################################################################
