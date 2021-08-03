/**
*****************************************************************************
**
**  File        : main.c - PC
**
**  Abstract    : main function.
**
**  Functions   : main
**
**  Environment : Atollic TrueSTUDIO(R)
**
*****************************************************************************
*/

/* Includes */
#include "main.h"



/*
 *======================================================
 * Introduce 'Time' µsec Delay
 *======================================================
 */
void Delay(uint32_t Time)
{
	uint32_t compt = 0;
	while (compt < Time)
	{
		asm ("nop");															asm ("nop");
		asm ("nop");asm ("nop");									asm ("nop");asm ("nop");
		asm ("nop");asm ("nop");asm ("nop");			asm ("nop");asm ("nop");asm ("nop");
		asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
		asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
		asm ("nop");asm ("nop");asm ("nop");			asm ("nop");asm ("nop");asm ("nop");
		asm ("nop");asm ("nop");									asm ("nop");asm ("nop");
		asm ("nop");															asm ("nop");

		compt += 1;
	}
}

/*
 *======================================================
 * Introduce 'Time' µsec Delay
 *======================================================
 */
void Delay20(uint32_t Time)
{
	uint32_t compt = 0;
	while (compt < Time)
	{
		asm ("nop");															asm ("nop");

		compt += 1;
	}
}

/*
 * ================
 * 60 + 17 * 20
 * ================
 */
inline void Wait400ns()
{
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
}

/*
 * ================
 * 60 + 37 * 20
 * ================
 */
inline void Wait800ns()
{
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");
}

/*
 * ================
 * 200 + 12 * 20
 * ================
 */
inline void Wait450ns()
{
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
}

/*
 * ================
 * 200 + 32 * 20
 * ================
 */
inline void Wait850ns()
{
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");asm ("nop");
	asm ("nop");asm ("nop");
}

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
	uint32_t _ii, _jj, _kk;
	//uint16_t nMoy = 0;
	//uint32_t tempSampValue = 0;


	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); 		// PA5 (ADC) - PA15 (USART1) - PA7 (PWM)
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); 		// PB0 (PWM) - PB1 (PWM)
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);					//

	//SysTick_Config(480);  // 0.01 ms = 10us if clock frequency 48 MHz

	//SystemCoreClockUpdate();

	//RC_Conf();
	//IR_IC_ReadSignal(); //<========== SELECT CORRECT TIMER
	//ADC_Conf();
	DATA_Conf();

	//////////////GGGGGRRRRRBBBBB
	dataWS[0] = 0b000000000000000;
	dataWS[1] = 0b000000000000000;
	dataWS[2] = 0b111111111100000;
	dataWS[3] = 0b000000000000000;
	dataWS[4] = 0b000001111100000;
	dataWS[5] = 0b000000000000000;
	dataWS[6] = 0b000000000011111;

	while (1)
	{
		for (_kk = 0; _kk < 10; _kk++)
		{
			for (_jj = 0; _jj < 75; _jj++)
			{
				for (_ii = 0; _ii < 24; _ii++)
				{
					GPIOB->BSRR = 0x2;
					Wait800ns();
					GPIOB->BSRR = 0x20000;
					Wait450ns();
				}
			}
			Delay(80);
		}
		for (_kk = 0; _kk < 10; _kk++)
		{
			for (_jj = 0; _jj < 75; _jj++)
			{
				for (_ii = 0; _ii < 24; _ii++)
				{
					GPIOB->BSRR = 0x2;
					Wait400ns();
					GPIOB->BSRR = 0x20000;
					Wait850ns();
				}
			}
			Delay(80);
		}

		/*TIM_FUNC->SR &= ~TIM_SR_UIF;
		TIM_FUNC->CR1 |= TIM_CR1_CEN; 							// Timer Enable for Duration (10ms)*/

		/*
		ii++;
		DATA_PlayFrame();
		Delay(3000);
		actualFrame += 1;
		if (actualFrame >= NB_LED)
			actualFrame = 0;
		DATA_SetNewFrame(actualFrame);

		if ((ii % 20000) == 0)
		{
			IR_IC_Decode();
			ADC1->CR |= ADC_CR_ADSTART;
		}
		if (ii == 700000)
		{
			if (isID == 0)
			{
				DATA_SetColor(0, 0, 0); ////////////////////////////////
				RC_InitID();
				selectedID = FLASH_Read(ADD_ID);
				isID = 1;
			}

			ii = 0;
		}

		if ((ADC1->ISR & ADC_ISR_EOC) != 0)
		{
			tempSampValue += (ADC1->DR & 0x00000FFF);
			nMoy += 1;
			if (nMoy == 30)
			{
				sampValue = tempSampValue / nMoy;
				if ((isID == 0) && (sampValue < SAMP_12V))
				{
					DATA_SetColor(100, 0, 0);
				}
				nMoy = 0;
				tempSampValue = 0;
			}
		}
		*/

		//if (messAvailable == 1)
		//	RC_Decode();
		//while (((TIM_FUNC->SR) & TIM_SR_UIF) != TIM_SR_UIF); // Wait for 10ms if necessary (100 FPS).
	}
	return 0;
}


