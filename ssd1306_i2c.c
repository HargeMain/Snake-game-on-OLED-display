/**
 * original author:  Tilen Majerle<tilen@majerle.eu>
 * modification for STM32f10x: Alexander Lutsai<s.lyra@ya.ru>
 * modification line 19 to line 33: GrunF
 */
#include "ssd1306_i2c.h"

/* Private variables */
static uint32_t ssd1306_I2C_Timeout;

/* Private defines */
#define I2C_TRANSMITTER_MODE   0
#define I2C_RECEIVER_MODE      1
#define I2C_ACK_ENABLE         1
#define I2C_ACK_DISABLE        0

void ssd1306_I2C_Init()
	{
	//i2c init I2C1 STANDARAD 100kHz speed
	RCC->APB2ENR |=RCC_APB2ENR_IOPBEN;
	RCC->APB1ENR |=RCC_APB1ENR_I2C1EN;
	RCC->APB2ENR |=RCC_APB2ENR_AFIOEN;
	//set pins PB6 and PB7 as open-drain Alternative Function
	GPIOB->CRL |=GPIO_CRL_MODE6; //11-11
	GPIOB->CRL  |=GPIO_CRL_CNF6; //11
	GPIOB->CRL |=GPIO_CRL_MODE7; //11-11
	GPIOB->CRL  |=GPIO_CRL_CNF7; //11
	I2C1->CR2 |= (36 & 0x3F); //  14 FREQ value in CR2 reg masked all except first 6 bits 0x3F ->11 11 11 
	I2C1->CCR |= (180 & 0xFFF);// 70 CCR value in CCR reg masked all except first 12 bits 0xFFF ->1111 1111 1111 
	
		 I2C1->TRISE = (37 & 0x3F);  //15fr0m datasheet TRISE = (FREQvalue/1000ns)+1;
		 I2C1->CR1|= I2C_CR1_PE; // enable peripherial
		 I2C1->CR1 |= I2C_CR1_ACK;  // ACK enabled -----> disabled by PN=0
	

}





void ssd1306_I2C_WriteMulti(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
	uint8_t i;
	ssd1306_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
	ssd1306_I2C_WriteData(I2Cx, reg);
	for (i = 0; i < count; i++) {
		ssd1306_I2C_WriteData(I2Cx, data[i]);
	}
	ssd1306_I2C_Stop(I2Cx);
}




/* Private functions */
int16_t ssd1306_I2C_Start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction, uint8_t ack) {
	/* Generate I2C start pulse */
	I2Cx->CR1 |= I2C_CR1_START;
	
	/* Wait till I2C is busy */
	ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
	while (!(I2Cx->SR1 & I2C_SR1_SB)) {
		if (--ssd1306_I2C_Timeout == 0x00) {
			return 1;
		}
	}

	/* Enable ack if we select it */
	if (ack) {
		I2Cx->CR1 |= I2C_CR1_ACK;
	}

	/* Send write/read bit */
	if (direction == I2C_TRANSMITTER_MODE) {
		/* Send address with zero last bit */
		I2Cx->DR = address & ~I2C_OAR1_ADD0;
		
		/* Wait till finished */
		ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
		while (!(I2Cx->SR1 & I2C_SR1_ADDR)) {
			if (--ssd1306_I2C_Timeout == 0x00) {
				return 1;
			}
		}
	}
	if (direction == I2C_RECEIVER_MODE) {
		/* Send address with 1 last bit */
		I2Cx->DR = address | I2C_OAR1_ADD0;
		
		/* Wait till finished */
		ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
	//	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {   /////////////////////pazi ovo
			if (--ssd1306_I2C_Timeout == 0x00) {
				return 1;
			}
		//}
	}
	
	/* Read status register to clear ADDR flag */
	I2Cx->SR2;
	
	/* Return 0, everything ok */
	return 0;
}

void ssd1306_I2C_WriteData(I2C_TypeDef* I2Cx, uint8_t data) {
	/* Wait till I2C is not busy anymore */
	ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
	while (!(I2Cx->SR1 & I2C_SR1_TXE) && ssd1306_I2C_Timeout) {
		ssd1306_I2C_Timeout--;
	}

	/* Send I2C data */
	I2Cx->DR = data;
}

void ssd1306_I2C_Write(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t data) {
	ssd1306_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
	ssd1306_I2C_WriteData(I2Cx, reg);
	ssd1306_I2C_WriteData(I2Cx, data);
	ssd1306_I2C_Stop(I2Cx);
}


uint8_t ssd1306_I2C_Stop(I2C_TypeDef* I2Cx) {
	/* Wait till transmitter not empty */
	ssd1306_I2C_Timeout = ssd1306_I2C_TIMEOUT;
	while (((!(I2Cx->SR1 & I2C_SR1_TXE)) || (!(I2Cx->SR1 & I2C_SR1_BTF)))) {
		if (--ssd1306_I2C_Timeout == 0x00) {
			return 1;
		}
	}
	
	/* Generate stop */
	I2Cx->CR1 |= I2C_CR1_STOP;
	
	/* Return 0, everything ok */
	return 0;
}

uint8_t ssd1306_I2C_IsDeviceConnected(I2C_TypeDef* I2Cx, uint8_t address) {
	uint8_t connected = 0;
	/* Try to start, function will return 0 in case device will send ACK */
	if (!ssd1306_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_ENABLE)) {
		connected = 1;
	}

	/* STOP I2C */
	ssd1306_I2C_Stop(I2Cx);

	/* Return status */
	return connected;
}
