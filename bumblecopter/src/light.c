/*
 *  Project:      Bumblecopter
 *  File:         light.c
 *  Author:       Gerd Bartelt
 *
 *  Description:  Controls the LEDs and used them to display
 *                the rotation angle or error states
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
#include "light.h"
#include "rotation.h"

GPIO_InitTypeDef  GPIO_InitStructure;
int phase;
int on;
e_sysstate sysstate_old = OFF;
int error_cnt=0;
int error_tim=0;
e_errstate error = OK;
e_errstate new_error = OK;
int light_m_cnt = 0;

void light_init(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOB, ENABLE);

	/* Configure PB9 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//Switch on yellow power LED
	GPIOB->BSRRL = GPIO_Pin_9;

	/* Configure PE0..3 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = LED_G_1 | LED_R_1 |LED_G_2 |LED_R_2 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	phase = 0;
	on	= 0;
	LED_OFF(LED_G_1 | LED_R_1 |LED_G_2 |LED_R_2) ;
}

void light_update(){
	if (on) {
		switch (phase) {
		case 0:
			LED_ON(LED_G_1 | LED_G_2); // switch on  LEDs
			LED_OFF(LED_R_1 | LED_R_2); // switch off LEDs
			break;
		case 1:
			LED_ON(LED_G_1 | LED_R_2); // switch on  LEDs
			LED_OFF(LED_R_1 | LED_G_2); // switch off LEDs
			break;
		case 2:
			LED_ON(LED_R_1 | LED_R_2); // switch on  LEDs
			LED_OFF(LED_G_1 | LED_G_2); // switch off LEDs
			break;
		case 3:
			LED_ON(LED_R_1 | LED_G_2); // switch on  LEDs
			LED_OFF(LED_G_1 | LED_R_2); // switch off LEDs
			break;
		}
	}
	else {
		LED_OFF(LED_G_1 | LED_R_1 |LED_G_2 |LED_R_2) ;
	}
}

void light_set_phase(int p_phase) {
	phase = p_phase;
	light_update();
}

void light_set_on(int p_on) {
	on = p_on;
	light_update();
}

void light_set_error(e_errstate e) {
	if (e != OFF) {
		if ((new_error == OFF) || (e < new_error)) {
			new_error = e;
		}
		if ((error == OFF) || (e < error)) {
			error = e;
		}
	}
}

void light_power_led(int p_on) {
	if (p_on == ON) {
		// swtich on LED
		GPIOB->BSRRL = GPIO_Pin_9;
	}
	else {
		// swtich off LED
		GPIOB->BSRRH = GPIO_Pin_9;
	}
}


void light_task(e_sysstate sysstate) {

	if (error != OK) {
		error_tim++;
		if (error_tim > 3500) { // 350ms
			error_tim=0;
			error_cnt++;
			if (error_cnt>error) {
				error_cnt = -5;
				// take the new error
				error = new_error;
				new_error = OK;
			}
		}

//		if (error_tim < 1000 && error_cnt> 0) {
//			LED_ON(LED_R_1 | LED_R_2);
//			LED_OFF(LED_G_1 | LED_G_2);
//		}
//		else {
//			LED_OFF(LED_R_1 | LED_R_2 | LED_G_1 | LED_G_2); // switch off LEDs
//		}
//		return;

		if (error_tim > 1000 && error_cnt> 0) {
			LED_OFF(LED_R_1 | LED_R_2 | LED_G_1 | LED_G_2); // switch off LEDs
			return;
		}

	}

	if (sysstate!=sysstate_old) {
		light_m_cnt = 2500; // 250ms
		sysstate_old = sysstate;
	}

	if (light_m_cnt>0) {
		light_m_cnt--;
		LED_OFF(LED_R_1 | LED_R_2 | LED_G_1 | LED_G_2); // switch off LEDs
		return;
	}

	switch (sysstate) {
	case OFF:
		LED_OFF(LED_R_1 | LED_R_2 | LED_G_1 | LED_G_2); // switch off LEDs
		break;
	case IDLE:
		LED_ON(LED_G_1 | LED_G_2);
		LED_OFF(LED_R_1 | LED_R_2);
		break;
	case GYRO:
		if (rotation_getAngle() & 0x00100000) {
			LED_ON(LED_R_1 | LED_R_2);
			LED_OFF(LED_G_1 | LED_G_2);
		}
		else {
			LED_ON(LED_G_1 | LED_G_2);
			LED_OFF(LED_R_1 | LED_R_2);
		}
	break;
	case SUN:
		break;
	}


}
