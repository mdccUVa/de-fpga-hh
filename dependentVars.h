#ifndef DEPENDENTVARS_H
#define DEPENDENTVARS_H

#include "systemc.h"
#include "common.h"
#include "math.h"


extern int CICLO;

#define idxAN1 0
#define idxAM1 1
#define idxAH1 2
#define idxAN3I 3
#define idxAM3I 4
#define idxAH3NEG 5

#define idxBN1 6
#define idxBM1 7
#define idxBN3I 8
#define idxBM3NEG 9
#define idxBH3NEG 10


double HH(double V, double* dn, double* dm, double* dh, double n, double m, double h,
	double an1, double an3I, double am1, double am3I, double ah1, double ah3N,
	double bn1, double bn3I, double bm1, double bm3N, double bh3N) {

	const double ENa = 55.17;	// mv Na reversal potential
	const double EK = -72.14;	// mv K reversal potential
	const double El = -49.42;	// mv Leakage reversal potential
	const double gbarNa = 1.2;	// mS/cm^2 Na conductance
	const double gbarK = 0.36;	// mS/cm^2 K conductance
	const double gbarl = 0.003; // mS/cm^2 Leakage conductance
	const double I = 0.1;		// Applied Current
	const double CmI = 1.0 / 0.01;	// Membrane Capacitance

	const double an2 = 50.0;
	const double am2 = 35.0;
	const double ah2 = 60.0;
	const double bn2 = 50.0;
	const double bm2 = 50.0;
	const double bh2 = 50.0;
	const double bh1 = 1.0;

	double dydt, gNa, gK, gl, INa, IK, Il;

	gNa = gbarNa * (m * m * m) * h;
	gK = gbarK * (n * n * n * n);
	gl = gbarl;
	INa = gNa * (V - ENa);
	IK = gK * (V - EK);
	Il = gl * (V - El);

	dydt = CmI * (I - (INa + IK + Il));

	double an = (an1 * (V + an2) / (1 - exp(-(V + an2) * an3I)));
	double am = (am1 * (V + am2) / (1 - exp(-(V + am2) * am3I)));
	double ah = (ah1 * exp(ah3N * (V + ah2)));

	double bn = (bn1 * exp(-(V + bn2) * bn3I));
	double bm = (bm1 * exp(bm3N * (V + bm2)));
	double bh = (1.0 / (bh1 + exp(bm3N * (V + bh2))));

	*dn = an * (1.0 - n) - bn * n;
	*dm = am * (1.0 - m) - bm * m;
	*dh = ah * (1.0 - h) - bh * h;

	return dydt;
}


// this module computes the first set of dependent variables for the first iteration of calculations

SC_MODULE(dependentVars) {
public:

	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<double>		independientes[NVARS];
	sc_in<bool>			validIn;

	sc_out<double>		indepOut[NVARS];
	sc_out<double>		depOut[NVARSdep];
	sc_out <bool>		validOut;

	void calculos() {

		if (rst.read()) {
			ptR = 1;
			ptW = 0;
			for (int j = 0; j < LAT_DEP; ++j) {
				validez[j] = false;
				for (int i = 0; i < NVARS; ++i)
					delayIndependientes[i][j] = 0.0;
			}
		}
		else {
			validOut.write(validez[ptR]);
			for (int i = 0; i < NVARS; ++i)
				indepOut[i].write(delayIndependientes[i][ptR]);
			for (int i = 0; i < NVARSdep; ++i)
				depOut[i].write(delayDependientes[i][ptR]);

			ptR = (ptR + 1) % LAT_DEP;

			validez[ptW] = validIn.read();	

			if (validIn.read()) {	
				//printf("In: ");
				for (int i = 0; i < NVARS; ++i) {
					delayIndependientes[i][ptW] = independientes[i].read();
				}

				const double an2 = 50.0;
				const double am2 = 35.0;
				const double ah2 = 60.0;
				const double bn2 = 50.0;
				const double bm2 = 50.0;
				const double bh2 = 50.0;
				const double bh1 = 1.0;
				double V = -an2 + 0.0001;	//independientes[idxVinicial];


				double an1 = delayIndependientes[0][ptW];
				double an3I = delayIndependientes[1][ptW];
				double am1 = delayIndependientes[2][ptW];
				double am3I = delayIndependientes[3][ptW];
				double ah1 = delayIndependientes[4][ptW];
				double ah3N = delayIndependientes[5][ptW];

				double bn1 = delayIndependientes[6][ptW];
				double bn3I = delayIndependientes[7][ptW];
				double bm1 = delayIndependientes[8][ptW];
				double bm3N = delayIndependientes[9][ptW];
				double bh3N = delayIndependientes[10][ptW];
				
				double an = (an1 * (V + an2) / (1 - exp(-(V + an2) * an3I)));
				double am = (am1 * (V + am2) / (1 - exp(-(V + am2) * am3I)));
				double ah = (ah1 * exp(ah3N * (V + ah2)));

				double bn = (bn1 * exp(-(V + bn2) * bn3I));
				double bm = (bm1 * exp(bm3N * (V + bm2)));
				double bh = (1.0 / (bh1 + exp(bh3N * (V + bh2))));

				delayDependientes[0][ptW] = V;
				delayDependientes[1][ptW] = an / (an + bn);	// n
				delayDependientes[2][ptW] = am / (am + bm);	// m
				delayDependientes[3][ptW] = ah / (ah + bh);	// h

			}
			ptW = (ptW + 1) % LAT_DEP;
		}

	}


	SC_CTOR(dependentVars) {
		cout << "dependentVars: " << name() << endl;

		SC_METHOD(calculos);
		sensitive << clk.pos();
	}

	private:
		// latency is emulated using the following delays
	int ptR, ptW;
	bool validez[LAT_DEP];
	double delayIndependientes[NVARS][LAT_DEP];
	double delayDependientes[NVARS][LAT_DEP];
	bool estado;

};

#endif;
