/*
 *  Project:      Bumblecopter
 *  File:         dac.c
 *  Author:       Gerd Bartelt - www.sebulli.com - www.sebulli.com
 *
 *  Description:  Used the DAC to output analog debug values
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "dac.h"
#include "project.h"

/**
 * Initialize the DAC
 */
void dac_init(void) {
#ifdef USE_DAC

	GPIO_InitTypeDef GPIO_InitStructure;
	  DAC_InitTypeDef DAC_InitStructure;

	  //GPIOA clock enable (to be used with DAC)
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	  // DAC Periph clock enable
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	  // DAC port (DAC_OUT1 = PA.4) configuration
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);

	  // DAC channel 1 configuration
	  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	  // Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is
	  // automatically connected to the DAC converter.
	  DAC_Cmd(DAC_Channel_1, ENABLE);

	  // Set DAC channel register
	  DAC_SetChannel1Data(DAC_Align_12b_R, 0);
#endif
}
/**
 * Sets the DAC
 *
 * @param
 * 			value DAC value from 0..4095
 */
void dac_setValue(uint16_t value) {
#ifdef USE_DAC
	  DAC_SetChannel1Data(DAC_Align_12b_R, value & 0x0FFF);
#endif
}
