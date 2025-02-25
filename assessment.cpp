#include "assessment.h"

extern int CICLO; 

void assessment::comb() {

	NcontadorLecturasT = contadorLecturasT;
	NcontadorEscriturasT = contadorEscriturasT;
	NcontRef1 = contRef1;
	NcontRef2 = contRef2.read();
	NcontRes = contRes;
	NcontLect = contLect.read();
	nuevoEstado = estado;
	NislaIdx = islaIdx;

	for (int i = 0; i < NVARSdepCost; ++i) {
		if (regValidInit)
			weMultiBit[i].write(weShift.bit(i));
		else
			weMultiBit[i].write(0);
	}

	weNres.write(false); weCoste.write(false);

	coste.write(costeSample);
	if (validCosteSample.read()) {
		avanzaGuardar.write(true);
		for (int i = 0; i < nISLAS; ++i)
			validCoste[i]->write(islaIdx.bit(i)); 
		NislaIdx(nISLAS - 1, 1) = islaIdx(nISLAS - 2, 0);
		NislaIdx.bit(0) = islaIdx.bit(nISLAS - 1);	// bit mask rotation 
	}
	else {
		avanzaGuardar.write(false);
		for (int i = 0; i < nISLAS; ++i)
			validCoste[i]->write(false);
	}

	switch (estado) {
	case 0: // starting up
		if (validInit.read()) 
			nuevoEstado = 1;
		break;
	case 1: // reset
		weNres = true;  addrNres = contRes; wrNres = 0;		
		weCoste = true; addrCoste = contRes; wrCoste = 0.0;	

		if (contRes == (nISLAS * NSOL - 1)) {
			nuevoEstado = 2;
			NcontRes = 0;
		}
		else
			NcontRes = contRes + 1;
		break;

	case 2: // normal operation, loading references, no reset
		if (contadorLecturasT == (nISLAS * NSOL * nPOINTS - 1)) 
			nuevoEstado = 3;
		break;
	case 3: // reading costs, no reset

		if (contadorEscriturasT == (nISLAS * NSOL * nPOINTS - 1)) 
			nuevoEstado = 4;
		break;
	case 4: // reading costs, and reseting stored values (calculated costs and number of received results)
		weNres.write(1);	addrNres.write(contRes);	wrNres.write(0);
		weCoste.write(1);	addrCoste.write(contRes);	wrCoste.write(0.0);
		if (contLect.read() == (nISLAS * NSOL - 1)) {
			nuevoEstado = 5;
//			cout << "ITER" << endl; 
		}
		break;
	case 5: // just for the last result
		weNres.write(1);	addrNres.write(contRes);	wrNres.write(0);
		weCoste.write(1);	addrCoste.write(contRes);	wrCoste.write(0.0);
		nuevoEstado = 6;
		break;
	case 6:
	default: // WAITs, only reseting stored values (calculated costs and number of received results)
		weNres.write(1);	addrNres.write(contRes);	wrNres.write(0);
		weCoste.write(1);	addrCoste.write(contRes);	wrCoste.write(0.0);
		if (contRes == (nISLAS * NSOL - 1)) {	
			nuevoEstado = 2; 
		}
	};


	if (regUse) {
		weNres.write(1);	addrNres.write(contLectPrev);	wrNres.write(nResActual.read() + 1);
	}

	if (regValidInit) {	
		if (contRef1 == (NVARSdepCost - 1)) {
			NcontRef1 = 0;
			if (contRef2.read() == (nPOINTS - 1)) {
				NcontRef2 = 0;
			}
			else
				NcontRef2 = contRef2.read() + 1;
		}
		else
			NcontRef1 = contRef1 + 1;
	}

	if (regValidIn) {
		if (contadorLecturasT == (nISLAS * NSOL * nPOINTS - 1))
			NcontadorLecturasT = 0;
		else
			NcontadorLecturasT = contadorLecturasT + 1;
	}

	if (validNewCoste.read()) {	
		if (contadorEscriturasT == (nISLAS * NSOL * nPOINTS - 1))
			NcontadorEscriturasT = 0;
		else
			NcontadorEscriturasT = contadorEscriturasT + 1;

		weCoste.write(noDescartarCoste.read());	 
		addrCoste.write(contRes);	wrCoste.write(newCoste.read());
	}

	if ((estado == 3) || (estado == 4) || (estado == 5)) {
		numRes.write( nResActual.read() );
		validProcCoste.write(true);
	}
	else
		validProcCoste.write(false);

	
	if (validIn.read() || (nuevoEstado == 3) || (estado == 3) || (estado == 4)) {
		if (contLect.read() == (nISLAS * NSOL - 1))
			NcontLect = 0;
		else
			NcontLect = contLect.read() + 1;
	}


	if (validNewCoste.read() || (estado == 4) || (estado == 5) || (estado == 6)) {	
		if (contRes == (nISLAS * NSOL - 1))
			NcontRes = 0;
		else
			NcontRes = contRes + 1;
	}

}


void assessment::regs() {

	sc_uint<NVARSdep> tmp;

	if (rst.read()) {
		contadorLecturasT = contadorEscriturasT = 0;
		contRef1 = 0;	 contRef2.write(0);
		contRes = contLectPrev = 0;	 contLect.write(0);
		estado = 0;
		islaIdx = 1;
		regValidIn = regValidInit = regDescartar = false;
		regT = 0; 
		regInitValue = 0.0; 
		validMinSq.write(false);
		noDescartarMinSq.write(false);
		weShift = 1;
		regUse = 0; 
		for (int i = 0; i < NVARSdepCost; ++i)
			dependientesReg[i].write(0);
	}
	else {

		for (int i = 0; i < NVARSdepCost; ++i)
			dependientesReg[i] = resultados[i];


		if (regValidInit) {	
			if (NVARSdepCost > 1) {
				tmp.bit(0) = weShift.bit(NVARSdepCost - 1);
				tmp(NVARSdepCost - 1, 1) = weShift(NVARSdepCost - 2, 0);
				weShift = tmp;
			}
		}

		contadorLecturasT = NcontadorLecturasT;
		contadorEscriturasT = NcontadorEscriturasT;
		contRef1 = NcontRef1;
		contRef2.write(NcontRef2);
		contRes = NcontRes;
		contLectPrev = contLect.read();
		contLect.write(NcontLect);
		
		estado = nuevoEstado;
		estadoSig.write(nuevoEstado);
		islaIdx = NislaIdx;

		regValidIn = validIn.read();
		regValidInit = validInit.read();
		regDescartar = descartar.read();
		regT = T.read();
		regInitValue.write(initValue.read());


		regUse = validIn.read() && (!descartar.read());	// descartar (discard) is not used for HH, as it implements RK-4 and all results are valid
	
		validMinSq.write(validIn.read());
		noDescartarMinSq.write(!descartar.read());

	}

	fire.write(!fire.read());

}


// implements min Square (in HH there is only 1 variable to consider, other optimization problems may require more)
void minSq::calculos() {

	if (rst.read()) {
		ptR = 1;
		ptW = 0;
		for (int j = 0; j < LAT_MIN_SQ; ++j) {
			validez[j] = false;
			keep[j] = false;
			resultados[j] = 0.0;
		}
	}
	else {
		validOut.write(validez[ptR]);
		keepOut.write(keep[ptR]);
		costeNuevo.write(resultados[ptR]);

		ptR = (ptR + 1) % LAT_MIN_SQ;

		validez[ptW] = validIn.read();	
		keep[ptW] = keepIn.read();

		if (validIn.read()) {
			
			resultados[ptW] = costeActual.read();
			for (int j = 0; j < NVARSdepCost; ++j) {
				double tmp;
				tmp = dependientes[j].read();


				tmp = tmp - referencias[j].read();
				resultados[ptW] += tmp * tmp;
				tmp = tmp;
			}
		}
			ptW = (ptW + 1) % LAT_MIN_SQ;
	}
}


void costDivider::calculos() {

	if (rst.read()) {
		ptR = 1;
		ptW = 0;
		for (int j = 0; j < LAT_DIV; ++j) {
			validez[j] = false;
			resultados[j] = 0.0;
		}
	}
	else {
		validOut.write(validez[ptR]);
		costePorMuestra.write(resultados[ptR]);

		ptR = (ptR + 1) % LAT_DIV;

		validez[ptW] = validIn.read();

		if (validIn.read()) {
			double c = coste.read();
			double m = muestras.read();
			resultados[ptW] = coste.read() / ((double)muestras.read());
			ptR = ptR;
		}
		ptW = (ptW + 1) % LAT_DIV;
	}

}


