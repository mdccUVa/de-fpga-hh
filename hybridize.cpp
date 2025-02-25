#include "hybridize.h"


void hybridize1::calculos() {
	
	if (rst.read()) {
		ptR = 1;
		ptW = 0; 
		for (int j = 0; j < LAT_HIB_1; ++j){
			validez[j] = false; 
			for (int i = 0; i < NVARS; ++i)
				resultados[i][j] = 0.0;		
		}
	}
	else {		
		validOut.write( validez[ptR] ); 
		for (int i = 0; i < NVARS; ++i)
			RES[i].write(resultados[i][ptR]);

		ptR = (ptR + 1) % LAT_HIB_1;

		validez[ptW] = validIn.read();

		if (validIn.read()) {			
			for (int i = 0; i < NVARS; ++i)
				resultados[i][ptW] = (A[i].read() - B[i].read()) * FACTOR;
		}
		ptW = (ptW + 1) % LAT_HIB_1;
	}
}


void hybridize2::calculos() {

	if (rst.read()) {
		ptR = 1;
		ptW = 0;
		for (int j = 0; j < LAT_HIB_2; ++j){
			validez[j] = false; 
			for (int i = 0; i < NVARS; ++i)
				resultados[i][j] = 0.0;		
		}
	}
	else {
		validOut.write( validez[ptR] ); 
		for (int i = 0; i < NVARS; ++i)
			RES[i].write(resultados[i][ptR]);

		ptR = (ptR + 1) % LAT_HIB_2;

		// these lines detect if there is a mismatch between the estimated latency and the scheduling when issuing addrC
		if (validIn1.read() != validIn2.read())
			cerr << "DATA HAS NOT ARRIVED AT THE RIGHT TIME FOR HIBRIDACION_2" << endl; 

		validez[ptW] = validIn1.read();	

		if (validIn1.read()) {	
			for (int i = 0; i < NVARS; ++i)
				resultados[i][ptW] = A[i].read() + B[i].read();

		}
		ptW = (ptW + 1) % LAT_HIB_2;
	}
}



