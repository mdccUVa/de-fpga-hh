#ifndef VALORAR_H
#define VALORAR_H

#include "systemc.h"
#include "common.h"
#include "mem64dp.h"


// normalizes de cost, dividing by the number of points
SC_MODULE (costDivider) {
public:

	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in <double>		coste;
	sc_in <short>		muestras;
	sc_in<bool>			validIn; 

	sc_out<double>		costePorMuestra; 
	sc_out<bool>		validOut; 

	void calculos();

	SC_CTOR(costDivider) {
		cout << "costDivider: " << name() << endl;

		SC_METHOD(calculos);
		sensitive << clk.pos();
	}

private: 
	// latency of the fp divider is emulated using the following delays
	int ptR, ptW;
	bool validez[LAT_DIV];
	double resultados[LAT_DIV];
};


// implements cost evalution as the sum of squares of differences
SC_MODULE (minSq) {
public:

	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in <double>		dependientes[NVARSdepCost];
	sc_in <double>		referencias[NVARSdepCost];
	sc_in <double>		costeActual; 
	sc_in<bool>			validIn, keepIn; 

	sc_out<double>		costeNuevo; 
	sc_out<bool>		validOut, keepOut; 

	void calculos();

	SC_CTOR(minSq) {
		cout << "minSq: " << name() << endl;

		SC_METHOD(calculos);
		sensitive << clk.pos();
	}

private: 
	// latency is emulated using the following delays
	int ptR, ptW;
	bool validez[LAT_MIN_SQ];
	bool keep[LAT_MIN_SQ];	
	double resultados[LAT_MIN_SQ];
};


// main module, that collects incoming results, compares then with the stored references, and issue the results to memArray for avaliation. 
SC_MODULE (assessment) {
public:

	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in <double>		resultados[NVARSdep];
	sc_in <double>		initValue;
	sc_in <sc_uint<9>>	T;		// up to 512 points...
	sc_in<bool>			validIn, validInit, descartar; 
	sc_out<double>		coste; 
	sc_out<bool>		validCoste[nISLAS], avanzaGuardar;


	void comb();
	void regs(); 


SC_CTOR(assessment) {
	cout << "assessment: " << name() << endl;

	char cad[256];

	instDiv = new costDivider("instDiv");
	instMinSq = new minSq("instMinSq");

	instDiv->clk(clk);
	instDiv->rst(rst);
	instDiv->coste(costeActual);
	instDiv->muestras(numRes);
	instDiv->validIn(validProcCoste);
	instDiv->costePorMuestra(costeSample);
	instDiv->validOut(validCosteSample);

	instMinSq->clk(clk);
	instMinSq->rst(rst);
	instMinSq->validIn(validMinSq);
	instMinSq->keepIn(noDescartarMinSq);
	instMinSq->costeActual(costeActual);
	instMinSq->costeNuevo(newCoste);
	instMinSq->validOut(validNewCoste);
	instMinSq->keepOut(noDescartarCoste);
	for (int i = 0; i < NVARSdepCost; ++i) {
		instMinSq->dependientes[i](dependientesReg[i]); 
		instMinSq->referencias[i](refs[i]);
	}

	nResultados = new mem8x512("nResultados");
	nResultados->clk(clk);
	nResultados->we(weNres);
	nResultados->addrRead(contLect);
	nResultados->addrWrite(addrNres);
	nResultados->dataIn(wrNres);
	nResultados->dataOut(nResActual);

	costeAcum = new mem64x512("costeAcum");
	costeAcum->clk(clk);
	costeAcum->we(weCoste);
	costeAcum->addrRead(contLect);
	costeAcum->addrWrite(addrCoste);
	costeAcum->dataIn(wrCoste);
	costeAcum->dataOut(costeActual);

	for (int i = 0; i < NVARSdepCost; ++i) {
		sprintf(cad, "referencias_%d", i);
		referencias[i] = new mem64x512(cad);
		referencias[i]->clk(clk);
		referencias[i]->we(weMultiBit[i]);
		referencias[i]->addrRead(T); 
		referencias[i]->addrWrite(contRef2);
		referencias[i]->dataIn(regInitValue);
		referencias[i]->dataOut(refs[i]);
	}

	fire.write(0);

	SC_METHOD(regs);
	sensitive << clk.pos();
	SC_METHOD(comb);
	sensitive << fire << validInit << validIn << descartar; 

  }

private:

	sc_signal<int> estadoSig; 
	int estado, nuevoEstado;
	int contRef1, contRes, contLectPrev, contadorLecturasT, contadorEscriturasT;
	int NcontRef1, NcontRef2, NcontRes, NcontLect, NcontadorLecturasT, NcontadorEscriturasT;

	bool regValidIn, regValidInit, regDescartar;
	int regT;
	sc_uint<NVARSdep> weShift; 

	sc_signal<double> refs[NVARSdepCost], vals[NVARSdepCost];
	sc_signal<short> numRes;

	sc_signal<bool> weNres, weCoste;
	sc_signal<bool> weMultiBit[NVARSdepCost];
	sc_signal<sc_uint<9>> contLect, addrNres, addrCoste, contRef2;
	sc_signal<sc_uint<8>> wrNres, nResActual;

	sc_signal<bool> validMinSq, validNewCoste, validProcCoste, validCosteSample;
	sc_signal<bool>	noDescartarMinSq, noDescartarCoste;

	sc_signal<double> wrCoste, costeActual, newCoste, costeSample, regInitValue;
	sc_signal<double> dependientesReg[NVARSdepCost];

	bool regUse; 

	sc_uint<nISLAS> islaIdx, NislaIdx; 


	costDivider* instDiv;
	minSq* instMinSq; 
	mem8x512* nResultados;
	mem64x512* costeAcum; 
	mem64x512* referencias[NVARSdepCost];

	sc_signal<bool> fire; 

}; 




#endif;