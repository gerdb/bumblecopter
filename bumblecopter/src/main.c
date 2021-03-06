/*
 *  Project:      Bumblecopter
 *  File:         main.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  main programm
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

#include "stm32f4xx.h"

#include "light.h"
#include "main.h"
#include "adc.h"
#include "pwm.h"
#include "rc.h"
#include "spi.h"
#include "usart.h"
#include "rotation.h"
#include "controller.h"
#include "power.h"
#include "dac.h"
#include "wings.h"
#include "project.h"

extern int32_t KP;
extern int32_t KD;

int tick = 0;
e_sysstate sysstate = IDLE;
int step= 600;


int main(void) {
	int taskcnt = 0;
//	int t2cnt = 0;
	int time = 0;
//	int16_t pwm = 0;
	int16_t rc = 0;

	// initialize all modules
	light_init();
	adc_init();
	dac_init();
	pwm_init();
	rc_init();
	spi_init();
	usart_init();
	controller_init();
	power_init();
	rotation_init();
	wings_init();

	sysstate = GYRO;

	// set sys tick to 100us
	SysTick_Config(SystemCoreClock / 10000);

	while (1) {

		// wait for task tick
		while (tick == 0);
		tick = 0;

		time++;
		usart_buffertask();
		spi_machine();
		rotation_task();
		power_task();
		wings_task(sysstate);
		light_task(sysstate);



		if (time == 2000) {
			sysstate = SUN;
			//light_set_error(ACTOR);
			time = 0;
			//my_printf("%d,%d,%d,%d,%d,%d,%d\r\n",adc_getResult(3),adc_getResult(4),reg,KP,KI,KD,step);
			//my_printf("%d\r\n",rotation_getAngle());
			//my_printf("%d\r\n",power_getUbatFilt());
//			t2cnt ++;
//			if (t2cnt == 10) {
//				t2cnt = 0;
//				if (pwm == step) {
//					pwm = -step;
//				}
//				else {
//					pwm = step;
//				}
//			}
//			controller_setSetpoint(0,pwm);
//			controller_setSetpoint(1,pwm);
//			controller_setSetpoint(2,pwm);
//			controller_setSetpoint(3,pwm);

			rc ++;
			if (rc == 99) {
				rc = -99;
			}
			rc_set(rc);



		}

		// IDLE mode, if the left joystick is pulled down
		if ( rc_get_channel(RC_CHANNEL_SPEED) < -30) {

			// Use the simulated mode if the left joystick is pulled to the
			// left down position
			if ( (rc_get_channel(RC_CHANNEL_TEST) > 10 ) &&
					( rc_get_channel(RC_CHANNEL_SWITCH) > 0) ){
				sysstate = SUNSIMULATION;
				rotation_useSimulation(rc_get_channel(RC_CHANNEL_TEST)-10);
			} else {
				sysstate = IDLE;
			}
		}
		else {

			// Switch between GYRO and SUN mode with the channel 5 switch
			if ( rc_get_channel(RC_CHANNEL_SWITCH) > 0) {
				sysstate = SUN;
			}
			else {
				sysstate = GYRO;
			}
		}

		// Do not use the simulated value
		if ( sysstate != SUNSIMULATION) {
			rotation_useSimulation(0);
		}

		taskcnt ++;
		if (taskcnt >= 4) {

			taskcnt = 0;
		}
		controller_task (taskcnt);
		adc_start_conv();


//		controller_setSetpoint(0,rc_get_channel(RC_CHANNEL_SPEED)*16);
//		controller_setSetpoint(1,rc_get_channel(RC_CHANNEL_SPEED)*16);
//		controller_setSetpoint(2,rc_get_channel(RC_CHANNEL_SPEED)*16);
//		controller_setSetpoint(3,rc_get_channel(RC_CHANNEL_SPEED)*16);

	}
}



void setTick(void) {
	tick = 1;
}
