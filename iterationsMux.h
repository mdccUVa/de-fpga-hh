#ifndef ITERATIONSMUX_H
#define ITERATIONSMUX_H

#include "systemc.h"
#include "common.h"
#include "mem64dp.h"
#include "math.h"


// this mux lets the first set of data go to calculations
// then, results from calculations are recirculated

// THE MUX ALSO COUNTS THE NUMBER OF ITERATIONS AND BLOCKS RECIRCULATING RESULTS SENT BY THE 
// CALCULATIONS MODULE. IT ONLY RESUMES NORMAL OPERATION AFTER GETTING A FRESH VALUE FROM 
// A NEW SET OF SOLUTIONS
// THIS SOLUTIONS SEEMS SIMPLER THAN BLOCKING RECIRCULATION AT THE CALCULATIONS MODULE, 
// BASICALLY BECAUSE OF THE LARGE PIPELINE SIMULATION THAT HAPPENS THERE. 

int tiempo; 

SC_MODULE(iterationsMux) {
public:

	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<double>		independientes[NVARS];
	sc_in<double>		dependientesNuevas[NVARSdep];
	sc_in<double>		RECIRC[NVARSdep];

	sc_in<bool>			validIn, validRecirc, repetir, avanzaGuardar, avanzaNuevosValores[nISLAS];
	sc_in<double>		Trecirc, deltaTrecirc;

	sc_out<double>		indepOut[NVARS];
	sc_out<double>		depOut[NVARSdep];
	sc_out<double>		T, deltaT;
	sc_out <bool>		validOut;




	// the FSM obtains new values for state and counters 
	void fsm() {

		newState = state;
		newCounter = counter;
		newContador = contador.read();
		weIndep.write(false);
		weDep.write(false);
		validOut.write(false);
		T.write(Trecirc);
		deltaT.write(deltaTrecirc);

		address = contador;

		switch (state) {
		case 0: // wait for the first valid incoming solution
			T.write(0.0);
			deltaT.write(deltaTinicial);
			if (validIn.read()) {

				for (int i = 0; i < NVARS; ++i)
					indepOut[i].write(independientes[i]);

				for (int i = 0; i < NVARSdep; ++i) {
					depOut[i].write(dependientesNuevas[i]);
					nuevasDep[i].write(dependientesNuevas[i]);
				}
				weIndep.write(true);
				weDep.write(true);
				validOut.write(true);
				newState = 1;
				newContador = contador.read() + 1;
				newCounter = counter + 1;				
			}
			break;
		case 1: // new iteration, uninterrupted stream of inputs (validIn is not checked)

			for (int i = 0; i < NVARS; ++i) 
				indepOut[i].write(independientes[i]);

			for (int i = 0; i < NVARSdep; ++i) {
				depOut[i].write(dependientesNuevas[i]);
				nuevasDep[i].write(dependientesNuevas[i]);
			}

			T.write(0);
			deltaT.write(deltaTinicial);
			for (int i = 0; i < NVARS; ++i)
				indepOut[i].write(independientes[i]);
			for (int i = 0; i < NVARSdep; ++i) {
				depOut[i].write(dependientesNuevas[i]);
				nuevasDep[i].write(dependientesNuevas[i]);
			}
			weIndep.write(true);
			weDep.write(true);
			validOut.write(true);
			newCounter = counter + 1;
			if (contador.read() == (NSOL * nISLAS - 1)) {
				newContador = 0;
				newState = 2;
			}
			else
				newContador = contador.read() + 1;
			break;
		case 2: // next iterations
			if (validRecirc.read()) { // recirculated data
				if (repetir.read()) { // recirculated, but iteration must repeat
					//not necessary for RK4
					cout << "UNEXPECTED: non-convergence signal arrived to iterations mux" << endl;
					sc_stop();
				}
				else {	// valid data recirculating
					for (int i = 0; i < NVARS; ++i)
						indepOut[i].write(indepMem[i]);
					for (int i = 0; i < NVARSdep; ++i) {
						depOut[i].write(RECIRC[i]);
						nuevasDep[i].write(RECIRC[i]);	// redundant for RK4, but not removed. 
					}
					weDep.write(true);
					validOut.write(true);
				}
				if (contador.read() == (NSOL * nISLAS - 1))
					newContador = 0;
				else
					newContador = contador.read() + 1;
				if (counter == (nPOINTS * nISLAS * NSOL - 1)) {
					newCounter = 0;
					newState = 3;
				}
				else
					newCounter = counter + 1;
			}
			address = newContador;	
			break;
		default: // deliver hybridized solutions
			bool avanza = false; 
			for (int i = 0; i < NVARS; ++i) 
				indepOut[i].write(indepMem[i]);
			
			avanza = avanzaGuardar.read(); 
			if (avanza) {
				if (contador.read() == (NSOL * nISLAS - 1)) {
					newContador = 0;
					newState = 0;
				}
				else
					newContador = contador.read() + 1;
			}
		};
	}

	// updates registers

	void regs() {

		if (rst.read()) {
			state = false; 
			counter = 0; 
			contador.write(0);
		}
		else {
	
			counter = newCounter; 
			state = newState; 
			contador.write(newContador);
		}
		fire.write( !fire.read() ); 

	}


SC_CTOR(iterationsMux) {
	cout << "iterationsMux: " << name() << endl;

	fire.write(false); 
	char cad[256]; 


	// memories for storing variables

	for (int i = 0; i < NVARS; ++i) {
		sprintf(cad, "memIndep_%d", i);
		memIndep[i] = new mem64sp(cad);
		memIndep[i]->clk(clk);
		memIndep[i]->addr(address); 
		memIndep[i]->dataIn(independientes[i]);
		memIndep[i]->we(weIndep);
		memIndep[i]->dataOut(indepMem[i]);
	}

	for (int i = 0; i < NVARSdep; ++i) {
		sprintf(cad, "memDep_%d", i);
		memDep[i] = new mem64sp(cad);
		memDep[i]->clk(clk);
		memDep[i]->addr(address); 
		memDep[i]->dataIn(nuevasDep[i]);
		memDep[i]->we(weDep);
		memDep[i]->dataOut(depMem[i]);
	}


	SC_METHOD(regs);
	sensitive << clk.pos();
	
	SC_METHOD(fsm);
	sensitive << fire << validIn << validRecirc << repetir << Trecirc << deltaTrecirc << avanzaGuardar;
	for (int j = 0; j < nISLAS; ++j)
		sensitive << avanzaNuevosValores[j];
	for (int i = 0; i < NVARS; ++i)
		sensitive << independientes[i] << indepMem[i];
	for (int i = 0; i < NVARSdep; ++i)
		sensitive << RECIRC[i] << depMem[i] << dependientesNuevas[i];


  }

//private:
	mem64sp *memIndep[NVARS];		// NSOL*nISLAS
	mem64sp *memDep[NVARSdep];		// NSOL*nISLAS

	sc_uint<2> state, newState; 
	int counter, newCounter;
	sc_signal< sc_uint<addrBits>> contador, address;
	sc_uint<addrBits> newContador;

	sc_signal<double> indepMem[NVARS], depMem[NVARSdep], nuevasDep[NVARSdep];	
	sc_signal<bool> weIndep, weDep;
	sc_signal<bool> fire; 
	double regRECIRC[NVARSdep];
	bool regValidRecircNoRepetir; 
	double regT, regDelta; 

}; 

#endif;
