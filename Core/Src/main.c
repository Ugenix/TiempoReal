/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "ds1307.h"
#include "stdio.h"

#include "ssd1306.h"
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
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for mostrarLCD */
osThreadId_t mostrarLCDHandle;
const osThreadAttr_t mostrarLCD_attributes = {
  .name = "mostrarLCD",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for lectura */
osThreadId_t lecturaHandle;
const osThreadAttr_t lectura_attributes = {
  .name = "lectura",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for setRGB */
osThreadId_t setRGBHandle;
const osThreadAttr_t setRGB_attributes = {
  .name = "setRGB",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for OLED_TASKHAndle */
osThreadId_t OLED_TASKHAndleHandle;
const osThreadAttr_t OLED_TASKHAndle_attributes = {
  .name = "OLED_TASKHAndle",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for datos */
osMessageQueueId_t datosHandle;
const osMessageQueueAttr_t datos_attributes = {
  .name = "datos"
};
/* Definitions for lcdDisplayFlag */
osEventFlagsId_t lcdDisplayFlagHandle;
const osEventFlagsAttr_t lcdDisplayFlag_attributes = {
  .name = "lcdDisplayFlag"
};
/* Definitions for rgbModeFlag */
osEventFlagsId_t rgbModeFlagHandle;
const osEventFlagsAttr_t rgbModeFlag_attributes = {
  .name = "rgbModeFlag"
};
/* Definitions for myEvent01 */
osEventFlagsId_t myEvent01Handle;
const osEventFlagsAttr_t myEvent01_attributes = {
  .name = "myEvent01"
};
/* USER CODE BEGIN PV */
LCD_t lcd;
uint8_t sec,min,hour;
uint8_t week_day,day,month,year;
char str[16], msg[64];

uint8_t buffer[8];
char buff[8];
int data = 0;
char mensaje[2];
uint8_t H_data = 0;

uint8_t set_hora[3];
uint8_t set_date[4];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C3_Init(void);
void StartDefaultTask(void *argument);
void MostrarDatosLCD(void *argument);
void Lectura(void *argument);
void SetRGB(void *argument);
void StartTask06(void *argument);

/* USER CODE BEGIN PFP */
void ssd1306_InitText();
void lcd_config(void);
void lcd_rgb_init(LCD_t *lcd_dir);
void ds1307_init(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void set_RGB(uint8_t red, uint8_t green, uint8_t blue) //??????En el modulo esta cambiado R por G????
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, blue);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, green);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, red);
}
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
  lcd_config();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_I2C3_Init();
  /* USER CODE BEGIN 2 */
  ssd1306_Init();
  lcd_rgb_init(&lcd);
  ds1307_init();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of datos */
  datosHandle = osMessageQueueNew (32, sizeof(float), &datos_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of mostrarLCD */
  mostrarLCDHandle = osThreadNew(MostrarDatosLCD, NULL, &mostrarLCD_attributes);

  /* creation of lectura */
  lecturaHandle = osThreadNew(Lectura, NULL, &lectura_attributes);

  /* creation of setRGB */
  setRGBHandle = osThreadNew(SetRGB, NULL, &setRGB_attributes);

  /* creation of OLED_TASKHAndle */
  OLED_TASKHAndleHandle = osThreadNew(StartTask06, NULL, &OLED_TASKHAndle_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of lcdDisplayFlag */
  lcdDisplayFlagHandle = osEventFlagsNew(&lcdDisplayFlag_attributes);

  /* creation of rgbModeFlag */
  rgbModeFlagHandle = osEventFlagsNew(&rgbModeFlag_attributes);

  /* creation of myEvent01 */
  myEvent01Handle = osEventFlagsNew(&myEvent01_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  osEventFlagsSet(lcdDisplayFlagHandle, 0x00000001U);
  osEventFlagsSet(rgbModeFlagHandle, 0x00000080U);
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|RGB_green_Pin|RGB_red_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_EN_Pin|LCD_RS_Pin|D7_Pin|D6_Pin
                          |D5_Pin|D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RGB_blue_GPIO_Port, RGB_blue_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin RGB_green_Pin RGB_red_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|RGB_green_Pin|RGB_red_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_EN_Pin LCD_RS_Pin D7_Pin D6_Pin
                           D5_Pin D4_Pin */
  GPIO_InitStruct.Pin = LCD_EN_Pin|LCD_RS_Pin|D7_Pin|D6_Pin
                          |D5_Pin|D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RGB_blue_Pin */
  GPIO_InitStruct.Pin = RGB_blue_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RGB_blue_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BotonDisplay_Pin BotonRGB_Pin */
  GPIO_InitStruct.Pin = BotonDisplay_Pin|BotonRGB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void lcd_config(void) {
	lcd.RS_port= LCD_RS_GPIO_Port;
	lcd.RS_pin = LCD_RS_Pin;
	//lcd.RW_port = LCD_RW_GPIO_Port;
	//lcd.RW_pin = LCD_RW_Pin;
	lcd.EN_port = LCD_EN_GPIO_Port;
	lcd.EN_pin = LCD_EN_Pin;
	lcd.D4_port = D4_GPIO_Port;
	lcd.D4_pin = D4_Pin;
	lcd.D5_port = D5_GPIO_Port;
	lcd.D5_pin = D5_Pin;
	lcd.D6_port = D6_GPIO_Port;
	lcd.D6_pin = D6_Pin;
	lcd.D7_port = D7_GPIO_Port;
	lcd.D7_pin = D7_Pin;
}

void lcd_rgb_init(LCD_t *lcd_dir) {
	lcd_begin(lcd_dir, 16, 2, LCD_5x8DOTS);

	// Mensaje inicial
	lcd_setCursor(lcd_dir, 0, 0);
	lcd_print(lcd_dir, "  Tiempo Real   ");
	lcd_setCursor(lcd_dir, 0, 1);
	lcd_print(lcd_dir, "  Xiang & Alex  ");
	set_RGB(1,1,1);
	HAL_Delay(3000);
	lcd_clear(lcd_dir);
	lcd_home(lcd_dir);
}

void ds1307_init(void)
{
	rtc_init(0,1,0);

	rtc_set_time(00,00,00);
	rtc_set_date(1, 1, 1, 23);
}

void ssd1306_IniText()
{
    uint8_t y = 0;

    //ssd1306_Init();
    ssd1306_Fill(Black);

	#ifdef SSD1306_INCLUDE_FONT_11x18
	ssd1306_SetCursor(10, y);
	ssd1306_WriteString("Comando:", Font_6x8, White);
	//y += 19;
	//sprintf(mensaje, "%d", data);
	ssd1306_SetCursor(75, y);
	//ssd1306_WriteString(mensaje, Font_16x26, White);
	ssd1306_WriteString(buffer, Font_11x18, White);
	y += 19;
	ssd1306_SetCursor(10, y);
	sprintf(buff ,"%d", H_data);
	ssd1306_WriteString(buff, Font_16x26, White);
	#endif

    ssd1306_UpdateScreen();
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
	int comand = 0;
	/*if(buffer[0] >= 48 && buffer[0] <= 57 && buffer[1] >= 48 && buffer[1] <= 57){
		data = (buffer[0] - 48);
		data = (data * 10);
		data += (buffer[1] - 48);
		osEventFlagsSet(myEvent01Handle, 0x00000001U);
	}else{
		__NOP();
	}*/
	if(buffer[1] == 58)
	{
		if(buffer[2] >= 48 && buffer[2] <= 57) H_data = (buffer[2] - 48); //decimal
		if(buffer[3] >= 48 && buffer[3] <= 57){H_data = (H_data * 10); H_data += (buffer[3] - 48);} //unidad

		comand = buffer[0];
		switch(comand){
		case 72: //H-->hora
			set_hora[0] = H_data;
			break;
		case 77: //M-->min
			set_hora[1] = H_data;
			break;
		case 83: //S-->seg
			set_hora[2] = H_data;
			break;
		case 100: // d-->dia
			set_date[0] = H_data;
			break;
		case 109: // m-->mes
			set_date[1] = H_data;
			break;
		case 121: // y-->year
			set_date[2] = H_data;
			break;
		case 119: // w-->week day
			set_date[3] = H_data;
			break;
		}
	}

	if(1) rtc_set_time(set_hora[0], set_hora[1], set_hora[2]);
	else rtc_set_date(set_date[3], set_date[0], set_date[1], set_date[2]);

	osEventFlagsSet(myEvent01Handle, 0x00000001U);
	HAL_UART_Receive_IT(&huart2, buffer, 5);
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef * huart)
{
	//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_pin)
{
	if(GPIO_pin == GPIO_PIN_11) //Boton RGB
	{
		switch(osEventFlagsGet(rgbModeFlagHandle))
		{
			case 0x00000080U:
				osEventFlagsClear(rgbModeFlagHandle, 0x00000080U);
				osEventFlagsSet(rgbModeFlagHandle, 0x00000001U);
				break;
			case 0x00000001U:
				osEventFlagsClear(rgbModeFlagHandle, 0x00000001U);
				osEventFlagsSet(rgbModeFlagHandle, 0x00000002U);
				break;
			case 0x00000002U:
				osEventFlagsClear(rgbModeFlagHandle, 0x00000002U);
				osEventFlagsSet(rgbModeFlagHandle, 0x00000004U);
				break;
			case 0x00000004U:
				osEventFlagsClear(rgbModeFlagHandle, 0x00000004U);
				osEventFlagsSet(rgbModeFlagHandle, 0x00000008U);
				break;
			case 0x00000008U:
				osEventFlagsClear(rgbModeFlagHandle, 0x00000008U);
				osEventFlagsSet(rgbModeFlagHandle, 0x00000010U);
				break;
			case 0x00000010U:
				osEventFlagsClear(rgbModeFlagHandle, 0x00000010U);
				osEventFlagsSet(rgbModeFlagHandle, 0x00000020U);
				break;
			case 0x00000020U:
				osEventFlagsClear(rgbModeFlagHandle, 0x000000020U);
				osEventFlagsSet(rgbModeFlagHandle, 0x00000040U);
				break;
			case 0x00000040U:
				osEventFlagsClear(rgbModeFlagHandle, 0x00000040U);
				osEventFlagsSet(rgbModeFlagHandle, 0x00000080U);
				break;
		}
	}
	if(GPIO_pin == GPIO_PIN_10) //Boton display
	{
		if((osEventFlagsGet(lcdDisplayFlagHandle))==0x00000001U)
		{
			lcd_noDisplay(&lcd);
			osEventFlagsClear(lcdDisplayFlagHandle, 0x00000001U);
			osEventFlagsSet(lcdDisplayFlagHandle, 0x00000002U);
		}
		else
		{
			lcd_display(&lcd);
			osEventFlagsClear(lcdDisplayFlagHandle, 0x00000002U);
			osEventFlagsSet(lcdDisplayFlagHandle, 0x00000001U);
		}
	}
	else __NOP();
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  HAL_UART_Receive_IT(&huart2, buffer, 5);
  /* Infinite loop */
  for(;;)
  {
    osEventFlagsWait(myEvent01Handle, 0x00000002U, osFlagsNoClear, osWaitForever);
    HAL_UART_Transmit_IT(&huart2, "Y\n", 2);
    osEventFlagsClear(myEvent01Handle, 0x00000002U);
    osDelay(10);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_MostrarDatosLCD */
/**
* @brief Function implementing the mostrarLCD thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MostrarDatosLCD */
void MostrarDatosLCD(void *argument)
{
  /* USER CODE BEGIN MostrarDatosLCD */
  float datoMostrar;
  char *dayName[] = {"LUN", "MAR", "MIE", "JUE", "VIE", "SAB", "DOM"};
  /* Infinite loop */
  for(;;)
  {
	osMessageQueueGet(datosHandle, &datoMostrar, NULL, osWaitForever);

	rtc_get_time(&hour, &min, &sec);
	rtc_get_date(&week_day, &day, &month, &year);

	lcd_home(&lcd);
	lcd_setCursor(&lcd, 0, 0);
	sprintf(str,"%02d/%02d/%02d   %s", day, month, year, dayName[week_day-1]);
	lcd_print(&lcd, str);
	lcd_setCursor(&lcd, 0, 1);
	sprintf(str,"%02d:%02d:%02d T=%2.1fC", hour, min, sec, datoMostrar);
	lcd_print(&lcd, str);

    osDelay(10);
  }
  /* USER CODE END MostrarDatosLCD */
}

/* USER CODE BEGIN Header_Lectura */
/**
* @brief Function implementing the lectura thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Lectura */
void Lectura(void *argument)
{
  /* USER CODE BEGIN Lectura */
	uint8_t temp;
	float temp_celsius;
	HAL_StatusTypeDef status;

  /* Infinite loop */
  for(;;)
  {
	HAL_ADC_Start(&hadc1);
	status = HAL_ADC_PollForConversion(&hadc1, 1);
	if(status==HAL_OK)
		temp = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	temp_celsius = (((0.001221)*temp)-0.5)/(0.01); //????mal o quemado??

	osMessageQueuePut(datosHandle, &temp_celsius, osPriorityNormal, osWaitForever);
    osDelay(10);
  }
  /* USER CODE END Lectura */
}

/* USER CODE BEGIN Header_SetRGB */
/**
* @brief Function implementing the setRGB thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SetRGB */
void SetRGB(void *argument)
{
  /* USER CODE BEGIN SetRGB */
  /* Infinite loop */
  for(;;)
  {
	switch(osEventFlagsWait(rgbModeFlagHandle, 0xFF, osFlagsNoClear, osWaitForever))
	{
		case 0x0000001U:
			set_RGB(1,0,0); // R
		    break;
		case 0x0000002U:
			set_RGB(0,1,0); // G
			break;
		case 0x00000004U:
			set_RGB(0,0,1); // B
			break;
		case 0x00000008U:
			set_RGB(1,1,0); // R+G
			break;
		case 0x00000010U:
			set_RGB(1,0,1); // R+B
			break;
		case 0x00000020U:
			set_RGB(0,1,1); // G+B
			break;
		case 0x00000040U:
			set_RGB(1,1,1); // R+G+B
			break;
		case 0x00000080U:
			set_RGB(0,0,0); // apagado
			break;
	}
    osDelay(10);
  }
  /* USER CODE END SetRGB */
}

/* USER CODE BEGIN Header_StartTask06 */
/**
* @brief Function implementing the OLED_TASKHAndle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask06 */
void StartTask06(void *argument)
{
  /* USER CODE BEGIN StartTask06 */
	ssd1306_IniText();
  /* Infinite loop */
  for(;;)
  {
	osEventFlagsWait(myEvent01Handle, 0x00000001U, osFlagsNoClear, osWaitForever);
	ssd1306_IniText();
	osEventFlagsSet(myEvent01Handle, 0x00000002U);
	osEventFlagsClear(myEvent01Handle, 0x00000001U);
    osDelay(1);
  }
  /* USER CODE END StartTask06 */
}

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
