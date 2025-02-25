#ifndef MEMARRAY_H
#define MEMARRAY_H

#include "systemc.h"
#include "common.h"
#include "mem64dp.h"


// FP comparasion 
SC_MODULE(comparadorPF) {
public:
	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<double>		A, B;
	sc_in<bool>			validIn;
	sc_out<bool>		res, done;

	void calculos();


	SC_CTOR(comparadorPF) {
		cout << "comparadorPF: " << name() << endl;

		SC_METHOD(calculos);
		sensitive << clk.pos();

	}

private:
	// latency is emulated using the following delays
	int ptR, ptW;
	bool validez[LAT_COMP];
	bool resultados[LAT_COMP];
	bool estado;
};



// main module for memory Array
SC_MODULE (memArray) {
public:
	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<sc_uint<NVARS>> maskAB, maskC;
	sc_in<sc_uint<addrBits>> addrA, addrB, addrC;
	sc_in<bool> validIn1, validIn2, validInit;
	
	sc_out<bool> validOut1, validOut2;
	sc_out<double> A[NVARS], B[NVARS], C[NVARS];
	sc_out<double> pasaInitValue; 

	sc_in<bool> validCoste;
	sc_out<bool> avanzaNuevosValores, pasaValidInit; 
	sc_in <double> nuevosValores[NVARS];
	sc_in <double> nuevoCoste, initValue;



void regs();
void muxesMems();
void lecturas();
void comb();


SC_CTOR(memArray) {
	  
	cout << "memArray: " << name() << endl;
	char cad[256];

	COMP = new comparadorPF("COMP"); 
	COMP->clk(clk);
	COMP->rst(rst);
	COMP->A(nuevoCoste);
	COMP->B(costeParaEvaluar);
	COMP->validIn(validCoste);
	COMP->res(mejora);
	COMP->done(doneEvalCost);

	coste = new mem64dp("coste");
	coste->clk(clk);
	coste->addrA(addrCoste);
	coste->addrB(addrCoste);
	coste->dataIn(costeIn);
	coste->we(weMultiBit[NVARS]);
	coste->dataA(costeParaEvaluar); 
	coste->dataB(sinkCoste);

	for (int i = 0; i < NVARS; ++i) {
		sprintf(cad, "mem_%d", i);
		mem[i] = new mem64dp(cad);
		mem[i]->clk(clk);
		mem[i]->addrA(addrA2);
		mem[i]->addrB(addrB2);
		mem[i]->dataIn(din[i]);
		mem[i]->we(weMultiBit[i]);
		mem[i]->dataA(dataA[i]);
		mem[i]->dataB(dataB[i]);
	}

	FIRE.write(0);

	SC_METHOD(regs);
	sensitive << clk.pos();

	SC_METHOD(muxesMems);
	sensitive << FIRE << addrA << validInit << initValue;

	SC_METHOD(lecturas);
	sensitive << FIRE << maskABreg << maskCreg << valReg1 << valReg2;
	for(int i=0; i<NVARS; ++i)
		sensitive << dataA[i] << dataB[i];

	SC_METHOD(comb);
	sensitive << FIRE << doneEvalCost << validInit << validCoste << mejora;	

  }

//private:

	
	mem64dp *mem[NVARS];
	mem64dp *coste;		
	double regCoste; 
	int cont1, cont2, prevCont2; 
	int Ncont1, Ncont2;
	bool iniciado, Niniciado;

	comparadorPF* COMP; 
	sc_signal<bool> doneEvalCost, mejora;	
	sc_signal<double> costeParaEvaluar;
	sc_signal<bool> weMultiBit[NVARS + 1];
	sc_signal<bool> valReg1, valReg2; 

	sc_signal<double> costeIn, costeOut, sinkCoste; 
	sc_signal<double> din[NVARS], dataA[NVARS], dataB[NVARS];
	sc_signal<sc_uint<addrBits>> addrCoste, addrA2, addrB2;
	sc_signal < sc_uint<NVARS>> maskABreg, maskCreg;
	sc_uint<NVARS + 1 > weShift;

	sc_signal<bool> FIRE;
}; 

#endif;