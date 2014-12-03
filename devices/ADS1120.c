#include "ADS1120.h"

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"
#include <misc.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "watchdog.h"


extern struct task_watch task_watches[];

static void ADS1120_task(void *pvParameters);//

static void	SPI2_config(void);


xSemaphoreHandle xSPI_Buf_Mutex;


uint8_t ADS1120_init(void)//
{
	SPI2_config();
	xTaskCreate(ADS1120_task,(signed char*)"ADS1120_TASK",128,NULL, tskIDLE_PRIORITY + 1, NULL);
	return 0;
}


void	SPI2_config(void)//
{
		RCC_AHB1PeriphClockCmd(SPI2_GPIO_BUS, ENABLE);
	    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	    GPIO_InitTypeDef GPIO_InitStructure;
	    SPI_InitTypeDef SPI_InitStructure;

	    /* Configure SPI1 pins: SCK, MISO and MOSI -------------------------------*/
	    GPIO_InitStructure.GPIO_Pin   = SPI2_SCK|SPI2_MOSI;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	    GPIO_Init(SPI2_GPIO, &GPIO_InitStructure);

	    GPIO_InitStructure.GPIO_Pin   = SPI2_MISO;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	    GPIO_Init(SPI2_GPIO, &GPIO_InitStructure);


	    GPIO_InitStructure.GPIO_Pin   = SPI2_CS1;
	    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(SPI2_GPIO_CS, &GPIO_InitStructure);

		GPIO_PinAFConfig(SPI2_GPIO, SPI2_SCK_PinSource, GPIO_AF_SPI2);
		GPIO_PinAFConfig(SPI2_GPIO, SPI2_MOSI_PinSource, GPIO_AF_SPI2);
		GPIO_PinAFConfig(SPI2_GPIO, SPI2_MISO_PinSource, GPIO_AF_SPI2);

	    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	    //SPI_InitStructure.SPI_CRCPolynomial = 7;
	    SPI_Init(SPI2, &SPI_InitStructure);

	    /* Enable SPI2 */
	    SPI_CalculateCRC(SPI2, DISABLE);
	    SPI_Cmd(SPI2, ENABLE);

	    SPI2_GPIO_CS->BSRRL|=SPI2_CS1;// pin up SPI2_CS1
}



uint8_t SPI2_send (uint8_t data)
{
  while (!(SPI2->SR & SPI_SR_TXE));      //���������, ��� ���������� �������� ���������
  SPI2->DR = data;                       //��������� ������ ��� ��������
  while (!(SPI2->SR & SPI_SR_RXNE));     //���� ��������� ������
  return (SPI2->DR);		         //������ �������� ������
}

uint8_t SPI2_read (void)
{
  return SPI2_send(0xff);		  //������ �������� ������
}

uint32_t ADC_result_temp;
int32_t ADC_result;
uint8_t adc_reg;

enum
{
	ADS_REG_0=0x0,
	ADS_REG_1,
	ADS_REG_2,
	ADS_REG_3,
};
//----------------------------------------------------------------
static void ADS1120_task(void *pvParameters)//
{
	SPI2_GPIO_CS->BSRRH|=SPI2_CS1;// pin down SPI1_CS1
	SPI2_send (ADS_RESET);
	vTaskDelay(10);
	SPI2_send (ADS_WREG|(ADS_REG_0<<2)|(0x0));//1 reg 0x0
	SPI2_send (ADC_REG_CONFIG_00);
	vTaskDelay(10);

	SPI2_send (ADS_WREG|(ADS_REG_2<<2)|(0x0));//1 reg 0x2
	SPI2_send (ADC_REG_CONFIG_02);
	vTaskDelay(10);

	SPI2_send (ADS_WREG|(ADS_REG_3<<2)|(0x0));//1 reg 0x3
	SPI2_send (ADC_REG_CONFIG_03);
	vTaskDelay(10);

//	spi_send (ADS_RREG|(ADS_REG_0<<2)|(0x0));//1 reg 0x0
//	adc_reg=spi_read ();
//	vTaskDelay(10);
//	spi_send (ADS_RREG|(ADS_REG_3<<2)|(0x0));//1 reg 0x3
//	adc_reg=spi_read ();
//	vTaskDelay(10);

	while(1)
	{
		SPI2_send (ADS_START);
		while(GPIO_ReadInputDataBit(SPI2_GPIO, SPI2_MISO)==Bit_SET);//wait
		//spi_send (ADS_RDATA);
		ADC_result_temp=SPI2_read ();
		ADC_result_temp=ADC_result_temp<<8;
		ADC_result_temp|=SPI2_read ();
		ADC_result_temp=ADC_result_temp<<8;
		ADC_result_temp|=SPI2_read ();



		if(ADC_result_temp>0x7FFFFF)
		{
			ADC_result=-(0xFFFFFF-(int32_t)ADC_result_temp);
		}
		else
		{
			ADC_result=(int32_t)ADC_result_temp;
		}

		vTaskDelay(100);
	}
}

