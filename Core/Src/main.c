/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  *
  * Version Header Example (32B)
  * ICTR-L1E-APP-SupV.MajV.MinV-YYWW[P/D/I]
  * ICTR-L1E-REC-MajV.MinV-YYWW[P/D/I]
  * 
  * 
  * 
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w25rxx.h"
#include "gsm.h"
#include <stdio.h>
#include <string.h>
#include "../Config.h"
#include "../ws2812b/ws2812b.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef  void (*pFunction)(void);
typedef enum
{
  FACTORY_RESET_FLAG = 0,
  ACTIVATION_STATUS_FLAG,
  CERTS_UPDATE_FLAG,
  OTA_FLAG,
  RECOVERY_FLAG,
} flag_t;
typedef enum
{
  BOOTLOADER = 0,
  RECOVERY,
  APPLICATION,
} fw_type_t;
typedef enum 
{
  VERSION = 0,
  SN = 32,
  IMEI = 64,
  HNAME = 96,
  MUID = 128,
  PMdl = 160,
  DMdl = 192,
  UDID = 224,
  DCfg = 256,
} udcfg_addr_t;
/*typedef enum
{
  FACTORY_RESET_EVENT = 0,
  OTA_UPD_SUC_EVENT = 1,
} event_t;*/
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_START_ADDR 0x08010000UL
#define MAGIC "ITCR-ODIN-L1E-OTA-d41d8cd98f00b204e9800998ecf842"
#define BLR_VERSION "1.0-2234D"
#define BLR_VERSION_HEADER "ICTR-L1E-BLR-" + BLR_VERSION

#define MQTT_SERVER "mqtt.iotcreative.ru"
#define MQTT_USER "dave"
#define MQTT_PSWD "lemontree"

/* NVM ADDRESSES*/

#define RECOVERY_VER_HEAD_ADDR 0x000000UL
#define RECOVERY_LEN_ADDR 0x000020UL
#define RECOVERY_CRC_ADDR 0x000024UL

#define RECOVERY_ADDR 0x000028UL
#define FLAGS_ADDR 0x070000UL
#define LOGS_ADDR 0x060000UL

/* BOOT_INFO */

#define BOOT_SEC_ADDR 0x071000UL
#define APP_VER_HEAD_ADDR 0x071000UL
#define APP_LEN_ADDR 0x071020UL 
#define APP_CRC_ADDR 0x071024UL 
#define MAGIC_ADDR 0x071028UL 

/* USER_INFO */

#define USER_INFO_START_ADDR 0x073000UL
#define USER_INFO_END_ADDR 0x079000UL

#define ROOT_CERT_ADDR 0x07B000UL
#define CLIENT_CERT_ADDR 0x07D000UL

/* UPDATE PARTITION */

#define UPD_PARTITION_ADDR 0x080000UL
#define UPD_VER_HEAD_ADDR 0x080000UL
#define UPD_LEN_ADDR 0x080020UL 
#define UPD_CRC_ADDR 0x080024UL 
#define UPD_START_ADDR 0x080028UL

/*EVENT MESSAGES*/

#define FACTORY_RESET_EVENT "Factory reset"
#define OTA_UPDATE_EVENT "Updated to version: "
#define RECOVERY_EVENT "Flashed to recovery"
#define RECOVERY_SUCCESSFUL_EVENT "Recovered to version: "
#define SOS_EVENT "SOS"
#define MEMORY_ERROR_EVENT "Memory error: "
#define MODEM_ERROR_EVENT "Modem error: "
#define WRONG_CRC_EVENT "Wrong app checksum"
#define HAL_ERROR_EVENT "HAL Error: "
#define ODIN_ERROR_EVENT "ODIN Error: "

#define BOOTLOADER_DEBUG

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
struct UDCfg
{
  char Version [32];
  char SN [32]; 
  char IMEI [32];
  char UCID [32];
  char HName [32];
  char MUID [32];
  char PMdl [32];
  char DMdl [32];
  char UDID [32];
  char DCfg [32];
};
struct UDCfg dev_cfg;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
bool ODIN_Diags(void);
bool CheckForMW(void);
bool isAppPresent(void);
void ODIN_OTA(void);
void Flash_Verify(uint32_t nvm_addr, uint32_t nvm_len_addr);
void ODIN_Recovery(void);
void ODIN_SOS(void);
void ODIN_Err(uint8_t err_num);
void WriteBootSection(uint32_t ver_head_addr, uint32_t len_addr, uint32_t crc_addr);
void GotoApp(void);
void WriteLog(char* event);
void ReadLog(void);
bool getFlag(flag_t flag);
void setFlag(flag_t flag, bool value);
void FactoryReset(void);
bool CertsUpdate(void);
void ReadUDCfg(void);
void PrintLog(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_DMA_Init();
  MX_USB_DEVICE_Init();
  MX_TIM1_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(1000);
  printf("Lampan Bootloader, version %s\n", BLR_VERSION);
  //ws2812b_init();
  if(!ODIN_Diags())
  {
    ODIN_Err(1);
  }
  if(getFlag(FACTORY_RESET_FLAG))
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("Factory reset flag found\n");
    #endif
    FactoryReset();
  }
  if(getFlag(CERTS_UPDATE_FLAG))
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("Certificate update flag found\n");
    #endif
    CertsUpdate();
  }
  if(getFlag(OTA_FLAG))
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("OTA Flag found. Check for MW\n");
    #endif
    if(CheckForMW())
    {
	    ODIN_OTA();
    }
  }
  if(isAppPresent())
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("Application is intact. Proceed to app...\n");
    #endif
    GotoApp();
  }
  else
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("Application has been corrupted. Proceed to recovery...\n");
    #endif
	  ODIN_Recovery();
  }
  ODIN_SOS();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("Device is bricked. Proceed to shell...\n");
      printf("->");
    #endif
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int _write(int file, char *ptr, int len) { 
    CDC_Transmit_FS((uint8_t*) ptr, len); return len; 
}
/**
 * @brief ODIN_Diags() - POST function
 * 
 * @return true 
 * @return false 
 */
bool ODIN_Diags()
{
#ifdef BOOTLOADER_DEBUG
	printf("Start POST. Memory?\n");
#endif
	//POST - Memory and Modem
	if(!W25RXX_Init())
	{
    #ifdef BOOTLOADER_DEBUG
	    printf("Error during memory initialization.");
    #endif		    
    char* event_error = calloc(128, sizeof(char));
    strcat(event_error, MEMORY_ERROR_EVENT);
    strcat(event_error, "Initialization");
    WriteLog(event_error);
    free(event_error);
    //return false;
	}
  #ifdef BOOTLOADER_DEBUG
	  printf("Memory passed. Modem?\n");
  #endif
	if(!gsm_init())
	{
    #ifdef BOOTLOADER_DEBUG
	    printf("Error during modem initialization.");
    #endif			    
    char* event_error = calloc(128, sizeof(char));
    strcat(event_error, MODEM_ERROR_EVENT);
    strcat(event_error, "Initialization");
    WriteLog(event_error);
    free(event_error);
    return false;
	}
  if(!gsm_gprs_ntpServer("time.google.com", 6))
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("No connection to NTP.");
    #endif	
  }
  if(!gsm_gprs_ntpSyncTime())
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("Time not synchronized.");
    #endif	
  }
  #ifdef BOOTLOADER_DEBUG
	  printf("Modem passed. Exit DIAGS...\n");
  #endif
  return true;
}
/**
 * @brief Checking CRC from FLASH and CRC of image from NVM
 * 
 * @return true 
 * @return false 
 */
bool isAppPresent(void)
{
  #ifdef BOOTLOADER_DEBUG
	  printf("Application integrity check\n");
  #endif
  uint32_t file_len; 
  uint32_t image_crc;
  W25RXX_ReadBytes(file_len, APP_LEN_ADDR, 4);
  W25RXX_ReadBytes(image_crc, APP_CRC_ADDR, 4);
  #ifdef BOOTLOADER_DEBUG
	  printf("Original checksum: %x\n", image_crc);
  #endif
  uint32_t calc_crc = HAL_CRC_Calculate(&hcrc, (uint32_t*) APP_START_ADDR, file_len/4); //file_len/4 because HAL_CRC_Calculate requires number of UINT32_T words
  #ifdef BOOTLOADER_DEBUG
	  printf("Calculated checksum: %x\n", calc_crc);
  #endif
  if (calc_crc == image_crc)
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("Checksum is correct. Proceed...");
    #endif
    return true;
  }
	else
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("Checksum is NOT correct. Proceed...");
    #endif    
    WriteLog(WRONG_CRC_EVENT);
    return false;
  }
}
/**
 * @brief Check NVM for magic word
 * 
 * @return true MW Found in NVM
 * @return false MW NOT found in NVM
 */
bool CheckForMW(void)
{
  #ifdef BOOTLOADER_DEBUG
	  printf("Checking NVM for Magic Word\n");
  #endif
  char MW_Check [49];
  W25RXX_ReadBytes(MW_Check, MAGIC_ADDR, 50);
  if (MW_Check == MAGIC)
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("MW Found!\n");
    #endif
    return true;
  }
	else
  {
    #ifdef BOOTLOADER_DEBUG
	    printf("MW NOT found.\n");
    #endif
    return false;
  }
}
/**
 * @brief ODIN Error Handler
 * 
 * @param err_num Error Number - Refer to Errors Documentation
 */
void ODIN_Err(uint8_t err_num)
{
  char* event = calloc(128, sizeof(char));
  strcat(event, ODIN_ERROR_EVENT);
  strcat(event, (char)err_num);
  WriteLog(event);
  free(event);
  //Matrix code
  int i = 0;
	while(i < 4)
  {
    i++;

  }
    #ifdef BOOTLOADER_DEBUG
      return;
    #endif
  NVIC_SystemReset();
}
/**
 * @brief Flashing RecoveryFW
 * 
 */
void ODIN_Recovery(void)
{
    #ifdef BOOTLOADER_DEBUG
	    printf("Device is in recovery mode. Flash recovery FW...\n");
    #endif
    uint32_t len;
    W25RXX_ReadBytes(len, RECOVERY_LEN_ADDR, 4);
    if (len == 0)
    {
      #ifdef BOOTLOADER_DEBUG
	      printf("RecFW length is zero. RecFW info has been lost. Further flashing is not possible\n");
      #endif
      return;
    }
    Flash_Verify(RECOVERY_ADDR, RECOVERY_LEN_ADDR);
    #ifdef BOOTLOADER_DEBUG
	    printf("Recovery FW flashed. Copying App Information to Boot Section...\n");
    #endif
    WriteBootSection(RECOVERY_VER_HEAD_ADDR, RECOVERY_LEN_ADDR, RECOVERY_CRC_ADDR);  
    #ifdef BOOTLOADER_DEBUG
	    printf("Setting RECOVERY_FLAG...\n");
    #endif    
    setFlag(RECOVERY_FLAG, true);
    #ifdef BOOTLOADER_DEBUG
	    printf("Writing recovery event log...\n");
    #endif  
    WriteLog(RECOVERY_EVENT);

    #ifdef BOOTLOADER_DEBUG
	    printf("Rebooting...\n");
    #endif        
    NVIC_SystemReset();
}
/**
 * @brief Firmware Update Over-The-Air (OTA) Handler
 * 
 */
void ODIN_OTA(void)
{
    #ifdef BOOTLOADER_DEBUG
	    printf("Starting OTA...\n");
    #endif
    char VerHeader [32];
    W25RXX_ReadBytes(VerHeader, UPD_VER_HEAD_ADDR, 32);
    strcat(VerHeader, '\0');
    #ifdef BOOTLOADER_DEBUG
	    printf("Flashing FW: %s\n", VerHeader);
    #endif            
    Flash_Verify(UPD_START_ADDR, UPD_LEN_ADDR);
    #ifdef BOOTLOADER_DEBUG
	    printf("Flash&Verify complete. Write info to Boot Section\n");
    #endif
    WriteBootSection(UPD_VER_HEAD_ADDR, UPD_LEN_ADDR, UPD_CRC_ADDR);
    #ifdef BOOTLOADER_DEBUG
	    printf("Erasing update binary...\n");
    #endif
    
    uint32_t end_addr = W25RXX.CapacityInKiloByte * 1024;
    for(uint32_t idx = UPD_PARTITION_ADDR; idx <= end_addr; idx += 65536) //UPD_PARTITION_ADDR = UPD_VER_HEAD_ADDR
    {
        W25RXX_EraseBlock(idx);
    }
    #ifdef BOOTLOADER_DEBUG
	    printf("Writing log...\n");
    #endif
    if (getFlag(RECOVERY_FLAG))
    {
      setFlag(RECOVERY_FLAG, false);
      char* event = calloc(512, sizeof(char));
      strcpy(event, RECOVERY_SUCCESSFUL_EVENT);
      strcat(event, VerHeader);
      WriteLog(event);
      free(event);
    }
    else
    {
      char* event = calloc(512, sizeof(char));
      strcpy(event, OTA_UPDATE_EVENT);
      strcat(event, VerHeader);
      WriteLog(event);
    }
    #ifdef BOOTLOADER_DEBUG
	    printf("Reseting OTA_FLAG...\n");
    #endif
    setFlag(OTA_FLAG, false);
    #ifdef BOOTLOADER_DEBUG
	    printf("OTA Complete. Restart...\n");
    #endif
    NVIC_SystemReset();
}
/**
 * @brief Flash binary from NVM to FLASH MCU memory and verify correct flashing
 * 
 * @param nvm_addr Starting binary address in NVM
 * @param nvm_len_addr Binary length
 */
void Flash_Verify(uint32_t nvm_addr, uint32_t nvm_len_addr)
{
    #ifdef BOOTLOADER_DEBUG
	    printf("Flashing in progress:\n");
    #endif        
    uint32_t file_len; 
    W25RXX_ReadBytes(file_len, nvm_len_addr, 4); //Get update file size
    uint32_t NVM_Addr = nvm_addr; //Set variable to update file start address
    HAL_FLASH_Unlock();
    /* Erasing application FLASH sectors */
    #ifdef BOOTLOADER_DEBUG
	    printf("Erasing: 0%%");
    #endif            
    FLASH_Erase_Sector(FLASH_SECTOR_4, FLASH_VOLTAGE_RANGE_3); 
    #ifdef BOOTLOADER_DEBUG
	    printf("\b\b25%%");
    #endif       
    FLASH_Erase_Sector(FLASH_SECTOR_5, FLASH_VOLTAGE_RANGE_3);
    #ifdef BOOTLOADER_DEBUG
	    printf("\b\b\b50%%");
    #endif       
    FLASH_Erase_Sector(FLASH_SECTOR_6, FLASH_VOLTAGE_RANGE_3);
    #ifdef BOOTLOADER_DEBUG
	    printf("\b\b\b75%%");
    #endif      
    FLASH_Erase_Sector(FLASH_SECTOR_7, FLASH_VOLTAGE_RANGE_3); 
    #ifdef BOOTLOADER_DEBUG
	    printf("\b\b\b100%%\n");
      printf("Flashing:    ");
    #endif 
          
    for(uint32_t idx = APP_START_ADDR; idx <= APP_START_ADDR+file_len; idx += 8)
    {
      uint64_t buf;
      W25RXX_ReadBytes(buf, NVM_Addr, 8); //Read 8 bytes from NVM
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, idx, buf) == HAL_OK) //Program 8 bytes of flash
      {
        #ifdef BOOTLOADER_DEBUG
          uint8_t CurrPercent = 100-((((APP_START_ADDR+file_len)-idx)*100)/file_len);
          printf("\b\b\b%d%%", CurrPercent);
        #endif 
        NVM_Addr += 8; 
      }
      else
      {
        char* error = calloc(128, sizeof(char));
        strcat(error, HAL_FLASH_GetError);
        WriteLog(error);
        Error_Handler(); //If HAL_!OK goto error handler
      }
    }
    #ifdef BOOTLOADER_DEBUG
	    printf("\nFlash is complete.\n");
      printf("Verifying:    ");
    #endif 
    HAL_FLASH_Lock();
    for(uint32_t idx = APP_START_ADDR; idx <= APP_START_ADDR+file_len; idx += 4)
    {
      uint32_t buf;
      W25RXX_ReadBytes(buf, NVM_Addr, 4); //Read 8 bytes from NVM
      uint32_t result = *(__IO uint32_t *)idx;
      if (result == buf) //Check
      {
        #ifdef BOOTLOADER_DEBUG
          uint8_t CurrPercent = 1-((((APP_START_ADDR+file_len)-idx)*100)/file_len);
          printf("\b\b\b%d%%", CurrPercent);
        #endif 
        NVM_Addr += 4; 
      }
      else
      {
        ODIN_Err(6); //If flash was incorrect - goto err_handler
      }
    }
    #ifdef BOOTLOADER_DEBUG
	    printf("\nVerify complete.\n");
    #endif     
}
void ODIN_SOS(void)
{
    WriteLog(SOS_EVENT);
    #ifdef BOOTLOADER_DEBUG
	    printf("Device in SOS mode. Further recovery is not longer possible. Device is bricked. Refer to documentation\n");
    #endif
    gsm_gprs_mqttConnect(MQTT_SERVER, 1884, false, dev_cfg.SN, 60, MQTT_USER, MQTT_PSWD, 60);
    char* message = calloc(256, sizeof(char));
    strcat(message, "{\"SN\":\"");
    strcat(message, dev_cfg.SN);
    strcat(message, "\"}");
    gsm_gprs_mqttPublish("$device/SOS", true, false, message);
    free(message);
    gsm_gprs_mqttDisConnect();
    #ifndef BOOTLOADER_DEBUG
      while(1)
      {
        printf("Device in SOS mode. Further recovery is not longer possible. Device is bricked. Refer to documentation\n");
        HAL_Delay(10000);
      }
    #endif

    #ifdef BOOTLOADER_DEBUG
	    return;
    #endif  
    //SOS Message. Device connects to MQTT Server and send to topic $device/sos a SOS message.
}
void GotoApp(void)
{
  uint32_t jumpAddress = *(__IO uint32_t*) (APP_START_ADDR + 4); 
  pFunction Jump_To_Application = (pFunction) jumpAddress;
 
  //Maybe change all that huynya to HAL_DeInit()?
  HAL_DeInit();
  //MX_RCC_DeInit();
  //MX_SPI1_DeInit();
  //MX_FLASH_DeInit();
  //HAL_I2C_DeInit();
  //MX_IRQ_DeInit();
  //MX_GPIO_DeInit();
  
  __disable_irq();
  __set_MSP(*(__IO uint32_t*) APP_START_ADDR);
  __enable_irq();

  SCB -> ICSR = 0x00000000;   // reset value;
  SCB -> SCR = 0;
  SCB -> CCR = 0x00000200;    // reset value
  SCB -> SHP[0] = 0;
  SCB -> SHCSR = 0;
  SCB -> CFSR = (SCB_CFSR_DIVBYZERO_Msk | SCB_CFSR_UNALIGNED_Msk | SCB_CFSR_UNDEFINSTR_Msk | SCB_CFSR_NOCP_Msk | SCB_CFSR_INVPC_Msk | SCB_CFSR_INVSTATE_Msk);
  SCB -> HFSR = (SCB_HFSR_DEBUGEVT_Msk | SCB_HFSR_FORCED_Msk | SCB_HFSR_VECTTBL_Msk);
  SCB -> VTOR = APP_START_ADDR;

  SysTick -> CTRL = 0;
  SysTick -> LOAD = 0;
  SysTick -> VAL  = 0; 

  Jump_To_Application(); 
}
void WriteBootSection(uint32_t ver_head_addr, uint32_t len_addr, uint32_t crc_addr)
{
    #ifdef BOOTLOADER_DEBUG
	    printf("Copying information to boot section...\n");
      printf("Erasing boot section...\n");
    #endif
    W25RXX_EraseSector(BOOT_SEC_ADDR); //Erasing BOOT_INFO Sector
    #ifdef BOOTLOADER_DEBUG
	    printf("Copying version header...\n");
    #endif    
    char app_ver_head [31];
    W25RXX_ReadBytes(app_ver_head, ver_head_addr, 32);
    W25RXX_WriteSector(app_ver_head,APP_VER_HEAD_ADDR,0,32);
    #ifdef BOOTLOADER_DEBUG
      printf("Copying application length...\n");
    #endif
    uint32_t app_len;
    W25RXX_ReadBytes(app_len, len_addr, 4);
    W25RXX_WriteSector(app_len,APP_LEN_ADDR,0,4);
    #ifdef BOOTLOADER_DEBUG
      printf("Copying checksum...\n");
    #endif
    uint32_t app_crc;
    W25RXX_ReadBytes(app_crc, crc_addr, 4);
    W25RXX_WriteSector(app_crc,APP_CRC_ADDR,0,4);
    #ifdef BOOTLOADER_DEBUG
      printf("Copying to boot section is complete.\n");
    #endif 
}
bool getFlag(flag_t flag)
{
  uint8_t flag_stat;
  W25RXX_ReadByte(flag_stat, FLAGS_ADDR + flag);
  return (bool)flag_stat;
}
/**
 * @brief Creating log entry with particular event and write it to NVM.
 * 
 * @param event Body of the log
 */
void WriteLog(char* event)
{
  #ifdef BOOTLOADER_DEBUG
    printf("WriteLog() callled\n");
  #endif   
  char* log_string = calloc(512, sizeof(char));
  char* timestamp = calloc(512, sizeof(char));
  gsm_gprs_ntpGetTime(timestamp);
  strcat(log_string, timestamp);
  free(timestamp);
  strcat(log_string, "-BLR-");
  strcat(log_string, BLR_VERSION);
  strcat(log_string, ":");
  strcat(log_string, "\"");
  strcat(log_string, event);
  strcat(log_string, "\"\n\0");
  uint32_t last_addr;
  //#ifdef BOOTLOADER_DEBUG
    printf("LOG: %s\n", log_string);
  //#endif   
  W25RXX_ReadBytes(last_addr, LOGS_ADDR, 4);
  #ifdef BOOTLOADER_DEBUG
    printf("Last log at address %x\n", last_addr);
  #endif   
  uint32_t len = strlen(log_string);
  uint16_t i = 0;
  for(uint32_t idx = last_addr; idx <= idx + len; idx++)
  {
    W25RXX_WriteByte(log_string[i], idx);
    i++;
  }  
  uint32_t new_addr = last_addr + len + 1;
  uint8_t buf_addr [3];
  buf_addr[3] = (uint8_t)new_addr;
  buf_addr[2] = (uint8_t)(new_addr>>8);
  buf_addr[1] = (uint8_t)(new_addr>>16);
  buf_addr[0] = (uint8_t)(new_addr>>24);
  uint16_t k = 0;
  for(uint32_t idx = LOGS_ADDR; idx <= idx + 4; idx++)
  {
    W25RXX_WriteByte(buf_addr[i], idx);
    k++;
  }
  #ifdef BOOTLOADER_DEBUG
    printf("New last log address at %x\n", new_addr);
  #endif    
  free(log_string);
}
/**
 * @brief Function, which get certificates from NVM addresses in #define, write them to modem FS and install them.
 * 
 * @return true - successful certs install
 * @return false - unsuccessful certs install (catching only modem FS and cert-install errors)
 */
bool CertsUpdate(void)
{
  #ifdef BOOTLOADER_DEBUG
    printf("Starting certificates update. Allocating memory for buffer...\n");
  #endif    
  char* cert = calloc(2048, sizeof(char));
  #ifdef BOOTLOADER_DEBUG
    printf("Memory allocation successful. Reading root certificate:\n");
  #endif    
  uint32_t idx = ROOT_CERT_ADDR;
  bool file_read = false;
  while(!file_read)
  {
    char buf;
    W25RXX_ReadByte(buf, idx);
    if(buf == '\0')
    {
      file_read = true;
      break;
    }
    #ifdef BOOTLOADER_DEBUG
      printf(buf);
    #endif 
    strcat(cert, buf);
    idx++;
  }
  #ifdef BOOTLOADER_DEBUG
    printf("Root certificate has been read. Creating certificate file in modem FS...\n");
  #endif 
  if(!gsm_fs_create("root_cert.crt"))
  {
    #ifdef BOOTLOADER_DEBUG
      printf("File creation was failed. Exit certificate update...\n");
    #endif 
    return false;
  }
  #ifdef BOOTLOADER_DEBUG
    printf("Certificate file was successfully created. Writing certificate file in modem FS...\n");
  #endif 
  if(!gsm_fs_write("root_cert.crt", false, cert))
  {
    #ifdef BOOTLOADER_DEBUG
      printf("File writing was failed. Exit certificate update...\n");
    #endif 
    return false;
  }
  #ifdef BOOTLOADER_DEBUG
    printf("Certificate file was successfully written. Erasing buffer and reading client certificate:\n");
  #endif 
  memset(cert, '\0', 2048);
  idx = CLIENT_CERT_ADDR;
  file_read = false;
  while(!file_read)
  {
    char buf;
    W25RXX_ReadByte(buf, idx);
    if(buf == '\0')
    {
      file_read = true;
      break;
    }
    #ifdef BOOTLOADER_DEBUG
      printf(buf);
    #endif 
    strcat(cert, buf);
    idx++;
  }
  #ifdef BOOTLOADER_DEBUG
    printf("Client certificate has been read. Creating certificate file in modem FS...\n");
  #endif   
  if(!gsm_fs_create("client_cert.crt"))
  {
    #ifdef BOOTLOADER_DEBUG
      printf("File creation was failed. Exit certificate update...\n");
    #endif 
    return false;
  }
  #ifdef BOOTLOADER_DEBUG
    printf("Certificate file was successfully created. Writing certificate file in modem FS...\n");
  #endif 
  if(!gsm_fs_write("client_cert.crt", false, cert))
  {
    #ifdef BOOTLOADER_DEBUG
      printf("File writing was failed. Exit certificate update...\n");
    #endif 
    return false;
  }
  free(cert);
  #ifdef BOOTLOADER_DEBUG
    printf("Certificate file was successfully written. Installing certificates to modem...\n");
  #endif 
  if(!gsm_gprs_certs("root_cert.crt", "client_cert.crt"))
  {
    #ifdef BOOTLOADER_DEBUG
      printf("Certificates install was failed. Exit certificate update...\n");
    #endif 
    return false;
  }
  #ifdef BOOTLOADER_DEBUG
    printf("Certificates was successfully installed. Erasing certs in NVM...\n");
  #endif  
  W25RXX_EraseSector(ROOT_CERT_ADDR);
  W25RXX_EraseSector(ROOT_CERT_ADDR + 4096);
  W25RXX_EraseSector(CLIENT_CERT_ADDR);
  W25RXX_EraseSector(CLIENT_CERT_ADDR + 4096);
  #ifdef BOOTLOADER_DEBUG
    printf("Resetting CERTS_UPDATE_FLAG...\n");
  #endif
  setFlag(CERTS_UPDATE_FLAG, false);
  #ifdef BOOTLOADER_DEBUG
    printf("Certificates update was successful.\n");
  #endif   
  return true;
}
/**
 * @brief Perform factory by erasing sectors and reset FACTORY_RESET_FLAG
 * 
 * @retval None
 */
void FactoryReset(void)
{
  #ifdef BOOTLOADER_DEBUG
    printf("Start factory reset...\n");
  #endif      
  for(uint32_t idx = USER_INFO_START_ADDR; idx <= USER_INFO_END_ADDR; idx += 4096)
  {
    W25RXX_EraseSector(idx);
  }
  #ifdef BOOTLOADER_DEBUG
    printf("Writing log...\n");
  #endif  
  WriteLog(FACTORY_RESET_EVENT);
  #ifdef BOOTLOADER_DEBUG
    printf("Resetting FACTORY_RESET_FLAG...\n");
  #endif  
  setFlag(FACTORY_RESET_FLAG, false);
  #ifdef BOOTLOADER_DEBUG
    printf("Factory reset done.\n");
  #endif 
}
/**
 * @brief Print log from NVM
 * 
 */
void PrintLog(void)
{
  #ifdef BOOTLOADER_DEBUG
    printf("Starting printing log...\n");
  #endif 
  uint32_t last_addr;
  W25RXX_ReadBytes(last_addr, last_addr, 4);
  #ifdef BOOTLOADER_DEBUG
    printf("Log end adress: %x\n", last_addr);
  #endif
  for(uint32_t idx = LOGS_ADDR + 4; idx <= last_addr; idx += 1)
  {
    char buf;
    W25RXX_ReadByte(buf, idx);
    printf(buf);
  }
}
/**
 * @brief Set flag in NVM. Refer to flags decription.
 * 
 * @param flag Flag name
 * @param value
 */
void setFlag(flag_t flag, bool value)
{
  #ifdef BOOTLOADER_DEBUG
    printf("Setting flag at address %x...", FLAGS_ADDR + flag);
  #endif    
  W25RXX_WriteSector((uint8_t)value, FLAGS_ADDR + flag, 0, 1);
  #ifdef BOOTLOADER_DEBUG
    printf("Set value %d", value);
  #endif    
}
/**
 * @brief Reading UDCfg from OTP Memory (incl. UCID)
 * 
 * @retval None
 */
void ReadUDCfg(void)
{
  #ifdef BOOTLOADER_DEBUG
    printf("Reading UDCfg...");
  #endif
  for(uint32_t idx = FLASH_OTP_BASE; idx < FLASH_OTP_BASE + VERSION; idx += 4)
  {
    uint8_t buf = *(__IO uint8_t *)idx;
    strcat(dev_cfg.Version, (char*)buf);
  }
  strcat(dev_cfg.Version, '\0');
  #ifdef BOOTLOADER_DEBUG
    printf("UDCfg version %s...", dev_cfg.Version);
  #endif      
  for(uint32_t idx = FLASH_OTP_BASE + VERSION; idx < FLASH_OTP_BASE + SN; idx += 4)
  {
    uint8_t buf = *(__IO uint8_t *)idx;
    strcat(dev_cfg.SN, (char*)buf);
  }
  strcat(dev_cfg.SN, '\0');
  #ifdef BOOTLOADER_DEBUG
    printf("Serial Number is %s...", dev_cfg.SN);
  #endif        
  for(uint32_t idx = FLASH_OTP_BASE + SN; idx < FLASH_OTP_BASE + IMEI; idx += 4)
  {
    uint8_t buf = *(__IO uint8_t *)idx;
    strcat(dev_cfg.IMEI, (char*)buf);
  }
  strcat(dev_cfg.IMEI, '\0');  
  #ifdef BOOTLOADER_DEBUG
    printf("IMEI is %s...", dev_cfg.IMEI);
  #endif
  for(uint32_t idx = UID_BASE; idx < UID_BASE + 12; idx+=4)
  {
    uint32_t buf;
    buf = *(__IO uint32_t *)idx;
    strcat(dev_cfg.UCID, buf);
  }
  #ifdef BOOTLOADER_DEBUG
    printf("UCID is %s...", dev_cfg.UCID);
  #endif   
  for(uint32_t idx = FLASH_OTP_BASE + IMEI; idx < FLASH_OTP_BASE + HNAME; idx += 4)
  {
    uint8_t buf = *(__IO uint8_t *)idx;
    strcat(dev_cfg.HName, (char*)buf);
  }
  strcat(dev_cfg.HName, '\0');
  #ifdef BOOTLOADER_DEBUG
    printf("Hname is %s...", dev_cfg.HName);
  #endif     
  for(uint32_t idx = FLASH_OTP_BASE + HNAME; idx < FLASH_OTP_BASE + MUID; idx += 4)
  {
    uint8_t buf = *(__IO uint8_t *)idx;
    strcat(dev_cfg.MUID, (char*)buf);
  }
  strcat(dev_cfg.MUID, '\0');
  #ifdef BOOTLOADER_DEBUG
    printf("MUID is %s...", dev_cfg.MUID);
  #endif   
  for(uint32_t idx = FLASH_OTP_BASE + MUID; idx < FLASH_OTP_BASE + PMdl; idx += 4)
  {
    uint8_t buf = *(__IO uint8_t *)idx;
    strcat(dev_cfg.PMdl, (char*)buf);
  }
  strcat(dev_cfg.MUID, '\0');
  #ifdef BOOTLOADER_DEBUG
    printf("PMdl is %s...", dev_cfg.PMdl);
  #endif       
  for(uint32_t idx = FLASH_OTP_BASE + PMdl; idx < FLASH_OTP_BASE + DMdl; idx += 4)
  {
    uint8_t buf = *(__IO uint8_t *)idx;
    strcat(dev_cfg.DMdl, (char*)buf);
  }
  strcat(dev_cfg.PMdl, '\0');
  #ifdef BOOTLOADER_DEBUG
    printf("DMdl is %s...", dev_cfg.DMdl);
  #endif     
  for(uint32_t idx = FLASH_OTP_BASE + DMdl; idx < FLASH_OTP_BASE + UDID; idx += 4)
  {
    uint8_t buf = *(__IO uint8_t *)idx;
    strcat(dev_cfg.UDID, (char*)buf);
  }
  strcat(dev_cfg.DMdl, '\0');   
  #ifdef BOOTLOADER_DEBUG
    printf("UDID is %s...", dev_cfg.UDID);
  #endif         
  for(uint32_t idx = FLASH_OTP_BASE + UDID; idx < FLASH_OTP_BASE + DCfg; idx += 4)
  {
    uint8_t buf = *(__IO uint8_t *)idx;
    strcat(dev_cfg.DCfg, (char*)buf);
  }
  strcat(dev_cfg.DCfg, '\0');  
  #ifdef BOOTLOADER_DEBUG
    printf("DCfg is %s...", dev_cfg.DCfg);
  #endif                  
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  ODIN_Err(1);
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
