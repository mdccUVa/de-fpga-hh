#include"memArray.h"

void memArray::muxesMems() {

	for (int i = 0; i < NVARS; ++i) {
		if (validInit.read())
			din[i].write(initValue.read());
		else
			din[i].write(nuevosValores[i].read());
	}

	if (validInit.read()) {
		addrCoste.write(cont2);
		addrA2.write(cont2);
		addrB2.write(addrB.read());
		costeIn.write(initValue);
	}
	else if( doneEvalCost && mejora) {
		addrCoste.write(prevCont2);
		addrA2.write(prevCont2);
		addrB2.write(addrB.read());
		costeIn.write(regCoste);
	}
	else {
		addrCoste.write(cont2);
		if (validIn1.read()) {
			addrA2.write(addrA.read());
			addrB2.write(addrB.read());
		}
		else {
			addrA2.write(cont2);
			addrB2.write(addrC.read());	
		}
		costeIn.write(initValue.read());		
	}

}


void memArray::lecturas() {

	for (int i = 0; i < NVARS; ++i) {
		if (valReg1) { // AB	
			if (maskABreg.read().bit(i)) {
				A[i].write(dataA[i].read());
				B[i].write(dataB[i].read());
			}
			else {
				A[i].write(0);
				B[i].write(0);
			}
		}
		else {	// C
			if (maskCreg.read().bit(i)) 
				C[i].write(dataB[i].read());
			else 
				C[i].write(dataA[i].read());
		}
	}
}


void memArray::comb() {

	avanzaNuevosValores.write(doneEvalCost.read()); 

	for (int i = 0; i <= NVARS; ++i) {
		if (doneEvalCost && mejora)
			weMultiBit[i].write(1);
		else if ( !iniciado && validInit.read())
			weMultiBit[i].write(weShift.bit(i));
		else
			weMultiBit[i].write(0);
	}

	Niniciado = iniciado; 
	Ncont1 = cont1;
	Ncont2 = cont2;

	if (validInit.read()) {
		if (!iniciado) {
			if (cont1 == NVARS) {
				Ncont1 = 0;
				if (cont2 == (NSOL - 1)) {
					Ncont2 = 0;
					Niniciado = true;
				}
				else
					Ncont2 = cont2 + 1;
			}
			else
				Ncont1 = cont1 + 1;
		}
	}
	else {
		if (validCoste.read() || validIn2.read()) {	
			if (cont2 == (NSOL - 1)) {
				Ncont2 = 0;
				Niniciado = true;	
			}
			else
				Ncont2 = cont2 + 1;
		}
	}

}


void memArray::regs() {
	
	sc_uint<NVARS+1> tmp;
	
	if (rst.read()) {
		cont1 = cont2 = 0; 
		iniciado = false; 
		weShift = 1;
		maskABreg = maskCreg = 0;
	}
	else {

		maskABreg = maskAB.read();
		maskCreg = maskC.read();
		if (validCoste)
			regCoste = nuevoCoste.read(); 

		valReg1 = validIn1.read();
		valReg2 = validIn2.read();	

		validOut1.write(validIn1.read());
		validOut2.write(validIn2.read());

		pasaValidInit.write(false);

		if (validInit.read()) {
			tmp.bit(0) = weShift.bit(NVARS);
			tmp(NVARS, 1) = weShift(NVARS-1, 0);
			weShift = tmp; 

			if (iniciado) {	
				pasaValidInit.write(true);
				pasaInitValue.write(initValue.read());
			}
		}else{
			if (validCoste.read()) {
				regCoste = nuevoCoste.read();
				prevCont2 = cont2; 
			}
		}

		cont1 = Ncont1;
		cont2 = Ncont2;

		iniciado = Niniciado;

	}
	FIRE.write(!FIRE.read());

}





void comparadorPF::calculos() {

	if (rst.read()) {
		ptR = 1;
		ptW = 0;
		for (int j = 0; j < LAT_COMP; ++j) {
			validez[j] = resultados[j] = false;
		}
	}
	else {
		done.write(validez[ptR]);
		res.write(resultados[ptR]);

		ptR = (ptR + 1) % LAT_COMP;

		validez[ptW] = validIn.read();	

		if (validIn.read()) {
			resultados[ptW] = A.read() < B.read();			

			// we print any improvement
			// this can be enhanced so that it reports which solution has been improved, and the island it belongs to. 

			if (resultados[ptW])
				printf("comp %3.6lf  < %3.6lf     %d\n", A.read(), B.read(), resultados[ptW]); // reports a better solution was found.
			ptR = ptR; 
		}
		ptW = (ptW + 1) % LAT_COMP;
	}

}

