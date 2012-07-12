/*
 *  Project:      Bumblecopter
 *  File:         power.c
 *  Author:       Gerd Bartelt
 *
 *  Description:  Measures the battery voltage and sets under voltage errors
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
#include "power.h"
#include "adc.h"
#include "light.h"
#include "typedef.h"

uint32_t ubat_filt = 0;
uint32_t ubat_filtL = 0;
int powerUpCnt = 0;

#define UBAT_FILT 8192
#define LOW_BAT_L 10500
#define LOW_BAT_U 11000

int lowbat = 0;

void power_init(void) {
	powerUpCnt = 100;
	lowbat = 0;
}

uint32_t power_getUbat() {
	// 3.3V
	// Voltage divider 10:2k35 = 1:5,255319149
	// 3300 / 4096 *5,255.. = 271 / 64
	return adc_getResult(ADC_CHAN_BATT) * 271/64;
}

void power_task(void){
	if (powerUpCnt > 0) {
		ubat_filt = power_getUbat();
		ubat_filtL = ubat_filt * UBAT_FILT;
		powerUpCnt--;
	}
	else {
		ubat_filtL += power_getUbat() - ubat_filt;
		ubat_filt = ubat_filtL / UBAT_FILT;
	}

	// Set and reset low batt warning
	if (ubat_filt < LOW_BAT_L) lowbat = 1;
	if (ubat_filt > LOW_BAT_U) lowbat = 0;

	if (lowbat) {
		light_set_error(LOWBAT);
	}

}

uint32_t power_getUbatFilt() {
	return ubat_filt;
}

