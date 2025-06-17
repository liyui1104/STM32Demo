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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "oled.h"

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
  /* USER CODE BEGIN 2 */

  // Initialize OLED screen module
  OLED_Init();

  // Display the character 'A' at position (0, 0) with a font size of 8x16 dots
  OLED_ShowChar(0, 0, 'A', OLED_8X16);

  // Display the string "Hello World!" at position (16, 0) with a font size of 8x16 dots
  OLED_ShowString(16, 0, "Hello World!", OLED_8X16);

  // Display the character 'A' at position (0, 18) with a font size of 6x8 dots
  OLED_ShowChar(0, 18, 'A', OLED_6X8);

  // Display the string "Hello World!" at position (16, 18) with a font size of 6x8 dots
  OLED_ShowString(16, 18, "Hello World!", OLED_6X8);

  // Display the number 12345 at position (0, 28) with a length of 5 and a font size of 6x8 dots
  OLED_ShowNum(0, 28, 12345, 5, OLED_6X8);

  // Display the signed number -66 at position (40, 28) with a length of 2 and a font size of 6x8 dots
  OLED_ShowSignedNum(40, 28, -66, 2, OLED_6X8);

  // Display the hexadecimal number 0xA5A5 at position (70, 28) with a length of 4 and a font size of 6x8 dots
  OLED_ShowHexNum(70, 28, 0xA5A5, 4, OLED_6X8);

  // Display the binary number 0xA5 at position (0, 38) with a length of 8 and a font size of 6x8 dots
  OLED_ShowBinNum(0, 38, 0xA5, 8, OLED_6X8);

  // Display the floating-point number 123.45 at position (60, 38) with an integer part length of 3, a fractional part length of 2, and a font size of 6x8 dots
  OLED_ShowFloatNum(60, 38, 123.45, 3, 2, OLED_6X8);

  // Display the English and Chinese string "Hello,世界。" at position (0, 48), supporting mixed English and Chinese text
  OLED_ShowString(0, 48, "Hello,世界。", OLED_8X16);

  // Display an image at position (96, 48) with a width of 16 pixels and a height of 16 pixels. The image data is from the Diode array
  OLED_ShowImage(96, 48, 16, 16, Diode);

  // Print a formatted string at position (96, 18) with a font size of 6x8 dots. The formatted string is "[%02d]"
  OLED_Printf(96, 18, OLED_6X8, "[%02d]", 6);

  // Call the OLED_Update function to update the content of the OLED display memory array to the OLED hardware for display
  OLED_Update();

  // Delay for 3000ms to observe the phenomenon
  HAL_Delay(3000);

  // Clear the OLED display memory array
  OLED_Clear();

  // Draw a point at position (5, 8)
  OLED_DrawPoint(5, 8);

  // Get the point at position (5, 8)
  if (OLED_GetPoint(5, 8))
  {
    // If the specified point is lit, display the string "YES" at position (10, 4) with a font size of 6x8 dots
    OLED_ShowString(10, 4, "YES", OLED_6X8);
  }
  else
  {
    // If the specified point is not lit, display the string "NO " at position (10, 4) with a font size of 6x8 dots
    OLED_ShowString(10, 4, "NO ", OLED_6X8);
  }

  // Draw a straight line between positions (40, 0) and (127, 15)
  OLED_DrawLine(40, 0, 127, 15);

  // Draw a straight line between positions (40, 15) and (127, 0)
  OLED_DrawLine(40, 15, 127, 0);

  // Draw a rectangle at position (0, 20) with a width of 12 pixels and a height of 15 pixels, unfilled
  OLED_DrawRectangle(0, 20, 12, 15, OLED_UNFILLED);

  // Draw a rectangle at position (0, 40) with a width of 12 pixels and a height of 15 pixels, filled
  OLED_DrawRectangle(0, 40, 12, 15, OLED_FILLED);

  // Draw a triangle between positions (20, 20), (40, 25), and (30, 35), unfilled
  OLED_DrawTriangle(20, 20, 40, 25, 30, 35, OLED_UNFILLED);

  // Draw a triangle between positions (20, 40), (40, 45), and (30, 55), filled
  OLED_DrawTriangle(20, 40, 40, 45, 30, 55, OLED_FILLED);

  // Draw a circle at position (55, 27) with a radius of 8 pixels, unfilled
  OLED_DrawCircle(55, 27, 8, OLED_UNFILLED);

  // Draw a circle at position (55, 47) with a radius of 8 pixels, filled
  OLED_DrawCircle(55, 47, 8, OLED_FILLED);

  // Draw an ellipse at position (82, 27) with a horizontal semi-axis of 12 pixels and a vertical semi-axis of 8 pixels, unfilled
  OLED_DrawEllipse(82, 27, 12, 8, OLED_UNFILLED);

  // Draw an ellipse at position (82, 47) with a horizontal semi-axis of 12 pixels and a vertical semi-axis of 8 pixels, filled
  OLED_DrawEllipse(82, 47, 12, 8, OLED_FILLED);

  // Draw an arc at position (110, 18) with a radius of 15 pixels, a start angle of 25 degrees, and an end angle of 125 degrees, unfilled
  OLED_DrawArc(110, 18, 15, 25, 125, OLED_UNFILLED);

  // Draw an arc at position (110, 38) with a radius of 15 pixels, a start angle of 25 degrees, and an end angle of 125 degrees, filled
  OLED_DrawArc(110, 38, 15, 25, 125, OLED_FILLED);

  // Call the OLED_Update function to update the content of the OLED display memory array to the OLED hardware for display
  OLED_Update();

  // Delay for 3000ms to observe the phenomenon
  HAL_Delay(3000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    for (uint8_t i = 0; i < 4; i ++)
    {
      // Invert part of the data in the OLED display memory array, starting from position (0, i * 16) with a width of 128 pixels and a height of 16 pixels
      OLED_ReverseArea(0, i * 16, 128, 16);
      
      // Call the OLED_Update function to update the content of the OLED display memory array to the OLED hardware for display
      OLED_Update();
      
      // Delay for 1000ms to observe the phenomenon
      HAL_Delay(1000);
      
      // Invert the inverted content back
      OLED_ReverseArea(0, i * 16, 128, 16);
    }
    
    // Invert all the data in the OLED display memory array
    OLED_Reverse();
    
    // Call the OLED_Update function to update the content of the OLED display memory array to the OLED hardware for display
    OLED_Update();
    
    // Delay for 1000ms to observe the phenomenon
    HAL_Delay(1000);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
