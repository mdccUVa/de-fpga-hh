#include "limits.h"



void limitsMemory::memoria() {

	if (!rst.read()) {

		// max & min are stored in a shift-register

		if (validInit.read()) {
			for (int i = 0; i < (NVARS - 1); ++i) {
				max[i] = min[i];
				min[i] = max[i + 1];
			}
			max[NVARS - 1] = min[NVARS - 1];
			min[NVARS - 1] = initLimit.read();
		}

		for (int i = 0; i < NVARS; ++i) {
			maxOut[i].write(max[i]);
			minOut[i].write(min[i]);
		}
	}

}



void limitsCalculations::calculos() {
	
	if (rst.read()) {
		ptR = 1;
		ptW = 0; 
		for (int i = 0; i < NVARS; ++i)
			for (int j = 0; j < LAT_LIM; ++j){		
				resultados[i][j] = 0.0;	
				validez[j] = false;
			}
	}
	else {		
		validOut.write( validez[ptR] ); 
		for (int i = 0; i < NVARS; ++i)
			RES[i].write(resultados[i][ptR]);

		ptR = (ptR + 1) % LAT_LIM;

		validez[ptW] = validIn.read();	

		if (validIn.read()) {			
			for (int i = 0; i < NVARS; ++i){
				double leido = HIB[i].read();
				if (leido > maxIn[i].read())	leido = 2 * maxIn[i].read() - leido;	// at most, one of them is possible
				if (leido < minIn[i].read())	leido = 2 * minIn[i].read() - leido;
				resultados[i][ptW] = leido;
			}
		}
		ptW = (ptW + 1) % LAT_LIM;
	}
}
