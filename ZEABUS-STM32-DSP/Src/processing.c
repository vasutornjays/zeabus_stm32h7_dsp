/*
 * processing.c
 *
 *  Created on: Jun 13, 2018
 *      Author: Vasut
 */

#include "param.h"
#include "common.h"
#include "processing.h"

extern InputParam input;
extern OutputParam output;

float dw1[DATA_SIZE];
float dw2[DATA_SIZE];
float dw3[DATA_SIZE];
float dw4[DATA_SIZE];
float dx1[DATA_SIZE];
float dx2[DATA_SIZE];
float dx3[DATA_SIZE];
float dx4[DATA_SIZE];
float dy1[DATA_SIZE];
float dy2[DATA_SIZE];
float dy3[DATA_SIZE];
float dy4[DATA_SIZE];
float dz1[DATA_SIZE];
float dz2[DATA_SIZE];
float dz3[DATA_SIZE];
float dz4[DATA_SIZE];

float time = BUFFER_SIZE / SAMPLE_RATE * 1000; // multiple 1000 for change to ms

float filter_lp[163] = { 0.0005819638815361733, 0.00019879010498615907,
		0.00022767092201437183, 0.00025531297065362334, 0.0002807488034690767,
		0.00030292728623379316, 0.00032068355810150776, 0.0003326983139099703,
		0.0003376460852589169, 0.0003341933695865011, 0.00032091548182295145,
		0.000296364539390445, 0.000259263944358662, 0.00020833024331851343,
		0.00014217611865575787, 0.000059953547475915676,
		-0.000039470701644621466, -0.00015669713367315324,
		-0.0002922990664530668, -0.0004464946086915844, -0.000619304397528378,
		-0.0008103639470064903, -0.0010190058527831626, -0.0012441505215466334,
		-0.0014843683522803351, -0.0017377808029476643, -0.0020021465480106215,
		-0.0022748378647111205, -0.0025527693576020624, -0.0028325328007029393,
		-0.0031102537798806855, -0.003381755263268949, -0.0036424503626797805,
		-0.0038875404279695115, -0.00411189352951525, -0.004310296297175892,
		-0.0044772105191336255, -0.004607233224198584, -0.00469468844377201,
		-0.004734075974129997, -0.004720012262095053, -0.004647233657207179,
		-0.0045108127678204785, -0.004306182522151814, -0.0040291501121243415,
		-0.003675964090637525, -0.003243486229290978, -0.0027291478897309915,
		-0.002131306629547152, -0.0014481165371384246, -0.0006811364656148641,
		0.00017174625451366992, 0.0011072473186550713, 0.002123252706000729,
		0.0032172284812014293, 0.004385081043245875, 0.005621536004483763,
		0.0069206867969490845, 0.008275955199116843, 0.00967993286022986,
		0.011124518170530928, 0.012600984467765383, 0.014099690637281328,
		0.015610374102465277, 0.01712319800721637, 0.01862698495772486,
		0.020111229971570197, 0.02156478705195722, 0.02297668061881249,
		0.024335930556454173, 0.025631855645659486, 0.026854092299222407,
		0.027992783062529045, 0.029038665941885658, 0.029983193774586546,
		0.0308185265267006, 0.03153769063955536, 0.032134669991468245,
		0.03260433072397987, 0.032942730497619886, 0.03314692061051216,
		0.033215192546999395, 0.03314692061051216, 0.032942730497619886,
		0.03260433072397987, 0.032134669991468245, 0.03153769063955536,
		0.0308185265267006, 0.029983193774586546, 0.029038665941885658,
		0.027992783062529045, 0.026854092299222407, 0.025631855645659486,
		0.024335930556454177, 0.02297668061881249, 0.02156478705195722,
		0.020111229971570197, 0.01862698495772486, 0.01712319800721637,
		0.015610374102465277, 0.014099690637281328, 0.012600984467765383,
		0.011124518170530928, 0.00967993286022986, 0.008275955199116843,
		0.0069206867969490845, 0.005621536004483763, 0.004385081043245875,
		0.0032172284812014293, 0.002123252706000729, 0.0011072473186550713,
		0.00017174625451366992, -0.0006811364656148641, -0.0014481165371384246,
		-0.002131306629547152, -0.00272914788973099, -0.003243486229290978,
		-0.003675964090637525, -0.0040291501121243415, -0.004306182522151814,
		-0.0045108127678204785, -0.004647233657207179, -0.004720012262095053,
		-0.004734075974129997, -0.004694688443772011, -0.004607233224198584,
		-0.0044772105191336255, -0.004310296297175892, -0.00411189352951525,
		-0.0038875404279695115, -0.0036424503626797805, -0.003381755263268949,
		-0.0031102537798806855, -0.0028325328007029393, -0.0025527693576020624,
		-0.0022748378647111205, -0.0020021465480106215, -0.0017377808029476643,
		-0.0014843683522803351, -0.0012441505215466334, -0.0010190058527831626,
		-0.0008103639470064903, -0.0006193043975283784, -0.0004464946086915844,
		-0.0002922990664530668, -0.00015669713367315324,
		-0.00003947070164462148, 0.000059953547475915676,
		0.00014217611865575787, 0.0002083302433185148, 0.000259263944358662,
		0.000296364539390445, 0.00032091548182295145, 0.0003341933695865011,
		0.0003376460852589169, 0.0003326983139099703, 0.0003206835581015091,
		0.00030292728623379316, 0.000280748803469076, 0.00025531297065362334,
		0.00022767092201437183, 0.00019879010498615907, 0.0005819638815361733 };

int pulse_detect(float*in1_re, float*in1_im, float*in2_re, float*in2_im,
		float*in3_re, float*in3_im, float*in4_re, float*in4_im,
		float*out_re, float*out_im, int N, int Ns, float* pavr);

void processing(){

	float g_d_1_re[DATA_SIZE];
	float g_d_1_im[DATA_SIZE];
	float g_d_2_re[DATA_SIZE];
	float g_d_2_im[DATA_SIZE];
	float g_d_3_re[DATA_SIZE];
	float g_d_3_im[DATA_SIZE];
	float g_d_4_re[DATA_SIZE];
	float g_d_4_im[DATA_SIZE];

	float PID_output_gain;

	struct Temp dt;
		dt.w1 = (float*) (dw1);
		dt.w2 = (float*) (dw2);
		dt.w3 = (float*) (dw3);
		dt.w4 = (float*) (dw4);
		dt.x1 = (float*) (dx1);
		dt.x2 = (float*) (dx2);
		dt.x3 = (float*) (dx3);
		dt.x4 = (float*) (dx4);
		dt.y1 = (float*) (dy1);
		dt.y2 = (float*) (dy2);
		dt.y3 = (float*) (dy3);
		dt.y4 = (float*) (dy4);
		dt.z1 = (float*) (dz1);
		dt.z2 = (float*) (dz2);
		dt.z3 = (float*) (dz3);
		dt.z4 = (float*) (dz4);

	demod((float*)g_adc_1_f, (float*)g_adc_2_f, (float*)g_adc_3_f, (float*)g_adc_4_f,
		  (float*)g_d_1_re, (float*)g_d_1_im, (float*)g_d_2_re, (float*)g_d_2_im,
		  (float*)g_d_3_re, g_d_3_im, (float*)g_d_4_re, (float*)g_d_4_im,
		  input.Frequency, (float*)filter_lp, 163, dt);
	pulse_detect((float*)g_d_1_re, (float*)g_d_1_im, (float*)g_d_2_re, (float*)g_d_2_im,
		  (float*)g_d_3_re, g_d_3_im, (float*)g_d_4_re, (float*)g_d_4_im,(float*)output.output_re,
		  (float*)output.output_im,DATA_SIZE,PROCESS_PULSE_SIZE,&PID_output_gain);
}


void sampling(float*in1, float*in2, float*in3, float*in4,
		float*out1, float*out2, float*out3, float*out4){

	int i;
	int idx;
	for(i = 0; i < DATA_SIZE; i++){
		idx = i * SCALE_DOWN; // skip some sample in input for small sampling
		out1[i] = in1[idx];
		out2[i] = in2[idx];
		out3[i] = in3[idx];
		out4[i] = in4[idx];
	}

}

void demod(float*in1, float*in2, float*in3, float*in4,
		float*out1_re, float*out1_im, float*out2_re, float*out2_im,
		float*out3_re, float*out3_im, float*out4_re, float*out4_im,
		uint32_t fo, float*filter,int n_filter, struct Temp t){

	int i = 0;
	float wt = 0 , sin_wt, cos_wt;
	const float w = (2 * PI * (fo / 1000) * time) / DATA_SIZE;
	for (i = 0; i < DATA_SIZE; i++) {
		wt = ((float)i * w);
		cos_wt = arm_cos_f32(wt);
		sin_wt = arm_sin_f32(wt);
		t.w1[i] = cos_wt * in1[i];
		t.w2[i] = sin_wt * in1[i];
		t.x1[i] = cos_wt * in2[i];
		t.x2[i] = sin_wt * in2[i];
		t.y1[i] = cos_wt * in3[i];
		t.y2[i] = sin_wt * in3[i];
		t.z1[i] = cos_wt * in4[i];
		t.z2[i] = sin_wt * in4[i];
	}

	arm_conv_f32(t.w1,DATA_SIZE,filter,n_filter,t.w3);
	arm_conv_f32(t.w2,DATA_SIZE,filter,n_filter,t.w4);
	arm_conv_f32(t.x1,DATA_SIZE,filter,n_filter,t.x3);
	arm_conv_f32(t.x2,DATA_SIZE,filter,n_filter,t.x4);
	arm_conv_f32(t.y1,DATA_SIZE,filter,n_filter,t.y3);
	arm_conv_f32(t.y2,DATA_SIZE,filter,n_filter,t.y4);
	arm_conv_f32(t.z1,DATA_SIZE,filter,n_filter,t.z3);
	arm_conv_f32(t.z2,DATA_SIZE,filter,n_filter,t.z4);

	for(i = 0;i < DATA_SIZE; i++){
		out1_re[i] = t.w3[i];
		out1_im[i] = t.w4[i];
		out2_re[i] = t.x3[i];
		out2_im[i] = t.x4[i];
		out3_re[i] = t.y3[i];
		out3_im[i] = t.y4[i];
		out4_re[i] = t.z3[i];
		out4_im[i] = t.z4[i];
	}

}

int pulse_detect(float*in1_re, float*in1_im, float*in2_re, float*in2_im,
		float*in3_re, float*in3_im, float*in4_re, float*in4_im,
		float*out_re, float*out_im, int N, int Ns, float* pavr) {
	int output = 0;
	int i;
	float power[N];
	float pmin = pow(10, 10);
	float pref = 0.0;
	float h = 0.00002;
	float sum = 0.0;
	*pavr = 0.0;
	for (i = 0; i < N; i++) {
		power[i] = pow(in1_re[i], 2) + pow(in2_re[i], 2) + pow(in3_re[i], 2) + pow(in4_re[i], 2);
		sum += power[i];
		//DEBUG_PRINT("%f\n",power[i]);
	}
	pref = sum / N;
	h = h / 0.000039 * pref;
	sum = 0.0;
	for (i = 30; i < N; i++) {
		//sum += power[i];
		//DEBUG_PRINT("%d,   %f,   pref:%f\n",i,sum/(i+1),pref);
		*pavr = *pavr * ((float) (i)) / ((float) (i + 1))
				+ power[i] * 1.0 / ((float) (i + 1));
		//DEBUG_PRINT("%d,   %f,   pref:%f\n",i,power[i],pavr);
		//if(sum/(i+1)>pref)
		if (power[i] > *pavr + h) {
			output = i;
			break;
		}
	}

	//free(power);

	if (output != 0) {
		output += 5;
		for (i = output; i < output + Ns; i++) {
			out_re[i - output] = in1_re[i];
			out_re[Ns + i - output] = in2_re[i];
			out_re[2 * Ns + i - output] = in3_re[i];
			out_re[3 * Ns + i - output] = in4_re[i];
			out_im[i - output] = in1_im[i];
			out_im[Ns + i - output] = in2_im[i];
			out_im[2 * Ns + i - output] = in3_im[i];
			out_im[3 * Ns + i - output] = in4_im[i];
		}
		return (1);
	} else
		return (0);
}






