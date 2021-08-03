/*
 * PC_RC.c
 *
 *  Created on: Mar 23, 2017
 *      Author: Thomas
 */


#define TIM_IN_CAPTURE 		TIM3
#define CLK_IN_CAPTURE 		10 			// Clock for Input Capture (microseconds)
#define TIM_RELOAD_VALUE 	(30000 / CLK_IN_CAPTURE)

#define SIZE_MESSAGE 		50 			// Do not change this one : 2x32 bits for command + 3 states for initialization
#define SIZE_REGISTER 		105 			// Supports 1 normal message (67 + 1 Idle states) + 3 repeats (3 + 1 Idle states each)
#define MAXK 				40

#include "main.h"

uint32_t selectedID;
uint8_t isID = 0;
uint32_t messageRC = 0;
uint32_t tempMess;
uint8_t messAvailable = 0;
uint8_t comptBits = 0;
uint8_t stateRC = 0;
uint8_t onState = 1;

uint8_t funcMode = 0;
uint8_t funcSpeed = 10;
uint8_t funcLuminosity = 50;

uint8_t RValue = 31;
uint8_t GValue = 31;
uint8_t BValue = 31;

uint16_t lengthStates[SIZE_REGISTER];
uint16_t tempMessage[SIZE_MESSAGE];



/*
 * =====================================================================================
 * ========================== For Method 2 : Input Capture =============================
 *
 * Call this function just one time at the beginning, to initialize Timer DMA, and Pin
 * Use Input Capture with Timer
 * Use DMA request
 * PB10 - TIM2 CH3 - DMA1 CH1
 * =====================================================================================
 */
void IR_IC_ReadSignal(void)
{
	// TIM3 Ch1 at PC6 for Testing
	// DMA1 Channel 6

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 							//
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_1); 				// AF2 for TIM3 CH2

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // Enable Clock for TIM17
//*
	////////////////////// For TIM3 CH1 (PC6) ////////////////////////////
	TIM_IN_CAPTURE->DIER |= TIM_DIER_CC2DE; 							// DMA request enable for CC1
	TIM_IN_CAPTURE->CCMR2 |= (uint16_t)(1 << 8); 						// CCR1 Linked to TI1
	TIM_IN_CAPTURE->CCMR2 |= 0x3000; 									// Filter 8 clock cycle (IC1F = 3)
	TIM_IN_CAPTURE->CCER |= TIM_CCER_CC4NP | TIM_CCER_CC2P; 			// Trigger on both edges (page 508)
	//TIM_IN_CAPTURE->CCMR2 &= ~TIM_CCMR2_IC2PSC; 						// No prescaler on Input Capture
	TIM_IN_CAPTURE->CCER |= TIM_CCER_CC2E; 								// Enable Input Capture
	//////////////////////////////////////////////////////////////////////
/*/
	////////////////////// For TIM2 CH3 (PB10) /////////////////////////// (change the peripheral line 366, for DMA too)
	TIM_IN_CAPTURE->DIER |= TIM_DIER_CC3DE; 							// DMA request enable for CC1
	TIM_IN_CAPTURE->CCMR2 |= 0x1; 										// CCR1 Linked to TI1
	TIM_IN_CAPTURE->CCMR2 |= 0x30; 										// Filter 8 clock cycle
	TIM_IN_CAPTURE->CCER |= TIM_CCER_CC3NP | TIM_CCER_CC3P; 			// Trigger on both edges (page 508)
	TIM_IN_CAPTURE->CCMR2 &= ~TIM_CCMR2_IC3PSC; 						// No prescaler on Input Capture
	TIM_IN_CAPTURE->CCER |= TIM_CCER_CC3E; 								// Enable Input Capture
	//////////////////////////////////////////////////////////////////////
//*/
	TIM_IN_CAPTURE->PSC = (SYSTEMFRQ * CLK_IN_CAPTURE / 1000000) - 1; 	// Set the prescaler

	TIM_IN_CAPTURE->ARR = TIM_RELOAD_VALUE - 1;


	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA1_Channel4->CCR &= 0xFFFF8000; 						// Reset CCR Register
	DMA1_Channel4->CNDTR &= 0xFFFF0000; 					// Reset CNDTR Register

	DMA1_Channel4->CCR |= 0b10 << 12; 						// Low Priority Channel - The rate is very low ( > 0.5ms), doesn't need a high priority
	DMA1_Channel4->CCR |= 0b01 << 10; 						// 16-bits Memory Size
	DMA1_Channel4->CCR |= 0b01 << 8; 						// 16-bits Peripheral Size
	DMA1_Channel4->CCR |= DMA_CCR_MINC;						// Memory Auto-Increment
	DMA1_Channel4->CCR |= DMA_CCR_CIRC;						// Circular Mode - So the sequence is recorded entirely even if the sequence is decoded while recorded
	DMA1_Channel4->CCR &= ~DMA_CCR_DIR;						// Read from Peripheral

	DMA1_Channel4->CNDTR |= SIZE_REGISTER; 					// Number of Data in Memory before looping

	DMA1_Channel4->CPAR = (uint32_t) (&(TIM_IN_CAPTURE->CCR2)); 			// Address for TIM3 CCR1 (Counter) - For TIM3 CH1 (PC6)

	DMA1_Channel4->CMAR = (uint32_t) lengthStates; 			// Initial Address for Memory

	DMA1_Channel4->CCR |= DMA_CCR_EN; 						// Channel Enable
	TIM_IN_CAPTURE->CR1 |= TIM_CR1_CEN;
}

/*
 * =====================================================================================================
 * ================================== For Method 2 : Input Capture =====================================
 * =========================== This function takes a lot of time : ~108us ==============================
 *
 * Call this function to decode the command stored in Memory
 * Once the command has been read, Memory is reset (not for testing)
 *
 * Returns 0 if no new message
 * Returns 0xFFFFFFFF if the sequence is incorrect
 * Returns 0x66666666 if the sequence is correct (apparently), but doesn't correspond to any command
 * Returns IR_REPEAT if we have a repeat sequence (repeated every 107ms)
 * Returns 32-bits command if the sequence is correct and corresponds to an existing command
 * =====================================================================================================
 */
uint32_t IR_IC_Decode(void)
{
	uint8_t i, j, k = 0;
	uint8_t indexTemp[SIZE_REGISTER + 1];

	tempMess = 0;
	comptBits = 0;

	DMA1_Channel4->CCR &= ~DMA_CCR_EN; 						// Channel Disable
	//////////////////////////////////////////////////////
	// Select first all potential Index 				//
	// A potential Index starts with long low level 	//
	// Then there is a standard 0 or 1 				 	//
	//////////////////////////////////////////////////////
	for (i = 0 ; i < (SIZE_REGISTER - 2) ; i++)
	{
		if (((lengthStates[i + 1] > (LOW_START / CLK_IN_CAPTURE) + lengthStates[i]) && \
				((((lengthStates[i + 2] - lengthStates[i + 1]) >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[i + 2] - lengthStates[i + 1]) <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE))) || \
				(((lengthStates[i + 2] + TIM_RELOAD_VALUE - lengthStates[i + 1]) >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[i + 2] + TIM_RELOAD_VALUE - lengthStates[i + 1]) <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE))) || \
				(((lengthStates[i + 2] - lengthStates[i + 1]) >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[i + 2] - lengthStates[i + 1]) <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE))) || \
				(((lengthStates[i + 2] + TIM_RELOAD_VALUE - lengthStates[i + 1]) >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[i + 2] + TIM_RELOAD_VALUE - lengthStates[i + 1]) <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE))))) || \

				((lengthStates[i + 1] < TIM_RELOAD_VALUE) && ((lengthStates[i + 1] + TIM_RELOAD_VALUE) > (LOW_START / CLK_IN_CAPTURE) + lengthStates[i]) && \
				((((lengthStates[i + 2] - lengthStates[i + 1]) >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[i + 2] - lengthStates[i + 1]) <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE))) || \
				(((lengthStates[i + 2] - lengthStates[i + 1]) >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[i + 2] - lengthStates[i + 1]) <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE))))))
		{
			indexTemp[k] = i + 1;
			k += 1;
		}
	}
	if (((lengthStates[SIZE_REGISTER - 1] > (LOW_START / CLK_IN_CAPTURE) + lengthStates[SIZE_REGISTER - 2]) && \
			((((lengthStates[0] - lengthStates[SIZE_REGISTER - 1]) >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[0] - lengthStates[SIZE_REGISTER - 1]) <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE))) || \
			(((lengthStates[0] + TIM_RELOAD_VALUE - lengthStates[SIZE_REGISTER - 1]) >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[0] + TIM_RELOAD_VALUE - lengthStates[SIZE_REGISTER - 1]) <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE))) || \
			(((lengthStates[0] - lengthStates[SIZE_REGISTER - 1]) >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[0] - lengthStates[SIZE_REGISTER - 1]) <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE))) || \
			(((lengthStates[0] + TIM_RELOAD_VALUE - lengthStates[SIZE_REGISTER - 1]) >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[0] + TIM_RELOAD_VALUE - lengthStates[SIZE_REGISTER - 1]) <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE))))) || \

			((lengthStates[SIZE_REGISTER - 1] < TIM_RELOAD_VALUE) && ((lengthStates[SIZE_REGISTER - 1] + TIM_RELOAD_VALUE) > (LOW_START / CLK_IN_CAPTURE) + lengthStates[SIZE_REGISTER - 2]) && \
			((((lengthStates[0] - lengthStates[SIZE_REGISTER - 1]) >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[0] - lengthStates[SIZE_REGISTER - 1]) <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE))) || \
			(((lengthStates[0] - lengthStates[SIZE_REGISTER - 1]) >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[0] - lengthStates[SIZE_REGISTER - 1]) <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE))))))
	{
		indexTemp[k] = SIZE_REGISTER - 1;
		k += 1;
	}
	if (((lengthStates[0] > (LOW_START / CLK_IN_CAPTURE) + lengthStates[SIZE_REGISTER - 1]) && \
			((((lengthStates[1] - lengthStates[0]) >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[1] - lengthStates[0]) <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE))) || \
			(((lengthStates[1] + TIM_RELOAD_VALUE - lengthStates[0]) >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[1] + TIM_RELOAD_VALUE - lengthStates[0]) <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE))) || \
			(((lengthStates[1] - lengthStates[0]) >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[1] - lengthStates[0]) <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE))) || \
			(((lengthStates[1] + TIM_RELOAD_VALUE - lengthStates[0]) >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[1] + TIM_RELOAD_VALUE - lengthStates[0]) <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE))))) || \

			((lengthStates[0] < TIM_RELOAD_VALUE) && ((lengthStates[0] + TIM_RELOAD_VALUE) > (LOW_START / CLK_IN_CAPTURE) + lengthStates[SIZE_REGISTER - 1]) && \
			((((lengthStates[1] - lengthStates[0]) >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[1] - lengthStates[0]) <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE))) || \
			(((lengthStates[1] - lengthStates[0]) >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && ((lengthStates[1] - lengthStates[0]) <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE))))))
	{
		indexTemp[k] = 0;
		k += 1;
	}

	if (k > MAXK) 			// Limit number of indexes.
		k = MAXK;



	for (j = 0 ; j < k ; j++)
	{
	//////////////////////////////////////////////////
	// Set times in the right Order in Memory 		//
	// The first time is the startIndex one 		//
	//////////////////////////////////////////////////
		for (i = 0 ; i < SIZE_MESSAGE ; i++)
		{
			if ((indexTemp[j] + i) >= SIZE_REGISTER)
				tempMessage[i] = lengthStates[indexTemp[j] + i - SIZE_REGISTER];
			else
				tempMessage[i] = lengthStates[indexTemp[j] + i];
		}


	//////////////////////////////////////////////////////////////
	// Set Values Relatively 									//
	// Length is the difference between 2 times 				//
	// When the counter reach the reload value, it restarts 	//
	// We add the reload value if the time is decreasing 		//
	// tempMessage is used for decoding, not lengthStates 		//
	// tempMessage is actualized each time (not for testing) 	//
	//////////////////////////////////////////////////////////////
		for (i = 0 ; i < (SIZE_MESSAGE - 1) ; i++)
		{
			if (tempMessage[i] > tempMessage[i + 1])
				tempMessage[i] = TIM_RELOAD_VALUE + tempMessage[i + 1] - tempMessage[i];
			else
				tempMessage[i] = tempMessage[i + 1] - tempMessage[i];

		}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Now we start decoding the message 																	//
	// Returns 25-bits command if the sequence is correct 													//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
				for (i = 0 ; i < ((LENGTH_MESSAGE * 2) - 1) ; i++) 					// Read 25 bits
				{
					if ((i % 2) == 0)
					{
						if ((tempMessage[i] >= ((HIGH_0 - DELTABIT) / CLK_IN_CAPTURE)) && (tempMessage[i] <= ((HIGH_0 + DELTABIT) / CLK_IN_CAPTURE)))
						{
							tempMess &= ~(((uint32_t) 1) << comptBits);
							comptBits ++;
						}
						else if ((tempMessage[i] >= ((HIGH_1 - DELTABIT) / CLK_IN_CAPTURE)) && (tempMessage[i] <= ((HIGH_1 + DELTABIT) / CLK_IN_CAPTURE)))
						{
							tempMess |= (((uint32_t) 1) << comptBits);
							comptBits ++;
						}
						else
						{
							comptBits = 0;
							tempMess = 0;
							i = LENGTH_MESSAGE * 2;
						}
					}
					else 				// Check if the low level corresponds to the high level bit
					{
						if ((tempMessage[i] >= ((LOW_0 - DELTABIT) / CLK_IN_CAPTURE)) && (tempMessage[i] <= ((LOW_0 + DELTABIT) / CLK_IN_CAPTURE)) && ((tempMess & (((uint32_t) 1) << (comptBits - 1))) == 0))
						{
						}
						else if ((tempMessage[i] >= ((LOW_1 - DELTABIT) / CLK_IN_CAPTURE)) && (tempMessage[i] <= ((LOW_1 + DELTABIT) / CLK_IN_CAPTURE)) && ((tempMess & (((uint32_t) 1) << (comptBits - 1))) == (((uint32_t) 1) << (comptBits - 1))))
						{
						}
						else 			// If it does not correspond, the sequence is not valid
						{
							comptBits = 0;
							tempMess = 0;
							i = LENGTH_MESSAGE * 2;
						}
					}
				}

				if (comptBits == LENGTH_MESSAGE)
				{
					messageRC = tempMess;
					for (i = 0 ; i < SIZE_REGISTER ; i++)
						lengthStates[i] = 0; 								// Reset After Reading if the message is correct
					RC_Decode(); 											// Call function associated with the command
					DMA1_Channel4->CCR |= DMA_CCR_EN; 						// Channel Enable

					return messageRC;
				}
	}

////////////////////////////////////////////////

	DMA1_Channel4->CCR |= DMA_CCR_EN; 						// Channel Enable
	return 0;
}

void RC_Conf(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 						//
}

void RC_InitID(void)
{
	if (((messageRC >> 12) & 0x00001FFF) == MESS_ONOFF)
	{
		selectedID = messageRC & 0x00000FFF;
		FLASH_FastUpdate16Bits((uint16_t)selectedID, ADD_ID);
		isID = 1;
	}
	else
		selectedID = FLASH_Read(ADD_ID);
	DATA_RegulatorR(0); //////////////////////////////
	DATA_RegulatorG(0); //////////////////////////////
	DATA_RegulatorB(0); //////////////////////////////
	//isID = 1;
}

void RC_Decode(void)
{
	messAvailable = 0;

	if (isID == 0)
		RC_InitID();
	else if ((messageRC & 0x00000FFF) == selectedID)
	{
		switch ((messageRC >> 12) & 0x00001FFF)
		{
		case MESS_ONOFF:
			RC_FuncOnOff();
			break;
		case MESS_PAUSE:
			RC_FuncPause();
			break;
		case MESS_LUMLOW:
			RC_FuncLumLow();
			break;
		case MESS_LUMHIGH:
			RC_FuncLumHigh();
			break;
		case MESS_WHITE:
			RC_FuncWhite();
			break;
		case MESS_BLUE:
			RC_FuncBlue();
			break;
		case MESS_GREEN:
			RC_FuncGreen();
			break;
		case MESS_RED:
			RC_FuncRed();
			break;
		case MESS_PURPLE:
			RC_FuncPurple();
			break;
		case MESS_CYAN:
			RC_FuncCyan();
			break;
		case MESS_YELLOW:
			RC_FuncYellow();
			break;
		case MESS_ORANGE:
			RC_FuncOrange();
			break;
		case MESS_SPEEDP:
			RC_FuncSpeedP();
			break;
		case MESS_FADE3:
			RC_FuncFade3();
			break;
		case MESS_JUMP3:
			RC_FuncJump3();
			break;
		case MESS_AUTO:
			RC_FuncAuto();
			break;
		case MESS_SPEEDM:
			RC_FuncSpeedM();
			break;
		case MESS_FADE7:
			RC_FuncFade7();
			break;
		case MESS_JUMP7:
			RC_FuncJump7();
			break;
		case MESS_FLASH:
			RC_FuncFlash();
			break;
		default:
			break;
		}
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncOnOff(void)
{
	funcMode = 0;
	if (onState)
	{
		onState = 0;
		//TIM_FUNC->CR1 &= ~TIM_CR1_CEN;
	}
	else
	{
		onState = 1;
		//TIM_FUNC->CR1 |= TIM_CR1_CEN;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncPause(void)
{
	if (onState)
	{
		if ((TIM_FUNC->CR1 & TIM_CR1_CEN) == TIM_CR1_CEN)
			TIM_FUNC->CR1 &= ~TIM_CR1_CEN;
		else
			TIM_FUNC->CR1 |= TIM_CR1_CEN;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncLumLow(void)
{
	if (onState)
	{
		if (funcLuminosity >= 20)
			funcLuminosity -= 10;
		else
			funcLuminosity = 10;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncLumHigh(void)
{
	if (onState)
	{
		if (funcLuminosity <= 90)
			funcLuminosity += 10;
		else
			funcLuminosity = 100;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncWhite(void)
{
	if (onState)
	{
		funcMode = 0;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncBlue(void)
{
	if (onState)
	{
		funcMode = 0;
		DATA_RegulatorB(31);
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncGreen(void)
{
	if (onState)
	{
		funcMode = 0;
		DATA_RegulatorG(31);
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncRed(void)
{
	if (onState)
	{
		funcMode = 0;
		DATA_RegulatorR(31);
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncPurple(void)
{
	if (onState)
	{
		funcMode = 0;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncCyan(void)
{
	if (onState)
	{
		funcMode = 0;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncYellow(void)
{
	if (onState)
	{
		funcMode = 0;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncOrange(void)
{
	if (onState)
	{
		funcMode = 0;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncSpeedP(void)
{
	if (onState)
	{
		if (funcSpeed >= 2)
			funcSpeed -= 1;
		else
			funcSpeed = 1;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncFade3(void)
{
	if (onState)
		funcMode = 4;
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncJump3(void)
{
	if (onState)
		funcMode = 3;
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncAuto(void)
{
	if (onState)
		funcMode = 1;
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncSpeedM(void)
{
	if (onState)
	{
		if (funcSpeed <= 9)
			funcSpeed += 1;
		else
			funcSpeed = 10;
	}
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncFade7(void)
{
	if (onState)
		funcMode = 6;
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncJump7(void)
{
	if (onState)
		funcMode = 5;
}

/*
 * ========================
 *
 * ========================
 */
void RC_FuncFlash(void)
{
	if (onState)
		funcMode = 2;
}



