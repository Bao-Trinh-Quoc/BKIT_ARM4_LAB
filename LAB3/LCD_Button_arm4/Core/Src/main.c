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
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "traffic_light.h"
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
void init_system();

int IsButtonUp();
int IsButtonDown();
void TestButtonMatrix();
void TestLcd();
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
  MX_FSMC_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  init_system();

//  TestLcd();
//  led_7seg_set_digit(1, 0, 0);
//  led_7seg_set_digit(2, 1, 0);
//  led_7seg_set_digit(3, 2, 0);
//  led_7seg_set_digit(4, 3, 0);
  uint16_t mode_counter = 0;
  lcd_show_picture(80, 200, 90, 90, gImageLogo);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  while(!timer2_flag);
	  timer2_flag = 0;

	  button_scan();

	  switch(current_mode) {
		  case MODE_NORMAL:
			  fsm_normal(mode_counter);
			  // Running system timer
			  mode_counter = (mode_counter + 1) % MODE_NORMAL_TICKS;
			  // Switch mode
			  if(button_count[12] == 1){
				  HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
				  current_mode = MODE_MODIFY_RED;
				  fsm_modify_init(RED_LIGHT);
			  }
			  break;
		  case MODE_MODIFY_RED:
			  // Running system timer
			  fsm_modify_light(mode_counter, RED_LIGHT);
			  mode_counter = (mode_counter + 1) % MODE_MODIFY_TICKS;
			  // Switch mode
			  if(button_count[12] == 1){
				  HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
				  current_mode = MODE_MODIFY_GREEN;
				  fsm_modify_init(GREEN_LIGHT);
			  }
			  break;
		  case MODE_MODIFY_GREEN:
			  // Running system timer
			  mode_counter = (mode_counter + 1) % MODE_MODIFY_TICKS;
			  fsm_modify_light(mode_counter, GREEN_LIGHT);
			  // Switch mode
			  if(button_count[12] == 1){
				  HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
				  current_mode = MODE_MODIFY_YELLOW;
				  fsm_modify_init(YELLOW_LIGHT);
			  }
			  break;
		  case MODE_MODIFY_YELLOW:
			  // Running system timer
			  mode_counter = (mode_counter + 1) % MODE_MODIFY_TICKS;
			  fsm_modify_light(mode_counter, YELLOW_LIGHT);
			  // Switch mode
			  if(button_count[12] == 1){
				  HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
				  current_mode = MODE_NORMAL;
				  fsm_normal_init();
			  }
			  break;
	  }
//	  if (timer4_flag == 1) {
//		  timer4_flag = 0;
//		  led_7seg_display();
//	  }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void init_system() {
	button_init();
	led_7seg_init();
	lcd_init();
	lcd_clear(WHITE);
	traffic_light_init();

	timer2_init();
	timer2_set(50);

	timer4_init();
	timer4_set(1);

}
// Button example 1: a button is pressed 1 time (50ms).
int IsButtonDown() {
	if (button_count[0] == 1) {
		return 1;
	}
	return 0;
}
// Check if a button is pressed 1 time (50ms),
// or if it is held down for more than 1 second, in which case it returns 1 every 100ms.
int IsButtonUp() {
	if (button_count[1] == 1 || (button_count[1] > 20 && button_count[1] % 2 == 0)) {
		return 1;
	}
	return 0;
}
void TestLcd() {
	lcd_fill(0, 0, 240, 20, BLUE);
//	lcd_show_string_center(0, 2, "Hello World !!!", RED, BLUE, 16, 1);
//	lcd_show_string(20, 30, "Test LCD Screen", WHITE, RED, 24, 0);
//	lcd_draw_rectangle(20, 80, 100, 160, GREEN);
//	lcd_draw_circle(160, 120, BRED, 40, 0);
	lcd_show_picture(80, 200, 90, 90, gImageLogo);
	lcd_show_string_center(0, 2, "Lab 3", WHITE, BLUE, 16, 1);
}
void TestButtonMatrix() {
	for (int i = 0; i < 16; i++) {
		if (button_count[i] != 0) {
			lcd_show_int_num(140, 105, i, 2, BRED, WHITE, 32);
		}
	}
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
