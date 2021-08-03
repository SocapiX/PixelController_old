/*
 * PC_ADC.c
 *
 *  Created on: Mar 23, 2017
 *      Author: Thomas
 */


#include "main.h"



#define SAMPLE_TIME ADC_SampleTime_28_5Cycles 	// Resistance < 25k

/*
 * Conversion time = 7.5 + 12.5 Clock Cycles = 278ns | 1 conversions -> 0,278 + 0.0416탎
 * Conversion time = 4.5 + 12.5 Clock Cycles = 236ns | 1 conversions -> 0,236 + 0.0416탎
 * Conversion time = 2.5 + 12.5 Clock Cycles = 208ns | 1 conversions -> 0,208 + 0.0416탎
 * Conversion time = 1.5 + 12.5 Clock Cycles = 194ns | 1 conversions -> 0,194 + 0.0416탎 - 1.302 with DMA interrupt
 * Conversion time = 181.5 + 12.5 Clock Cycles = 194ns | 1 conversion -> 2,694 + 0.0416탎 - 3.815 with DMA interrupt
 * (With 64MHz)
 */

uint16_t sampValue = 2500;


/*
 * ===========================================================================
 * Set up GPIO Pins for ADC
 * PA5 as Analog Input for ADC Conversion
 * ===========================================================================
 */
void ADC_InitGPIOs(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 						//
}


/*
 * ======================================================
 * Initializes ADC and selects parameters
 * Analog Pins : PA5 *******************************
 * ======================================================
 */
void ADC_Conf(void)
{
	ADC_InitTypeDef ADC_InitStructure = {0};

	if ((ADC1->CR & ADC_CR_ADEN) != 0)
		ADC1->CR &= (uint32_t) (~ADC_CR_ADEN);

	////////// Clock Selection (HSI 14MHz) ///////////

	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->CR2 |= RCC_CR2_HSI14ON;
	while ((RCC->CR2 & RCC_CR2_HSI14RDY) == 0);

	ADC1->CFGR2 = (uint32_t)ADC_ClockMode_AsynClk;

	ADC_InitGPIOs();

	////////// ADCs Initialization... //////////

	ADC_StructInit(&ADC_InitStructure);
	Delay(30);
	ADC1->CR |= ADC_CR_ADCAL;
	while ((ADC1->CR & ADC_CR_ADCAL) != 0);

	////////// ADC1 Parameters... //////////

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;								// In ADC_DR Register
	ADC_Init(ADC1, &ADC_InitStructure);


	//ADC1->IER |= 0x00000004;						// EOCIE set : enable interrupt at end of conversion (data in ADC_DR)
	//ADC1->IER |= 0x00000008;						// EOSIE : do not test EOS with EOC at the same time (flags interfere)
	ADC1->CFGR1 |= ADC_CFGR1_AUTDLY;				// Auto-delay On : prevent Overrun or errors
	ADC1->CFGR1 |= ADC_CFGR1_DISCEN; 				// Enable DISCEN bit : Discontinuous mode
	ADC1->CFGR1 &= ~(ADC_CFGR1_CONT);				// CONT = 0 : Single mode active : Conversion stops after 1 sequence


	ADC_ConfigChannels(); 							// Configure Channel order in ADC_SQRx, and SampleTime in SMPR


	ADC1->CR |= ADC_CR_ADEN; 						// Enables ADC1

	/* wait for ADRDY */
	while ((ADC1->ISR & ADC_ISR_ADRDY) == 0);

	//ADC START
	//ADC1->CR |= ADC_CR_ADSTART;
	//ADC->DR // Register to read
}


/*
 * ============================================================
 * Configures Channels for ADC1
 * 1 Channel for each ADC
 * ============================================================
 */
void ADC_ConfigChannels(void)
{
// Channel for ADC1
	ADC_ChannelConfig(ADC1, ADC_Channel_1, SAMPLE_TIME); 		// PA1
}

