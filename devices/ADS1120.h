#ifndef ADS1120_H
#define ADS1120_H

#include "stm32f4xx.h"


#define SPI2_GPIO_BUS 	    		RCC_AHB1Periph_GPIOB
#define SPI2_CS_GPIO_BUS 			RCC_AHB1Periph_GPIOB
#define SPI2_GPIO					GPIOB
#define SPI2_GPIO_CS				GPIOB

#define SPI2_SCK					GPIO_Pin_13
#define SPI2_MISO					GPIO_Pin_14
#define SPI2_MOSI					GPIO_Pin_15

#define SPI2_SCK_PinSource			GPIO_PinSource13
#define SPI2_MISO_PinSource			GPIO_PinSource14
#define SPI2_MOSI_PinSource			GPIO_PinSource15

#define SPI2_CS1					GPIO_Pin_12


#define ADS_RESET		0x06
#define ADS_START		0x08
#define ADS_POWERDOWN	0x02
#define ADS_RDATA		0x10
#define ADS_RREG		0x20
#define ADS_WREG		0x40

//reg config
#define ADC_REG_CONFIG_00	0x06//0x36//0xE1/*0xD1*///0x81
#define ADC_REG_CONFIG_02	0x66//0xA6//0x21//1000uA current source//0x20
#define ADC_REG_CONFIG_03	0x22//0x62//0xC2//idac1 ->refn0//0x02

uint8_t ADS1120_init(void);

#endif
