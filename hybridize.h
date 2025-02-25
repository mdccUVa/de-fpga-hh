#ifndef HYBRIDIZE_H
#define HYBRIDIZE_H

#include "systemc.h"
#include "common.h"


// hybridization in 2 steps

SC_MODULE (hybridize1) {
public:
	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<double>		A[NVARS], B[NVARS]; 

	sc_in<bool>			validIn;
		
	sc_out<double>		RES[NVARS];
	sc_out<bool>		validOut;

void calculos();


SC_CTOR(hybridize1) {
	cout << "hybridize1: " << name() << endl;

	SC_METHOD(calculos);
	sensitive << clk.pos();

  }

private:
	// latency is emulated using the following delays
	int ptR, ptW; 
	bool validez[LAT_HIB_1]; 
	double resultados[NVARS][LAT_HIB_1];
	bool estado; 
}; 


SC_MODULE(hybridize2) {
public:
	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<double>		A[NVARS], B[NVARS];

	sc_in<bool>			validIn1, validIn2;	// para detectar errores

	sc_out<double>		RES[NVARS];
	sc_out<bool>		validOut;

	void calculos();


	SC_CTOR(hybridize2) {
		cout << "hybridize2: " << name() << endl;

		SC_METHOD(calculos);
		sensitive << clk.pos();

	}

private:
	// latency is emulated using the following delays
	int ptR, ptW;
	bool validez[LAT_HIB_2]; 
	double resultados[NVARS][LAT_HIB_2];
	bool estado;
};




#endif;

