/*
 * PC_DATA.c
 *
 *  Created on: Mar 23, 2017
 *      Author: Thomas
 */



#include "main.h"

uint16_t dataWS[NB_LED] = {0};
uint8_t actualFrame = 0;


/*
 * ===========================================================
 * Initializes Pins for PWM
 * ===========================================================
 */
void DATA_InitGPIOs(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	// Configure PB9 as Alternate Function
	/*
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 						//

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_1); 	// Configure AF mux to connect TIM3 to PB1 (Channel 4)
	*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*
 * ============================================================
 * Configure PWM
 * TIM3
 * ============================================================
 */
void DATA_Conf(void)
{
	//NVIC_InitTypeDef NVIC_InitStructure = {0};


	DATA_InitGPIOs();


	////////////////////////////
/*
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);

	TIM_FUNC->CR1 &= ~TIM_CR1_CKD; 			// Clock division 1
	TIM_FUNC->CR1 |= TIM_CR1_ARPE; 			// Auto-Reload Register is Buffered
	TIM_FUNC->CR1 &= ~TIM_CR1_CMS; 			// Edge Aligned Mode (Counter UP)
	TIM_FUNC->CR1 &= ~TIM_CR1_DIR; 			// Counter UP
	TIM_FUNC->CR1 |= TIM_CR1_OPM; 			// One Pulse Mode : CEN bit reset at the next update event
	TIM_FUNC->CR1 |= TIM_CR1_URS; 			// Only counter Overflow generates interrupt request
	TIM_FUNC->CR1 &= ~TIM_CR1_UDIS; 		// Enable Update Events

	TIM_FUNC->EGR |= TIM_EGR_UG; 			// Reset the counter after Update Event

	TIM_FUNC->DIER |= TIM_DIER_UIE; 		// Update Interrupt Enable (Test UIF in SR register)

	TIM_FUNC->PSC = 4799; 					// Prescaler : 10kHz
	TIM_FUNC->ARR = 99; 					// Period = 99 for 10ms

	NVIC_InitStructure.NVIC_IRQChannel = TIM17_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
*/
	//TIM_FUNC->CR1 |= TIM_CR1_CEN; 							// Timer Enable for Duration
}

/*
 * ===================================================================
 * Adjust the length of pulse for RED
 * ===================================================================
 */
void DATA_RegulatorR(uint16_t phase)
{
	if (phase < 32)
		RValue = phase;
}

/*
 * ===================================================================
 * Adjust the length of pulse for GREEN
 * ===================================================================
 */
void DATA_RegulatorG(uint16_t phase)
{
	if (phase < 32)
		GValue = phase;
}

/*
 * ===================================================================
 * Adjust the length of pulse for BLUE
 * ===================================================================
 */
void DATA_RegulatorB(uint16_t phase)
{
	if (phase < 32)
		BValue = phase;
}

/*
 * ===================================================================
 * Set the color and mode
 * Mode is 0 to 6
 * - 0: continuous
 * - 1: auto 		- 2: flash
 * - 3: Jump3 		- 4: Fade3
 * - 5: Jump7 		- 6: Fade7
 *
 * Speed is 1 to 10 for non-continuous Modes
 * ===================================================================
 */
void DATA_SetColor(uint8_t percentR, uint8_t percentG, uint8_t percentB)
{
}

void DATA_PlayFrame(void)
{
	uint8_t ledCount;
	int8_t bitCount;

	for (ledCount = 0 ; ledCount < NB_LED ; ledCount++)
	{
		// GREEN High bit first

		for (bitCount = 0 ; bitCount < 3 ; bitCount++)
		{
			GPIOB->BSRR = 0x2;
			asm ("nop");
			asm ("nop");
			asm ("nop");
			asm ("nop");
			GPIOB->BSRR = 0x20000;
			Delay20(2);
		}
		for (bitCount = 14 ; bitCount >= 10 ; bitCount--)
		{
			if (((dataWS[ledCount] >> bitCount) & (uint16_t)0b1) == 0)
			{
				GPIOB->BSRR = 0x2;
				asm ("nop");
				asm ("nop");
				asm ("nop");
				asm ("nop");
				GPIOB->BSRR = 0x20000;
				Delay20(2);
			}
			else
			{
				GPIOB->BSRR = 0x2;
				Delay20(3);
				GPIOB->BSRR = 0x20000;
			}
		}
		// RED High bit first

		for (bitCount = 0 ; bitCount < 3 ; bitCount++)
		{
			GPIOB->BSRR = 0x2;
			Delay20(1);
			GPIOB->BSRR = 0x20000;
			Delay20(2);
		}
		for (bitCount = 9 ; bitCount >= 5 ; bitCount--)
		{
			if (((dataWS[ledCount] >> bitCount) & 0b1) == 0)
			{
				GPIOB->BSRR = 0x2;
				Delay20(1);
				GPIOB->BSRR = 0x20000;
				Delay20(2);
			}
			else
			{
				GPIOB->BSRR = 0x2;
				Delay20(2);
				GPIOB->BSRR = 0x20000;
				asm ("nop");
				asm ("nop");
			}
		}
		// BLUE High bit first

		for (bitCount = 0 ; bitCount < 3 ; bitCount++)
		{
			GPIOB->BSRR = 0x2;
			Delay20(1);
			GPIOB->BSRR = 0x20000;
			Delay20(2);
		}
		for (bitCount = 4 ; bitCount >= 0 ; bitCount--)
		{
			if (((dataWS[ledCount] >> bitCount) & 0b1) == 0)
			{
				GPIOB->BSRR = 0x2;
				Delay20(1);
				GPIOB->BSRR = 0x20000;
				Delay20(2);
			}
			else
			{
				GPIOB->BSRR = 0x2;
				Delay20(2);
				GPIOB->BSRR = 0x20000;
				asm ("nop");
				asm ("nop");
			}
		}
	}
}

void DATA_SetNewFrame(uint8_t actFrame)
{
	uint8_t nbLed;
	for (nbLed = 0 ; nbLed < NB_LED ; nbLed++)
	{
		DATA_SetLEDColor(nbLed, 0, 0, 0);
	}
	DATA_SetLEDColor(actFrame, RValue, GValue, BValue);
	DATA_SetLEDColor(actFrame - 1, RValue, GValue, BValue / 2);
	DATA_SetLEDColor(actFrame + 1, RValue, GValue, BValue / 2);
	DATA_SetLEDColor(actFrame - 2, RValue, GValue / 2, 0);
	DATA_SetLEDColor(actFrame + 2, RValue, GValue / 2, 0);
	DATA_SetLEDColor(actFrame - 3, RValue / 2, 0, 0);
	DATA_SetLEDColor(actFrame + 3, RValue / 2, 0, 0);

	DATA_SetLEDColor(NB_LED - actFrame, RValue, GValue, BValue);
	DATA_SetLEDColor(NB_LED - actFrame - 1, RValue, GValue, BValue / 2);
	DATA_SetLEDColor(NB_LED - actFrame + 1, RValue, GValue, BValue / 2);
	DATA_SetLEDColor(NB_LED - actFrame - 2, RValue, GValue / 2, 0);
	DATA_SetLEDColor(NB_LED - actFrame + 2, RValue, GValue / 2, 0);
	DATA_SetLEDColor(NB_LED - actFrame - 3, RValue, 0, 0);
	DATA_SetLEDColor(NB_LED - actFrame + 3, RValue, 0, 0);
}

void DATA_SetLEDColor(int16_t IdLED, uint8_t pcR, uint8_t pcG, uint8_t pcB)
{
	if ((IdLED >= (2 * NB_LED)) || (IdLED < (-1 * NB_LED)))
		IdLED = 0;
	if (pcR > 31)
		pcR = 31;
	if (pcG > 31)
		pcG = 31;
	if (pcB > 31)
		pcB = 31;

	if (IdLED >= NB_LED)
		dataWS[IdLED - NB_LED] = (pcG << 10) + (pcR << 5) + pcB;
	else if (IdLED < 0)
		dataWS[IdLED + NB_LED] = (pcG << 10) + (pcR << 5) + pcB;
	else
		dataWS[IdLED] = (pcG << 10) + (pcR << 5) + pcB;
}


