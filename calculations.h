#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include "systemc.h"
#include "common.h"

// Hodgkin-Huxley calculations are implemented sequentially, and the output is delayed according to the latency reported by Vivado. 
SC_MODULE (calculations) {
public:

	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<double>		independientes[NVARS];
	sc_in<double>		dependientes[NVARSdep];
	sc_in<bool>			validIn;

	sc_in<double>		Tin, deltaTin; 

	sc_out<double>		RES[NVARSdep];
	sc_out<double>		Tout, deltaTout;
	sc_out<sc_uint<9>>	Tinteger;
	sc_out<bool>		validOut, repetir; 



void calcular();


SC_CTOR(calculations) {
	cout << "calculations: " << name() << endl;

	SC_METHOD(calcular);
	sensitive << clk.pos();

  }

private:

	int contador; 
	// latency is emulated using the following delays
	int ptR, ptW;
	bool validez[LAT_PIPE];
	bool repe[LAT_PIPE];
	double resDEP[NVARSdep][LAT_PIPE];
	double T[LAT_PIPE];
	int Tint[LAT_PIPE];
	double deltaT[LAT_PIPE];

	double HH_calculo(double V, double* dn, double* dm, double* dh, double n, double m, double h,
		double an1, double an3I, double am1, double am3I, double ah1, double ah3N,
		double bn1, double bn3I, double bm1, double bm3N, double bh3N);

}; 

#endif;