#ifndef LIMITS_H
#define LIMITS_H

#include "systemc.h"
#include "common.h"


// this module stores the limits (bounds) for hybridized independent variables
// bounding its the last stage in cross-over

SC_MODULE (limitsMemory) {
public:
	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<double>		initLimit; 
	sc_in<bool>			validInit;
		
	sc_out<double>		maxOut[NVARS], minOut[NVARS];

void memoria();


SC_CTOR(limitsMemory) {
	cout << "limitsMemory: " << name() << endl;

	SC_METHOD(memoria);
	sensitive << clk.pos();

  }

private:
	double max[NVARS], min[NVARS]; 
}; 



// this modules reads incoming variable values and corrects them if they are out of limits
SC_MODULE (limitsCalculations) {
public:
	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<double>		HIB[NVARS];
	sc_in<bool>			validIn;
		
	sc_in<double>		maxIn[NVARS], minIn[NVARS]; 
	sc_out<double>		RES[NVARS];
	sc_out<bool>		validOut;

void calculos();


SC_CTOR(limitsCalculations) {
	cout << "limitsCalculations: " << name() << endl;

	SC_METHOD(calculos);
	sensitive << clk.pos();

  }

private:	
	// latency is emulated using the following delays
	int ptR, ptW; 
	bool validez[LAT_LIM]; 
	double resultados[NVARS][LAT_LIM];
}; 



// this module is, essentially, a wrapping for the 2 previous ones
SC_MODULE(limits) {		
public:
	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<double>		HIB[NVARS], initLimit;
	sc_in<bool>			validIn, validInit;

	sc_out<double>		RES[NVARS];
	sc_out<bool>		validOut;


	SC_CTOR(limits) {
		cout << "limits: " << name() << endl;

		instLimitarMemoria = new limitsMemory("instLimitarMemoria");
		instLimitarCalculos = new limitsCalculations("instLimitarCalculos");

		instLimitarMemoria->clk(clk);
		instLimitarMemoria->rst(rst);
		instLimitarMemoria->initLimit(initLimit);
		instLimitarMemoria->validInit(validInit);
		for (int i = 0; i < NVARS; ++i) {
			instLimitarMemoria->maxOut[i](max[i]);
			instLimitarMemoria->minOut[i](min[i]);
		}

		instLimitarCalculos->clk(clk);
		instLimitarCalculos->rst(rst);
		instLimitarCalculos->validIn(validIn);

		for (int i = 0; i < NVARS; ++i) {
			instLimitarCalculos->HIB[i](HIB[i]);
			instLimitarCalculos->maxIn[i](max[i]);
			instLimitarCalculos->minIn[i](min[i]);
			instLimitarCalculos->RES[i](RES[i]);
		}
		instLimitarCalculos->validOut(validOut);

	}

private:

	limitsMemory* instLimitarMemoria;
	limitsCalculations* instLimitarCalculos;

	sc_signal<double> max[NVARS], min[NVARS];  
};


#endif;