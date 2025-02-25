#include "systemc.h"
#include <sysc/tracing/sc_vcd_trace.h>

#include "common.h"
#include "values.h"


//	DIFFERENTIAL EVOLUTION FOR HODGKIN-HUXLEY 


#include "control.h"
#include "memArray.h"
#include "islandsMux.h"
#include "hybridize.h"
#include "limits.h"
#include "dependentVars.h"
#include "iterationsMux.h"
#include "calculations.h"
#include "looper.h"
#include "assessment.h"


#define an(v) (0.01*(v+50.0)/(1-exp(-(v+50.0)/10.0)))
#define am(v) (0.1*(v+35.0)/(1-exp(-(v+35.0)/10.0)))
#define ah(v) (0.07*exp(-0.05*(v+60.0)))

#define bn(v) (0.125*exp(-(v+60.0)/80.0))
#define bm(v) (4.0*exp(-0.0556*(v+60.0)))
#define bh(v) (1.0/(1.0+exp(-(0.1)*(v+30.0))))


// top module, which instantiates all the other modules and signals and connects them together

SC_MODULE(top) {
public:	
	
	sc_in < bool > clk;
	sc_in < bool > rst;


	sc_signal<sc_uint<NVARS>> maskAB, maskC; 
	sc_signal<sc_uint<addrBits>> addrA, addrB, addrC;
	sc_signal<bool> validInitMemArray, validInitLimits, validInitReferences;
	sc_signal<bool> validAddr1[nISLAS], validAddr2[nISLAS];
	sc_signal<bool> validAB[nISLAS], validC[nISLAS];
	sc_signal<bool> validInitPass[nISLAS];
	sc_signal<bool> avanzaNuevosValores[nISLAS];
	sc_signal<double> initValuePass[nISLAS];
	sc_signal<double> A[nISLAS][NVARS], B[nISLAS][NVARS], C[nISLAS][NVARS];
	sc_signal<double> Aout[NVARS], Bout[NVARS], Cout[NVARS], hyb1[NVARS], hyb2[NVARS], RES[NVARS];
	sc_signal<double> indepToCalc[NVARS], limitadas[NVARS], indepToMuxIter[NVARS];
	sc_signal<double> calculadas[NVARSdep], depToCalc[NVARSdep], depToMuxIter[NVARSdep];
	sc_signal<double> Tcalc, deltaTcalc, TToCalc, deltaTToCalc;

	sc_signal<double>		initValue;
	sc_signal<bool>			validIn, validInit, validOutAB, validOutC, validHyb1, validHyb2, validLimited, validToMuxIter, descartar;
	sc_signal<bool>			validRecircToMux, repetirCalc, avanzaGuardar, validToCalc, validCalc;
	sc_signal<sc_uint<9>>	TintCalc;

	sc_signal<double>		coste;
	sc_signal<bool>			validCoste[nISLAS];
	sc_signal<double>		nuevoCoste;


control* instControl;
memArray* instMemArray[nISLAS];
islandsMux* instMuxIslas;
hybridize1* instHibridar1;
hybridize2* instHibridar2;
limits* instLimitar; 
dependentVars* instDependientes;
iterationsMux* instMuxIteraciones;
calculations* instCalculos;
looper* instLooper;
assessment* instValorar;


SC_CTOR(top) // the module constructor
{
	char cad[256]; 

instControl = new control("instControl");
instMuxIslas = new islandsMux("instMuxIslas");
instHibridar1 = new hybridize1("instHibridar1");
instHibridar2 = new hybridize2("instHibridar2");
instLimitar = new limits("instLimitar");
instDependientes = new dependentVars("instDependientes");

instMuxIteraciones = new iterationsMux("instMuxIteraciones");
instCalculos = new calculations("instCalculos");
instLooper = new looper("instLooper");
instValorar = new assessment("instValorar");

for (int i = 0; i < nISLAS; ++i) {
	sprintf(cad, "instMemArray_%d", i); 
	instMemArray[i] = new memArray(cad);
}


instControl->clk(clk);
instControl->rst(rst);
instControl->maskAB(maskAB);
instControl->maskC(maskC);
instControl->addrA(addrA);
instControl->addrB(addrB);
instControl->addrC(addrC);
instControl->validInitMemArray(validInitMemArray);
instControl->validInitLimits(validInitLimits);
instControl->validInitReferences(validInitReferences);
instControl->initValue(initValue);
instControl->avanzaGuardar(avanzaGuardar);
for (int i = 0; i < nISLAS; ++i) {
	instControl->validAddr1[i](validAddr1[i]);
	instControl->validAddr2[i](validAddr2[i]);	
}


for (int i = 0; i < nISLAS; ++i) {
	instMemArray[i]->clk(clk);
	instMemArray[i]->rst(rst);
	instMemArray[i]->maskAB(maskAB);
	instMemArray[i]->maskC(maskC);
	instMemArray[i]->addrA(addrA);
	instMemArray[i]->addrB(addrB);
	instMemArray[i]->addrC(addrC);
	instMemArray[i]->validIn1(validAddr1[i]);
	instMemArray[i]->validIn2(validAddr2[i]);
	instMemArray[i]->validOut1(validAB[i]);
	instMemArray[i]->validOut2(validC[i]);
	for(int j=0; j<NVARS; ++j){
		instMemArray[i]->A[j](A[i][j]);
		instMemArray[i]->B[j](B[i][j]);
		instMemArray[i]->C[j](C[i][j]);
		instMemArray[i]->nuevosValores[j](indepToCalc[j]);
	}
	if(i){
		instMemArray[i]->validInit(validInitPass[i-1]);
		instMemArray[i]->initValue(initValuePass[i-1]);
		instMemArray[i]->pasaInitValue(initValuePass[i]);
		instMemArray[i]->pasaValidInit(validInitPass[i]);
	}else{
		instMemArray[i]->validInit(validInitMemArray);
		instMemArray[i]->pasaValidInit(validInitPass[i]);
		instMemArray[i]->initValue(initValue);
		instMemArray[i]->pasaInitValue(initValuePass[i]); 		
	}



	instMemArray[i]->validCoste(validCoste[i]);
	instMemArray[i]->avanzaNuevosValores(avanzaNuevosValores[i]);
	instMemArray[i]->nuevoCoste(nuevoCoste); 
}


for (int i = 0; i < nISLAS; ++i) {
	for(int j=0; j<NVARS; ++j){
		instMuxIslas->A[i][j](A[i][j]);
		instMuxIslas->B[i][j](B[i][j]);
		instMuxIslas->C[i][j](C[i][j]);
	}
	instMuxIslas->validInAB[i](validAB[i]);
	instMuxIslas->validInC[i](validC[i]);
}
for(int j=0; j<NVARS; ++j){
	instMuxIslas->Aout[j](Aout[j]);
	instMuxIslas->Bout[j](Bout[j]);
	instMuxIslas->Cout[j](Cout[j]);
}	
instMuxIslas->validOutAB(validOutAB);
instMuxIslas->validOutC(validOutC);

instHibridar1->clk(clk);
instHibridar1->rst(rst);
for(int j=0; j<NVARS; ++j){
	instHibridar1->A[j](Aout[j]);
	instHibridar1->B[j](Bout[j]);
	instHibridar1->RES[j](hyb1[j]);
}
instHibridar1->validIn(validOutAB);
instHibridar1->validOut(validHyb1);

instHibridar2->clk(clk);
instHibridar2->rst(rst);
for(int j=0; j<NVARS; ++j){
	instHibridar2->A[j](hyb1[j]);
	instHibridar2->B[j](Cout[j]);
	instHibridar2->RES[j](hyb2[j]);
}
instHibridar2->validIn1(validHyb1);
instHibridar2->validIn2(validOutC);
instHibridar2->validOut(validHyb2);


instLimitar->clk(clk);
instLimitar->rst(rst);
for(int j=0; j<NVARS; ++j){
	instLimitar->HIB[j](hyb2[j]);		
	instLimitar->RES[j](limitadas[j]);
}
instLimitar->validInit(validInitLimits);
instLimitar->initLimit(initValue);
instLimitar->validIn(validHyb2);	
instLimitar->validOut(validLimited);



instDependientes->clk(clk);
instDependientes->rst(rst);
for (int j = 0; j < NVARS; ++j) {
	instDependientes->independientes[j](limitadas[j]);
	instDependientes->indepOut[j](indepToMuxIter[j]);
}
for (int j = 0; j < NVARSdep; ++j) {
	instDependientes->depOut[j](depToMuxIter[j]);
}
instDependientes->validIn(validLimited);
instDependientes->validOut(validToMuxIter);



instMuxIteraciones->clk(clk);
instMuxIteraciones->rst(rst);
for (int j = 0; j < NVARS; ++j) {
	instMuxIteraciones->independientes[j](indepToMuxIter[j]);
	instMuxIteraciones->indepOut[j](indepToCalc[j]);
}
for (int j = 0; j < NVARSdep; ++j) {
	instMuxIteraciones->dependientesNuevas[j](depToMuxIter[j]);
	instMuxIteraciones->RECIRC[j](calculadas[j]);
	instMuxIteraciones->depOut[j](depToCalc[j]);
}

instMuxIteraciones->validIn(validToMuxIter);
instMuxIteraciones->validRecirc(validRecircToMux);
instMuxIteraciones->repetir(repetirCalc);
for (int j = 0; j < nISLAS; ++j) 
	instMuxIteraciones->avanzaNuevosValores[j](avanzaNuevosValores[j]);
instMuxIteraciones->Trecirc(Tcalc);
instMuxIteraciones->deltaTrecirc(deltaTcalc);
instMuxIteraciones->T(TToCalc);
instMuxIteraciones->deltaT(deltaTToCalc);
instMuxIteraciones->validOut(validToCalc);
instMuxIteraciones->avanzaGuardar(avanzaGuardar);


instCalculos->clk(clk);
instCalculos->rst(rst);
for (int j = 0; j < NVARS; ++j) 
	instCalculos->independientes[j](indepToCalc[j]);

for (int j = 0; j < NVARSdep; ++j) {
	instCalculos->dependientes[j](depToCalc[j]);
	instCalculos->RES[j](calculadas[j]);
}

instCalculos->validIn(validToCalc);
instCalculos->Tin(TToCalc);
instCalculos->deltaTin(deltaTToCalc);

instCalculos->Tout(Tcalc);
instCalculos->deltaTout(deltaTcalc);
instCalculos->Tinteger(TintCalc);
instCalculos->validOut(validCalc);
instCalculos->repetir(repetirCalc);





instLooper->clk(clk);
instLooper->rst(rst);
instLooper->validIn(validCalc);
instLooper->validRecirc(validRecircToMux);


instValorar->clk(clk);
instValorar->rst(rst);
for (int j = 0; j < NVARSdep; ++j) {
	instValorar->resultados[j](calculadas[j]);
}
instValorar->initValue(initValue);
instValorar->T(TintCalc);
instValorar->validIn(validCalc);
instValorar->validInit(validInitReferences);
instValorar->descartar(repetirCalc);
instValorar->coste(nuevoCoste);
instValorar->avanzaGuardar(avanzaGuardar);
for (int j = 0; j < nISLAS; ++j)
	instValorar->validCoste[j](validCoste[j]);


}
};



// main function

#define makeTrace_REMOVE_THIS_FOR_TRACING_OUTPUT

int sc_main(int nargs, char* vargs[]){

	sc_clock clk("clk", 1, SC_NS);
	sc_signal <bool> rst;

#ifdef makeTrace
 //uncomment to get a trace file that can be analyzed with GTKWAVE
	sc_trace_file* Tf;
	Tf = sc_create_vcd_trace_file("traces_DE");
	((vcd_trace_file*)Tf)->set_time_unit(0.5, SC_NS);
#endif

	top principal("top");
	principal.clk(clk);
	principal.rst(rst);

#ifdef makeTrace

	sc_trace(Tf, clk, "clk");
	sc_trace(Tf, rst, "rst");

	sc_trace(Tf, principal.instMuxIteraciones->contador, "contador");
	sc_trace(Tf, principal.instMuxIteraciones->indepMem[0], "indepMem0");
	sc_trace(Tf, principal.instMuxIteraciones->indepMem[1], "indepMem1");
	sc_trace(Tf, principal.instMuxIteraciones->indepMem[2], "indepMem2");
	sc_trace(Tf, principal.instMuxIteraciones->indepMem[3], "indepMem3");
	sc_trace(Tf, principal.avanzaNuevosValores[0], "avanzaNuevosValores0");
	sc_trace(Tf, principal.avanzaNuevosValores[1], "avanzaNuevosValores1");
	sc_trace(Tf, principal.avanzaNuevosValores[2], "avanzaNuevosValores2");
	sc_trace(Tf, principal.avanzaNuevosValores[3], "avanzaNuevosValores3");


	//	... add more signals here if wanted

#endif


	rst.write(true);
	sc_start(2, SC_NS);
	rst.write(false);

	sc_start();

	return 0;


}


