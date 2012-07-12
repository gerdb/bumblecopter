/*
 *  Project:      Bumblecopter
 *  File:         controller.c
 *  Author:       Gerd Bartelt
 *
 *  Description:  PD controller for the high speed actuators
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


#include "controller.h"
#include "adc.h"
#include "pwm.h"


int32_t meas;
int ad_channel_map[4][2];
int32_t rdiff;
int32_t meas_old[4]={0,0,0,0};
int32_t reg = 0;
int32_t setpoint[4]={0,0,0,0};
int32_t KP = 120; //120
int32_t KI = 0;
int32_t KD = 300; //-300
#define CONTR_LIMIT 1000

/**
 * Initialize the controller and configure the analog channels
 */
void controller_init(void) {

	// Create a table with the index of all analog channels
	ad_channel_map[0][0]=3;
	ad_channel_map[0][1]=4;

	ad_channel_map[1][0]=2;
	ad_channel_map[1][1]=5;

	ad_channel_map[2][0]=1;
	ad_channel_map[2][1]=6;

	ad_channel_map[3][0]=0;
	ad_channel_map[3][1]=7;

}
/**
 * Setter for the controllers setpoint
 *
 * @param
 * 			channel Channel no from 0 to 3
 * 			setp  Setpoint from -1000 to +1000
 *
 */
void controller_setSetpoint(int channel, int setp) {
	setpoint[channel] = setp;
}

/**
 * Controller task
 * Call this every 100µs
 *
 * @param
 * 			channel Channel no from 0 to 3
 *
 */
void controller_task(int channel){

	// Get the sensor value.
	meas = adc_getResult(ad_channel_map[channel][0]) - adc_getResult(ad_channel_map[channel][1]);

	// Calculate the difference between setpoint and sensor value
	rdiff = setpoint[channel] - meas;

	// The PD regulator with a scaling factor
	reg = rdiff *KP + (meas_old[channel]-meas) * KD;
	reg /= 32;

	// Store the old value for the D-value
	meas_old[channel] = meas;

	// Limit the value
	if (reg > +CONTR_LIMIT) reg = +CONTR_LIMIT;
	if (reg < -CONTR_LIMIT) reg = -CONTR_LIMIT;

	// Set the PWM output
	pwm_set(channel,reg);
}

