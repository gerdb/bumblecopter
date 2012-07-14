/*
 *  Project:      Bumblecopter
 *  File:         controller.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Calculates the angle based on the polarized light
 *                or the gyro sensor
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
#include "rotation.h"
#include "spi.h"
#include "adc.h"

__IO int32_t integrator=0;
__IO int32_t gyro_offset=-8;
__IO int pol_lp_filt = 0;
__IO int pol_hp_filt = 0;
__IO int32_t pol_lp_filtL = 0;
__IO int pol_act = 0;
int fir_buf[256];
int fir_cnt = 0;
int32_t fir_sum = 0;
int firSign = 0;
int firSign_old = 0;
int fir_supressSignCnt = 0;
uint16_t posEdge = 0;
uint16_t negEdge = 0;
uint16_t periode = 0;
uint16_t median = 0;
uint16_t phasecnt = 0;
uint16_t relphasecnt = 0;
uint32_t phase = 0;

#define POLFILT 8192
#define SUPPRESSSIGN 8192
#define MAXSIGNCNT 30000

void rotation_init(void) {
	int i;
	for (i=0;i<256;i++) {
		fir_buf[i] = 0;
	}

	fir_sum = 0;
	fir_cnt = 0;
}

void rotation_task(void) {
	integrator+= spi_getGyro() + gyro_offset;

	// get value of polarization sensor
	pol_act = adc_getResult(ADC_CHAN_POL);

	// low pass filter
	pol_lp_filtL += pol_act-pol_lp_filt;
	pol_lp_filt = pol_lp_filtL / POLFILT;

	// high pass filter
	pol_hp_filt = pol_act - pol_lp_filt;

	// FIR filter
	fir_sum+= pol_hp_filt;
	fir_buf[fir_cnt] = pol_hp_filt;
	fir_cnt++;
	fir_cnt &= 0x00ff;
	fir_sum-= fir_buf[fir_cnt];

	phasecnt++;

	if (fir_supressSignCnt < SUPPRESSSIGN) {
		fir_supressSignCnt++;
	}
	else {
		firSign = (fir_sum > 0);

		// positive edge detected
		if ((firSign == !0) && (firSign_old == 0) ) {
			posEdge = phasecnt;
			fir_supressSignCnt = 0;
		}
		// negative edge detected
		if ((firSign == !0) && (firSign_old == 0) ) {
			periode =  phasecnt - negEdge;
			negEdge = phasecnt;
			if (negEdge >= posEdge) {
				median = ((uint32_t)negEdge + (uint32_t)posEdge) / 2;
			}
			else {
				median = (((uint32_t)negEdge + 65536L + (uint32_t)posEdge) / 2) & 0x0000ffff;
			}
			fir_supressSignCnt = 0;
		}
		firSign_old = fir_sum;

		relphasecnt = phasecnt - median;

		phase = relphasecnt * 256 / periode;
	}



}

int rotation_getPolFilt(void) {
	return fir_sum;
}

int rotation_getAngle(void) {
	return integrator;
}
