
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f3xx_hal.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#define UART_CR1_FIELDS  ((uint32_t)(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | \
                                     USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8))
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
typedef  void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;
int nu_bytes=0,state=0;
char i_byte;
uint32_t buffer[256],address=0;
int n_page=0;
#define USER_FLASH_LAST_PAGE_ADDRESS  0x0803F800
#define USER_FLASH_END_ADDRESS        0x0803FFFF /* 256 KBytes */
#define FLASH_PAGE_SIZE               0x800      /* 2 Kbytes */
#define APPLICATION_ADDRESS     (uint32_t)0x08003000
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
HAL_StatusTypeDef tx_char(UART_HandleTypeDef*,char*);//put the byte into tx ring buffer
HAL_StatusTypeDef rx_char(UART_HandleTypeDef*,char*);//get byte from rx ring buffer
HAL_StatusTypeDef UART_T_IT(UART_HandleTypeDef *huart);//interrupt handler for tx
HAL_StatusTypeDef UART_R_IT(UART_HandleTypeDef *huart);//interrupt handler for rx
int uart_count_rx(UART_HandleTypeDef *huart);//return the no of byte in tx ring buffer
int uart_count_tx(UART_HandleTypeDef *huart);//return the no of byte in rx ring buffer
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
  MX_USART2_UART_Init();
  int status_input=0;
  char test_byte[2]="up";
  tx_char(&huart2,&test_byte[0]);
  tx_char(&huart2,&test_byte[1]);
  HAL_Delay(5000);
  int re_count=uart_count_rx(&huart2);
  if(re_count){
	  char rec_byte;
	  rx_char(&huart2,&rec_byte);
	  if(rec_byte=='u'&& uart_count_rx(&huart2))
	  {
		rx_char(&huart2,&rec_byte);
		if(rec_byte=='p')
		{
		  HAL_GPIO_WritePin(GPIOE,LD6_Pin,GPIO_PIN_SET);
		  status_input=1;
		}
		else
		{
		  HAL_GPIO_WritePin(GPIOE,LD6_Pin,GPIO_PIN_RESET);
		}
	  }
	  else
	  {
		  HAL_GPIO_WritePin(GPIOE,LD6_Pin,GPIO_PIN_RESET);
	  }
  }
  else
  {
	  HAL_GPIO_WritePin(GPIOE,LD6_Pin,GPIO_PIN_RESET);
  }
  /* USER CODE BEGIN 2 */
  uint32_t flashaddress = 0x8003000;


      //HAL_GPIO_WritePin(GPIOE,LD4_Pin,status_input);
      if (status_input==1)
      {
    	  //in bootloader mode
      }
      else
      {
    	  //switch to application mode
    	  if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
    	      {
    	        /* Jump to user application */
    		    uint32_t * jadd=(APPLICATION_ADDRESS + 4);
    	        JumpAddress = *jadd ;
    	        Jump_To_Application = (pFunction) JumpAddress;

    	        /* Initialize user application's Stack Pointer */
    	        __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

    	        /* Jump to application */
    	        Jump_To_Application();
    	      }
      }
      FLASH_EraseInitTypeDef erpage;
          erpage.TypeErase=FLASH_TYPEERASE_PAGES;
          erpage.NbPages=1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  int nu=uart_count_rx(&huart2);


	  	  	   while(nu)
	  	  	   {
	  	  		   rx_char(&huart2,&i_byte);
	  	  		   switch(state){
	  	  		   case 0:
	  	  			   	   state=(i_byte==255)?1:0;
	  	  			   	   break;
	  	  		   case 1:
	  	  			       state=(i_byte==255)?2:0;
	  	  			 	   break;
	  	  		   case 2:
	  	  			       state=(i_byte==255)?3:0;
	  	  			 	   break;
	  	  		   case 3:
	  	  			 	   state=(i_byte==255)?4:0;
	  	  			 	   address=0;
	  	  			 	   nu_bytes=0;
	  	  			 	   break;
	  	  		   case 4:

	  	  			      if(nu_bytes<4)
	  	  			      {
	  	  			    	address+=i_byte<<(nu_bytes*8);
	  	  			      }
	  	  			      else{
	  	  			    	buffer[nu_bytes-4]=i_byte;
	  	  			      }
	  	  			      nu_bytes++;
	  	  			      state=(nu_bytes==260)?0:4;
	  	  			      if (nu_bytes==260)
	  	  			    		   {


	  	  			    	         int n_page=2;
	  	  			    	         int flag_er=0;

	  	  			    	             HAL_FLASH_Unlock();
	  	  			    	             if(address%2048==0){
	  	  			    	             uint32_t page_error;
	  	  			    	             erpage.PageAddress=address;
	  	  			    	             HAL_FLASHEx_Erase(&erpage,&page_error);
	  	  			    	             HAL_Delay(10);
	  	  			    	             }
	  									 for(int k=0;k<64;k++)
	  									 {
	  										 uint32_t data=((buffer[k*4+3])<<24&0xff000000)+(buffer[k*4+2]<<16&0xff0000)+(buffer[k*4+1]<<8&0xff00)+buffer[k*4];
	  										 int status=HAL_ERROR;



	  											 status=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,(address+k*4),data);

	  											 if(status!=HAL_OK)
	  										     HAL_Delay(10);



	  	  			    	  	    }
	  							    HAL_FLASH_Lock();

	  	  			    	  	    if(!flag_er){
	  	  			    	  	  	char o[2]="ok";
	  	  			    	  	    tx_char(&huart2,&o[0]);
	  	  			    	  	    tx_char(&huart2,&o[1]);
	  	  			    	  	    }
	  	  			    	  	    else
	  	  			    	  	    {
	  	  			    	  	    char o[2]="fa";
	  	  			    	  	    tx_char(&huart2,&o[0]);
	  	  			    	  	    tx_char(&huart2,&o[1]);
	  	  			    	  	    }
	  	  			    	  	    nu_bytes=0;
	  	  			    		   }
	  	  			       break;
	  	  		   }
	  	  		   nu--;
	  	  	   }
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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
int uart_count_rx(UART_HandleTypeDef *huart)
{
	return (unsigned int)(buffer_size + huart->Ring_head_rx - huart->Ring_tail_rx) % buffer_size;
}
int uart_count_tx(UART_HandleTypeDef *huart)
{
	return (unsigned int)(buffer_size + huart->Ring_head_tx - huart->Ring_tail_tx) % buffer_size;
}
HAL_StatusTypeDef rx_char(UART_HandleTypeDef *huart,char *a)
{
	if( uart_count_rx(huart)>0)
		__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
	if (huart->Ring_head_rx == huart->Ring_tail_rx) {


			if(huart->gState == HAL_UART_STATE_BUSY_TX)
			      {
			        huart->gState = HAL_UART_STATE_BUSY_TX_RX;
			      }
			      else
			      {
			        /* Disable the UART Parity Error Interrupt */
			        __HAL_UART_ENABLE_IT(huart, UART_IT_PE);

			        /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
			        __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);

			        huart->gState = HAL_UART_STATE_BUSY_RX;
			      }
			return HAL_ERROR;
			} else {
			*a = huart->Ring_buffer_rx[huart->Ring_tail_rx];
			 huart->Ring_tail_rx= (uint8_t)(huart->Ring_tail_rx+ 1) % buffer_size;
			return HAL_OK;
}
}
HAL_StatusTypeDef tx_char(UART_HandleTypeDef *huart,char *a)
{
	  /* Process Locked */
	__HAL_LOCK(huart);
		int i = (huart->Ring_head_tx + 1) % buffer_size;
		 if(i == huart->Ring_tail_tx)
		 {
			  /* Process Unlocked */
			 __HAL_UNLOCK(huart);
			 return HAL_ERROR;
		 }
		 huart->Ring_buffer_tx[huart->Ring_head_tx]=*a;
		 huart->Ring_head_tx = i;
		 if(huart->gState == HAL_UART_STATE_BUSY_RX)
		     {
		       huart->gState = HAL_UART_STATE_BUSY_TX_RX;
		     }
		     else
		     {
		       huart->gState = HAL_UART_STATE_BUSY_TX;
		     }
		  /* Process Unlocked */
		 __HAL_UNLOCK(huart);
		 /* Enable the UART Transmit Data Register Empty Interrupt */
		 __HAL_UART_ENABLE_IT(huart, UART_IT_TXE);
		 if(huart->Instance==USART2)
		 {
			 uint32_t tmpreg                     = 0x00000000;
			 		 huart2.Init.Mode = UART_MODE_TX;
			 		 tmpreg = (uint32_t)huart->Init.WordLength | huart->Init.Parity | huart->Init.Mode | huart->Init.OverSampling ;
			 		 MODIFY_REG(huart->Instance->CR1, UART_CR1_FIELDS, tmpreg);
			 		// HAL_Delay(1);
		 }
	return HAL_OK;
}
HAL_StatusTypeDef UART_T_IT(UART_HandleTypeDef *huart)
{

  if ((huart->gState == HAL_UART_STATE_BUSY_TX) || (huart->gState == HAL_UART_STATE_BUSY_TX_RX))
  {

    if(huart->Ring_head_tx == huart->Ring_tail_tx)
    {
      /* Disable the UART Transmit Data Register Empty Interrupt */
      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

      /* Enable the UART Transmit Complete Interrupt */
      __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
      //HAL_UART_TxCpltCallback(&huart);

      return HAL_OK;
    }
    else
    {

        huart->Instance->TDR = (uint8_t)(huart->Ring_buffer_tx[huart->Ring_tail_tx ] & (uint8_t)0xFF);

        huart->Ring_tail_tx = (huart->Ring_tail_tx + 1) % buffer_size;


      return HAL_OK;
    }
  }
  else
  {
    return HAL_BUSY;
  }
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Ring_head_tx == huart->Ring_tail_tx)
	    {
	      /* Disable the UART Transmit Data Register Empty Interrupt */
	      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

	      /* Enable the UART Transmit Complete Interrupt */
	      __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
	      if(huart->Instance==USART2)
	            		 {
	            			 uint32_t tmpreg                     = 0x00000000;
	            			 		 huart2.Init.Mode = UART_MODE_RX;
	            			 		 tmpreg = (uint32_t)huart->Init.WordLength | huart->Init.Parity | huart->Init.Mode | huart->Init.OverSampling ;
	            			 		 MODIFY_REG(huart->Instance->CR1, UART_CR1_FIELDS, tmpreg);
	            			 		// HAL_Delay(1);
	            		 }
	    }
	else
		__HAL_UART_ENABLE_IT(huart, UART_IT_TXE);
}
HAL_StatusTypeDef UART_R_IT(UART_HandleTypeDef *huart)
{

	  uint16_t uhMask = huart->Mask;
	  //HAL_GPIO_TogglePin(GPIOC, LD4_Pin);
	  //if((huart->State == HAL_UART_STATE_BUSY_RX) || (huart->State == HAL_UART_STATE_BUSY_TX_RX))
	  //{
		  int i = (unsigned int)(huart->Ring_head_rx + 1) % buffer_size;


	      		if (i != huart->Ring_tail_rx)
	      		{
	      			huart->Ring_buffer_rx[huart->Ring_head_rx] = (uint8_t)(huart->Instance->RDR );
	      			huart->Ring_head_rx = i;
	      		}

	    if(huart->Ring_head_rx == huart->Ring_tail_rx)
	    {
	      __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);

	      /* Check if a transmit Process is ongoing or not */
	      if(huart->gState == HAL_UART_STATE_BUSY_TX_RX)
	      {
	        huart->gState = HAL_UART_STATE_BUSY_TX;
	      }
	      else
	      {
	        /* Disable the UART Parity Error Interrupt */
	        __HAL_UART_DISABLE_IT(huart, UART_IT_PE);

	        /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
	        __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

	        huart->gState = HAL_UART_STATE_READY;
	      }

	      //HAL_UART_RxCpltCallback(huart);

	      return HAL_OK;
	    }

	    return HAL_OK;
	  //}
	  //else
	  //{
	    //return HAL_BUSY;
	  //}
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
