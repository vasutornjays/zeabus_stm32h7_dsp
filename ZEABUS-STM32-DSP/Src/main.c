
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32h7xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "mdma.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

#include "common.h"
#include "abs_threshold.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
__SECTION_RAM_D2 uint32_t g_adc1_2_buffer[RAW_DATA_BUFFER_SIZE];
__SECTION_RAM_D2 uint32_t g_adc3_4_buffer[RAW_DATA_BUFFER_SIZE];
__SECTION_AXIRAM uint32_t g_adc_1_h[BUFFER_SIZE];
__SECTION_AXIRAM int g_adc_1_test[BUFFER_SIZE];
__SECTION_AXIRAM uint32_t g_adc_2_h[BUFFER_SIZE];
__SECTION_AXIRAM uint32_t g_adc_3_h[BUFFER_SIZE];
__SECTION_AXIRAM uint32_t g_adc_4_h[BUFFER_SIZE];
uint32_t g_raw_data_index;
uint32_t g_pulse_detect_index;
float g_front_thres;
int g_raw_front_thres;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

int ADC_Start(){
	if(HAL_ADC_Start_DMA(&hadc1,(uint32_t*)g_adc1_2_buffer,RAW_DATA_BUFFER_SIZE) != HAL_OK){
		  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		  // Reset LD3 (RED) if start fail
		  return 0;
	  }

	  if(HAL_ADC_Start_DMA(&hadc3,(uint32_t*)g_adc3_4_buffer,RAW_DATA_BUFFER_SIZE) != HAL_OK){
		  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		  // Reset LD3 (RED) if start fail
		  return 0;
	  }

	  return 1;
}

int TIMER_Start(){
	if(HAL_TIM_Base_Start(&htim2) != HAL_OK){
		  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		  // Reset LD3 (RED) if start fail
		  return 0;
	 }

	return 1;
}

int Set_LNA_Gain(){

	uint16_t i2c_dev_addr = 0x2F<<1; // MAX 5387 Address
	uint8_t i2c_val[2];
	i2c_val[0] = 0x13; // Set both CH
	i2c_val[1] = 168;  // VGain = 1.1 * ( g_i2c_val / 255 )

	if(HAL_I2C_Master_Transmit(&hi2c1,i2c_dev_addr,i2c_val,2,100) != HAL_OK){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		// Reset LD3 (RED) if can not set gain value
		return 0;
	}

	return 1;

}

void Wait_DMA(DMA_HandleTypeDef *hdma, int eot,int next_round){

	if(next_round == 0){
		while((RAW_DATA_BUFFER_SIZE - (uint16_t)(((DMA_Stream_TypeDef   *)hdma->Instance)->NDTR)) <= eot);
	} else if(next_round == 1) {
		while((uint16_t)(((DMA_Stream_TypeDef   *)hdma->Instance)->NDTR) == 0);
		while((RAW_DATA_BUFFER_SIZE - (uint16_t)(((DMA_Stream_TypeDef   *)hdma->Instance)->NDTR)) <= eot);
	}

}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_DMA_Init();
  MX_MDMA_Init();
  MX_ADC1_Init();
  MX_ADC3_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);

  TIMER_Start(); 	// Start Timer

  ADC_Start();		// Start ADC with DMA

  Set_LNA_Gain();	// Set LNA GAIN

  g_front_thres = 0.1;	// set front threshold
  g_raw_front_thres = (g_front_thres * VOLT_RATIO) + 32768;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);

	  if(abs_threshold() == 1){
		  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
		  Get_Frame_Handler();
		  HAL_Delay(1);
	  }
	  else{
//		  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
	  }

	  g_raw_data_index = g_raw_data_index + 2;

	  if(g_raw_data_index >= RAW_DATA_BUFFER_SIZE){
		  g_raw_data_index = 0;
	  }
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Supply configuration update enable 
    */
  MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

    /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) 
  {
    
  }
    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
                              |RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 16;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_USB
                              |RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(SystemCoreClock/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

 void Get_Frame_Handler(){

//	 int pluse_header_index = 0;
	 int start_transfer_index = 0;
	 int end_transfer_index = 0;
	 int size_remain = 0;					//size of data that remain to transfer
	 int size_forward_transfer = 0;			//size of data can forward transfer before last address of raw data buffer
	 int next_round = 0;

	 // Case 1 ; some header is at the and of buffer
	 if (g_pulse_detect_index < PULSE_HEADER_SIZE - 1) {
		 size_forward_transfer = PULSE_HEADER_SIZE - g_pulse_detect_index - 1;
		 start_transfer_index = RAW_DATA_BUFFER_SIZE - size_forward_transfer; 	// start transfer at bottom of raw data
	 }
	 // Case 2 ; some body is at the next start of buffer
	 else if (RAW_DATA_BUFFER_SIZE - g_pulse_detect_index < (int) PULSE_BODY_SIZE) {
		 size_forward_transfer = PULSE_HEADER_SIZE + ((int)RAW_DATA_BUFFER_SIZE - g_pulse_detect_index);
		 start_transfer_index = g_pulse_detect_index - PULSE_HEADER_SIZE;
	 }
	 // Case 3 ; all frame is in this buffer lenght
	 else {
		 size_forward_transfer = (int) PULSE_FRAME_SIZE;
		 start_transfer_index = g_pulse_detect_index - PULSE_HEADER_SIZE;
	 }

	 size_remain = PULSE_FRAME_SIZE - size_forward_transfer;

	 if (size_remain == 0) {
		 end_transfer_index = start_transfer_index + (int) PULSE_FRAME_SIZE;
		 Wait_DMA(&hdma_adc1,end_transfer_index,next_round);
	 }
	 else {
		 end_transfer_index = size_remain;
		 next_round = 1;
	 }

	 Wait_DMA(&hdma_adc1,end_transfer_index,next_round);  		 // wait until frame is fill up

	 HAL_MDMA_Start(&hmdma_mdma_channel0_sw_0,(uint32_t)&g_adc1_2_buffer[start_transfer_index],(uint32_t)&g_adc_1_h,4,size_forward_transfer);
	 HAL_MDMA_Start(&hmdma_mdma_channel1_sw_0,(uint32_t)&g_adc1_2_buffer[start_transfer_index + 1],(uint32_t)&g_adc_2_h,4,size_forward_transfer);
	 HAL_MDMA_Start(&hmdma_mdma_channel2_sw_0,(uint32_t)&g_adc3_4_buffer[start_transfer_index],(uint32_t)&g_adc_3_h,4,size_forward_transfer);
	 HAL_MDMA_Start(&hmdma_mdma_channel3_sw_0,(uint32_t)&g_adc3_4_buffer[start_transfer_index + 1],(uint32_t)&g_adc_4_h,4,size_forward_transfer);

	 HAL_MDMA_Abort(&hmdma_mdma_channel0_sw_0);
	 HAL_MDMA_Abort(&hmdma_mdma_channel1_sw_0);
	 HAL_MDMA_Abort(&hmdma_mdma_channel2_sw_0);
	 HAL_MDMA_Abort(&hmdma_mdma_channel3_sw_0);

	 if(size_remain > 0){

		 HAL_MDMA_Start(&hmdma_mdma_channel0_sw_0,(uint32_t)&g_adc1_2_buffer,(uint32_t)&g_adc_1_h[size_forward_transfer],4,size_forward_transfer);
		 HAL_MDMA_Start(&hmdma_mdma_channel1_sw_0,(uint32_t)&g_adc1_2_buffer[1],(uint32_t)&g_adc_2_h[size_forward_transfer],4,size_forward_transfer);
		 HAL_MDMA_Start(&hmdma_mdma_channel2_sw_0,(uint32_t)&g_adc3_4_buffer,(uint32_t)&g_adc_3_h[size_forward_transfer],4,size_forward_transfer);
		 HAL_MDMA_Start(&hmdma_mdma_channel3_sw_0,(uint32_t)&g_adc3_4_buffer[1],(uint32_t)&g_adc_4_h[size_forward_transfer],4,size_forward_transfer);

	 	 HAL_MDMA_Abort(&hmdma_mdma_channel0_sw_0);
		 HAL_MDMA_Abort(&hmdma_mdma_channel1_sw_0);
		 HAL_MDMA_Abort(&hmdma_mdma_channel2_sw_0);
		 HAL_MDMA_Abort(&hmdma_mdma_channel3_sw_0);
	 }

	 for(int i = 0; i < BUFFER_SIZE; i++){
		 g_adc_1_test[i] = g_adc_1_h[i] - 32768;
	 }
	 HAL_Delay(1);
	 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);


 }

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
