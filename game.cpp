#include "stm32f10x.h"                  // Device header
#include "string.h"
#include "ssd1306.h"
static void datareceived();
uint16_t num,x,y;
uint16_t data[8];
uint8_t i;
int main(void)
{
	//gpio C clock and pin 13 output
	
  RCC->APB2ENR |=RCC_APB2ENR_IOPCEN;		//clock Port C
	RCC->APB2ENR |=RCC_APB2ENR_IOPAEN;		//clock Port A

	// declare as Output pin 13
	GPIOC->CRH |=GPIO_CRH_MODE13;
	GPIOC->CRH &=~GPIO_CRH_CNF13;
	
	// declare as Output port a pin 0-7
	GPIOA->CRL |=GPIO_CRL_MODE;
	GPIOA->CRL &=~GPIO_CRL_CNF;
	
	//enabling GPIO and USART1

	RCC->APB2ENR |=RCC_APB2ENR_USART1EN; //USART1 enable 1<<14
	
	//pin konfiguration for uart rx/tx
	//PA9 TX pin Alternative push pull 50MHz output  -> 1011
	
	GPIOA->CRH |=GPIO_CRH_MODE9;				//0x00000030 -> 0011
	GPIOA ->CRH |= GPIO_CRH_CNF9_1;			//0x00000080 -> 1000
	GPIOA->CRH &=~GPIO_CRH_CNF9_0;      //0x00000040 -> 0100
	
	//PA10 RX pin floating input  -> 0100 //default state of GPIO
	
	
	//badurate setting 9600 Kbps
	
	//     --------   BUSCLOCK/(16*DESIRE_USART_SPEED) -> 72MHZ/(16*9600) ->468.75
	
	//468->1D4   dec->hex
	//0.75 * 16 = 12-> C
	//468.75=0x1D4C
	
	USART1->BRR=0x1D4C; //9600 


	//enable RX enable TX UART enbale in CR1 register
	
	USART1->CR1 |=USART_CR1_RE; 				//RX enable, receive data enabled
	USART1 ->CR1 |=USART_CR1_TE;				//TX, enabled,  transmite data enabled
	USART1->CR1 |=USART_CR1_UE;					//USART enabled
	
	GPIOC->ODR=(1<<13);
	
	
	while(1)
	{
		datareceived();

  }
}
void datareceived(){
//check incoming data
		
		if(USART1->SR &	USART_SR_RXNE) 		//check if data is received
			{

				data[i] = USART1->DR;			//read it and store in data array
			if (data[0]==88)
			{				
				if (data[i]!=10)
				{
						if(i!=0)num =  (data[i] - '0')  + (num * 10);
						i++;
				}
				else
				{
					    	x=num;//
								i=0;
								num=0;
				}
			}
			else if(data[0]==89)
			{
			if (data[i]!=10)
				{
						if(i!=0)num =  (data[i] - '0')  + (num * 10);
						i++;
				}
				else
				{
					      y=num;
								i=0;
								num=0; 
			  }
				while(!(USART1->SR & USART_SR_TC)){;};	
	}
}
}






//num =  (data[i] - '0')  + (num * 10);
	void Kriz(int x, int y){
		int x1 = x/2;
		int y1 = y/3;
		//int x0 = x - 4;
		//int x1 = x + 4;
		//SSD1306_DrawLine(x0, y, x1, y, SSD1306_COLOR_WHITE);
		SSD1306_DrawPixel(x1,y1,SSD1306_COLOR_WHITE);
		SSD1306_UpdateScreen();
	}