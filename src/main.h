/*
 * main.h
 *
 *  Created on: Mar 23, 2017
 *      Author: Thomas
 */

#ifndef MAIN_H_
#define MAIN_H_


#include <stddef.h>
//#include "stm32f0xx_rcc.h"
//#include "stm32f0xx_gpio.h"
#include "stm32f0xx_conf.h"

#define SYSTEMFRQ 			48000000

#define NB_LED 				75

#define TIM_FUNC 			TIM17

// TIMES IN MICROSECONDS
#define LOW_START 			1500
#define LOW_0 				400
#define HIGH_0 				160
#define LOW_1 				120
#define HIGH_1 				440
#define DELTABIT 			100
#define LENGTH_BIT 			(LOW_0 + HIGH_0)

// LENGTH IN BITS
#define LENGTH_ID 			12
#define LENGTH_MESSAGE 		25
// MESSAGES
#define MESS_ONOFF 			0b0000000110000
#define MESS_PAUSE 			0b0000011000000
#define MESS_LUMLOW 		0b0001100000000
#define MESS_LUMHIGH 		0b0110000000000
#define MESS_WHITE 			0b0001111000000
#define MESS_BLUE 			0b0110000110000
#define MESS_GREEN 			0b0110011000000
#define MESS_RED 			0b0111100000000
#define MESS_PURPLE 		0b0110000000011
#define MESS_CYAN 			0b0110000001100
#define MESS_YELLOW 		0b0000011110000
#define MESS_ORANGE 		0b0001100110000
#define MESS_SPEEDP 		0b0000011001100
#define MESS_FADE3 			0b0000011000011
#define MESS_JUMP3 			0b0001100000011
#define MESS_AUTO 			0b0001100001100
#define MESS_SPEEDM 		0b0000000000011
#define MESS_FADE7 			0b0000000001100
#define MESS_JUMP7 			0b0000000111100
#define MESS_FLASH 			0b0000000110011

#define ADD_ID 				0x0000

#define SAMP_12V 			1943


inline void Delay(uint32_t Time);
inline void Delay20(uint32_t Time);

//inline void Wait400ns();
//inline void Wait800ns();
//inline void Wait450ns();
//inline void Wait850ns();

void IR_IC_ReadSignal(void);
uint32_t IR_IC_Decode(void);
void RC_Conf(void);
void RC_InitID(void);
void RC_Read(void);
void RC_Decode(void);

void RC_FuncOnOff(void);
void RC_FuncPause(void);
void RC_FuncLumLow(void);
void RC_FuncLumHigh(void);
void RC_FuncWhite(void);
void RC_FuncBlue(void);
void RC_FuncGreen(void);
void RC_FuncRed(void);
void RC_FuncPurple(void);
void RC_FuncCyan(void);
void RC_FuncYellow(void);
void RC_FuncOrange(void);
void RC_FuncSpeedP(void);
void RC_FuncFade3(void);
void RC_FuncJump3(void);
void RC_FuncAuto(void);
void RC_FuncSpeedM(void);
void RC_FuncFade7(void);
void RC_FuncJump7(void);
void RC_FuncFlash(void);

void ADC_InitGPIOs(void);
void ADC_Conf(void);
void ADC_ConfigChannels(void);

void DATA_InitGPIOs(void);
void DATA_Conf(void);
void DATA_RegulatorR(uint16_t phase);
void DATA_RegulatorG(uint16_t phase);
void DATA_RegulatorB(uint16_t phase);
void DATA_SetColor(uint8_t percentR, uint8_t percentG, uint8_t percentB);
void DATA_PlayFrame(void);
void DATA_SetNewFrame(uint8_t actFrame);
void DATA_SetLEDColor(int16_t IdLED, uint8_t pcR, uint8_t pcG, uint8_t pcB);

uint16_t FLASH_FastUpdate16Bits(uint16_t flashDat, uint32_t flashAdd);
uint16_t FLASH_Read(uint32_t flashAdd);


extern uint16_t dataWS[NB_LED];
extern uint16_t sampValue;
extern uint8_t actualFrame;

extern uint32_t selectedID;
extern uint8_t isID;
extern uint32_t messageRC;
extern uint32_t tempMess;
extern uint8_t messAvailable;
extern uint8_t comptBits;
extern uint8_t stateRC;

extern uint8_t funcMode;
extern uint8_t funcSpeed;
extern uint8_t funcLuminosity;

extern uint8_t RValue;
extern uint8_t GValue;
extern uint8_t BValue;


#endif /* MAIN_H_ */
