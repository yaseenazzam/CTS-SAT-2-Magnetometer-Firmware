/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void lis2mdl_run_fixed(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE BEGIN 0 */


/* USER CODE BEGIN 0 */
// ---- LIS2MDL WHO_AM_I smoke test ----
// Uses I2C1 on PB9=SDA (D14) / PB8=SCL (D15) and LPUART1 for prints.

static void uart_print(const char *s) {
  HAL_UART_Transmit(&hlpuart1, (uint8_t*)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
}

static void who_test(void) {
  const uint8_t cand[2] = {0x1E, 0x1F};   // SA1=GND -> 0x1E, SA1=VDD -> 0x1F
  char line[128];

  uart_print("\r\n--- LIS2MDL WHO probe (0x1E/0x1F) ---\r\n");
  HAL_Delay(10); // power-up settle

  uint8_t found_addr = 0;
  uint8_t who_val    = 0;
  HAL_StatusTypeDef st_rdy = HAL_ERROR, st_who = HAL_ERROR;

  for (int i = 0; i < 2; ++i) {
    uint8_t a = cand[i];

    st_rdy = HAL_I2C_IsDeviceReady(&hi2c1, a<<1, 3, 10);
    int n = snprintf(line, sizeof line, "Probe 0x%02X: %s\r\n",
                     a, (st_rdy==HAL_OK) ? "ACK" : "NACK");
    HAL_UART_Transmit(&hlpuart1, (uint8_t*)line, n, HAL_MAX_DELAY);

    if (st_rdy != HAL_OK) continue;

    who_val = 0;
    st_who = HAL_I2C_Mem_Read(&hi2c1, a<<1, 0x4F, I2C_MEMADD_SIZE_8BIT,
                              &who_val, 1, 50);
    n = snprintf(line, sizeof line, "WHO @0x%02X: status=%ld val=0x%02X\r\n",
                 a, (long)st_who, who_val);
    HAL_UART_Transmit(&hlpuart1, (uint8_t*)line, n, HAL_MAX_DELAY);

    if (st_who == HAL_OK && who_val == 0x40) {  // correct device ID
      found_addr = a;
      break;
    }
  }

  uint32_t err  = HAL_I2C_GetError(&hi2c1);
  uint32_t stat = HAL_I2C_GetState(&hi2c1);
  int n = snprintf(line, sizeof line,
                   "HAL state=0x%02lX err=0x%08lX\r\n", stat, err);
  HAL_UART_Transmit(&hlpuart1, (uint8_t*)line, n, HAL_MAX_DELAY);

  if (found_addr) {
    n = snprintf(line, sizeof line,
                 "RESULT: OK  addr=0x%02X  WHO=0x%02X\r\n",
                 found_addr, who_val);
    HAL_UART_Transmit(&hlpuart1, (uint8_t*)line, n, HAL_MAX_DELAY);
  } else if (st_rdy != HAL_OK) {
    uart_print("RESULT: NACK at both 0x1E and 0x1F (check GND, power, CS, SDA/SCL)\r\n");
  } else if (st_who != HAL_OK) {
    uart_print("RESULT: ACKed address, but WHO read failed\r\n");
  } else {
    uart_print("RESULT: WHO returned unexpected value (not 0x40)\r\n");
  }

  // Hold so you can read the message
  while (1) { HAL_Delay(1000); }
}


/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

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
  MX_LPUART1_UART_Init();
  //MX_USART3_UART_Init();
  //MX_USB_OTG_FS_PCD_Init();
  MX_I2C1_Init();
 // MX_I2C2_Init();
  who_test();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 30;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


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
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
