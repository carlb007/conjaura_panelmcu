/*
 * spi.c
 *
 *  Created on: 2 Aug 2019
 *      Author: me
 */

#include "spi.h"

void InitSPI(){
	// http://libopencm3.org/docs/latest/stm32f4/html/group__spi__defines.html
	//CONFIGURE SPI PINS AS PER HAL LIBRARIES.
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_SPI2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = LED_RAM_CLK_TX_Pin|LED_RAM_DATA_TX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = PI_CLK_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
	HAL_GPIO_Init(PI_CLK_RX_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = PI_DATA_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_SPI2;
	HAL_GPIO_Init(PI_DATA_RX_GPIO_Port, &GPIO_InitStruct);

	SPI1Defaults();
	SPI2Defaults();
}

void SPI1Defaults(){
	//CONFIGURE SPI DEFAULTS FOR OUR NEEDS:
	SPI1->CR1 &= ~SPI_CR1_SPE;			//BIT 6 DISABLE SPI
	SPI1->CR1 = 0;						//CLEAR ALL BITS
	SPI1->CR2 = 0;						//CLEAR ALL BITS

	SPI1->CR1 |= SPI_CR1_MSTR;			//BIT 2. MASTER MODE ACTIVE
	SPI1->CR1 |= SPI_LSBFIRST;			//BIT 7. LSB FIRST DURING LED DATA.
	SPI1->CR1 |= SPI_CR1_SSM;			//BIT 9. DISABLE SLAVE MANAGEMENT.
	SPI1->CR2 |= SPI_CR2_TXDMAEN;		//BIT 1 TO 1. ENABLE DMA TX (BIT 0 ENABLES RX DMA)
	SPI1->CR2 |= SPI_CR2_SSOE;			//BIT 2. ENABLE SS
	SPI1->CR2 |= SIZE_8BIT;				//BIT 8,9,10,11. SET TO 0111 FOR 8 BIT.
	SPI1->CR2 |= SPI_CR2_FRXTH;			//BIT 12. RXNE EVENT IF FIFO >=8BIT.

	SPI1->CR1 |= SPI_CR1_SPE;			//SET 6th BIT TO 1 TO ENABLE SPI
}

void SPI2Defaults(){
	//CONFIGURE SPI DEFAULTS FOR OUR NEEDS:
	SPI2->CR1 &= ~SPI_CR1_SPE;			//BIT 6 DISABLE SPI
	SPI2->CR1 = 0;						//CLEAR ALL BITS
	SPI2->CR2 = 0;						//CLEAR ALL BITS

	SPI2->CR1 |= SPI_CR1_CPOL;			//BIT 1. CPOL = HIGH
	SPI2->CR1 |= SPI_CR1_SSM;			//BIT 9. DISABLE SLAVE MANAGEMENT.
	SPI2->CR1 |= SPI_CR1_RXONLY;		//BIT 10. RX ONLY TURNED ON.
	SPI2->CR2 |= SPI_CR2_RXDMAEN;		//BIT 0. SET TO 1 ACTIVATED RX DMA
	SPI2->CR2 |= SIZE_8BIT;				//BIT 8,9,10,11. SET TO 0111 FOR 8 BIT.
	SPI2->CR2 |= SPI_CR2_FRXTH;			//BIT 12. RXNE EVENT IF FIFO >=8BIT.

	SPI2->CR1 |= SPI_CR1_SPE;			//SET 6th BIT TO 1 TO ENABLE SPI
}

void ConfigLEDDataSPI(){
	//LED DATA NEEDS A FASTER BUS, LSB AND CPOL OF LOW.
	SPI1->CR1 &= ~SPI_CR1_SPE;			//BIT 6 DISABLE SPI
	SPI1->CR1 &= ~SPI_CR1_CPOL;			//BIT 1. CPOL = LOW
	SPI1->CR1 &= ~SPI_SPEED_CLK2;		//BIT 3,4,5. CLK/2
	SPI1->CR1 |= SPI_LSBFIRST;			//BIT 7. LSB FIRST DURING LED DATA.
	SPI1->CR1 |= SPI_CR1_SPE;			//SET 6th BIT TO 1 TO ENABLE SPI
}

void ConfigReturnDataSPI(){
	//RETURN DATA NEEDS A SLOWER BUS, MSB AND CPOL OF HIGH.
	SPI1->CR1 &= ~SPI_CR1_SPE;			//BIT 6 DISABLE SPI
	SPI1->CR1 |= SPI_CR1_CPOL;			//BIT 1. CPOL = HIGH
	SPI1->CR1 &= ~SPI_SPEED_CLK2;		//CLEAR CLOCK SPEED (PUSH INTO FCLK/2)
	SPI1->CR1 |= SPI_SPEED_CLK8;		//BIT 3,4,5. CLK/8 010
	SPI1->CR1 &= ~SPI_LSBFIRST;			//BIT 7. MSB FIRST DURING RETURN DATA.
	SPI1->CR1 |= SPI_CR1_SPE;			//SET 6th BIT TO 1 TO ENABLE SPI
}


/*
 * FOR REFERENCE:
 * //CONFIGURE SPI DEFAULTS FOR OUR NEEDS:
	/SPI1->CR1 &= ~SPI_CR1_SPE;			//BIT 6 DISABLE SPI
	//SPI1->CR1 = 0;						//CLEAR ALL BITS
	//SPI1->CR2 = 0;						//CLEAR ALL BITS

	//SPI1->CR1 &= ~SPI_CR1_CPHA;		//BIT 0. EDGE RISING
	//SPI1->CR1 &= ~SPI_CR1_CPOL;		//BIT 1. CPOL = LOW
	//SPI1->CR1 |= SPI_CR1_MSTR;			//BIT 2. MASTER MODE ACTIVE
	//SPI1->CR1 &= ~SPI_SPEED;			//BIT 3,4,5. CLK/2

	//SPI1->CR1 |= SPI_LSBFIRST;			//BIT 7. LSB FIRST DURING LED DATA.
	//SPI1->CR1 &= ~SPI_CR1_SSI;		//BIT 8. INTERNAL SLAVE SELECT.
	//SPI1->CR1 |= SPI_CR1_SSM;			//BIT 9. DISABLE SLAVE MANAGEMENT.
	//SPI1->CR1 &= ~SPI_CR1_RXONLY;		//BIT 10. RX ONLY TURNED OFF.
	//SPI1->CR1 &= ~(1<<10);			//BIT 11. CRC LEN
	//SPI1->CR1 &= ~SPI_CR1_CRCNEXT;	//BIT 12. SET 0 TO TRANSMIT FROM BUFFER NOT CRC REGISTER.
	//SPI1->CR1 &= ~SPI_CR1_CRCEN;		//BIT 13. CRC DISABLED.
	//SPI1->CR1 &= ~SPI_CR1_BIDIOE;		//BIT 14. OUTPUT ENABLED
	//SPI1->CR1 &= ~SPI_CR1_BIDIMODE;	//BIT 15. BIDI MODE TURNED ON.

	//SPI1->CR2 &= ~SPI_CR2_RXDMAEN;	//BIT 0. DEACTIVATED RX DMA
	//SPI1->CR2 |= SPI_CR2_TXDMAEN;		//BIT 1 TO 1. ENABLE DMA TX (BIT 0 ENABLES RX DMA)
	//SPI1->CR2 |= SPI_CR2_SSOE;			//BIT 2. ENABLE SS
	//SPI1->CR2 &= ~SPI_CR2_NSSP;		//BIT 3. DISABLE NSSP (NO PULSE GENERATED)
	//SPI1->CR2 &= ~SPI_CR2_FRF;		//BIT 4. DISABLE TI MODE
	//SPI1->CR2 &= ~SPI_CR2_ERRIE;		//BIT 5. DISABLE ERROR INTERUPT
	//SPI1->CR2 &= ~SPI_CR2_RXNEIE;		//BIT 6. DISABLE RX BUFFER NOT EMPTY INTERUPT
	//SPI1->CR2 |= SPI_CR2_TXEIE;		//BIT 7. DISABLE TX BUFFER EMPTY INTERUPT
	//SPI1->CR2 |= SIZE_8BIT;				//BIT 8,9,10,11. SET TO 0111 FOR 8 BIT.
	//SPI1->CR2 |= SPI_CR2_FRXTH;			//BIT 12. RXNE EVENT IF FIFO >=8BIT.
	//SPI1->CR2 &= ~(1<<13);			//BIT 13. LDMA_RX SET TO EVEN.
	//SPI1->CR2 &= ~(1<<14);			//BIT 14. LDMA_TX SET TO EVEN.

	//SPI1->CR1 |= SPI_CR1_SPE;			//SET 6th BIT TO 1 TO ENABLE SPI
 *
 */
