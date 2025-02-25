#ifndef ISLANDSMUX_H
#define ISLANDSMUX_H

#include "systemc.h"
#include "common.h"


// this multiplexer implements a round-robin scheduling for different islands sending independent variables to the cross-over stages, calculations and cost assessment

SC_MODULE (islandsMux) {
public:

	sc_in<double>		A[nISLAS][NVARS];
	sc_in<double>		B[nISLAS][NVARS];
	sc_in<double>		C[nISLAS][NVARS];

	sc_in<bool>	validInAB[nISLAS], validInC[nISLAS]; 

	sc_out<double>		Aout[NVARS];
	sc_out<double>		Bout[NVARS];
	sc_out<double>		Cout[NVARS];

	sc_out <bool>		validOutAB, validOutC;


	void muxAB() {
		int i, j; 

		validOutAB.write(false);

		for (i = 0; i < nISLAS; ++i) {
			if (validInAB[i].read()) {
				validOutAB.write(true); 
				for (j = 0; j < NVARS; ++j) {
					Aout[j].write( A[i][j].read() );
					Bout[j].write( B[i][j].read() );
				}
			}
		}
	}

	void muxC() {
		int i, j;

		validOutC.write(false);

		for (i = 0; i < nISLAS; ++i) {
			if (validInC[i].read()) {
				validOutC.write(true);
				for (j = 0; j < NVARS; ++j)
					Cout[j].write(C[i][j].read());
			}
		}
	}


SC_CTOR(islandsMux) {
	cout << "islandsMux: " << name() << endl;

	SC_METHOD(muxAB);
	for (int i = 0; i < nISLAS; ++i){			
		sensitive << validInAB[i];
		for (int j = 0; j < NVARS; ++j) 
			sensitive << A[i][j] << B[i][j];
	}

	SC_METHOD(muxC);
	for (int i = 0; i < nISLAS; ++i){			
		sensitive << validInC[i];
		for (int j = 0; j < NVARS; ++j)
			sensitive << C[i][j];
	}

}

}; 

#endif;