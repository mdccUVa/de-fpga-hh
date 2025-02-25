#include "calculations.h"


int CICLO; 


// Hodgkin-Huxley implementation
// it does not pretend to be close to the synthesizable version

void calculations::calcular() {


	int i;
	double t, dt;
	double error;
		
	CICLO = (int)(sc_time_stamp().to_double()/1000.0);

	if (rst.read()) {
		ptR = 1;
		ptW = 0; 
		for (int j = 0; j < LAT_PIPE; ++j){
			validez[j] = false; 
			repe[j] = false; 
			T[j] = deltaT[j] = 0.0; 
			Tint[j] = 0;
			for (int i = 0; i < NVARSdep; ++i)
				resDEP[i][j] = 0.0;	
		}
		contador = 0; 
	}
	else {

		validOut.write(validez[ptR]);
		if (validez[ptR]) {
			repetir.write(false);		// en RK4 all iterations converge
			++contador; 
			
			Tout.write(T[ptR]);
			Tinteger.write(Tint[ptR]);

			deltaTout.write(deltaT[ptR]);	
			for (i = 0; i < NVARSdep; ++i)
				RES[i].write(resDEP[i][ptR]);
		}
		else {
			repetir.write(0);
			Tout.write(0);
			Tinteger.write(0);
			deltaTout.write(0);
			for (i = 0; i < NVARSdep; ++i)
				RES[i].write(0);
		}

		ptR = (ptR + 1) % LAT_PIPE;


		validez[ptW] = validIn.read();	
		
		if (!validIn.read())
			ptR = ptR; 

		if (validIn.read()) {	
			t = Tin.read();
			dt = deltaTin.read();

			// calculations

			double V = dependientes[0].read();
			double n = dependientes[1].read();
			double m = dependientes[2].read();
			double h = dependientes[3].read();

			double an1 = independientes[0].read(); 
			double an3I = independientes[1].read();
			double am1 = independientes[2].read();
			double am3I = independientes[3].read();
			double ah1 = independientes[4].read();
			double ah3N = independientes[5].read();

			double bn1 = independientes[6].read();
			double bn3I = independientes[7].read();
			double bm1 = independientes[8].read();
			double bm3N = independientes[9].read();
			double bh3N = independientes[10].read();

			double k1, k2, k3, k4;
			double n1, m1, h1;
			double n2, m2, h2;
			double n3, m3, h3;
			double n4, m4, h4;

			double pn1, pm1, ph1;
			double pn2, pm2, ph2;
			double pn3, pm3, ph3;
			double pn4, pm4, ph4;


			k1 = dt * HH_calculo(V, &pn1, &pm1, &ph1, n, m, h,
				an1, an3I, am1, am3I, ah1, ah3N,
				bn1, bn3I, bm1, bm3N, bh3N);
			n1 = pn1 * dt; 		m1 = pm1 * dt;		h1 = ph1 * dt;

			k2 = dt * HH_calculo(V + (0.5 * k1), &pn2, &pm2, &ph2, n + (0.5 * n1), m + (0.5 * m1), h + (0.5 * h1),
				an1, an3I, am1, am3I, ah1, ah3N,
				bn1, bn3I, bm1, bm3N, bh3N);
			n2 = pn2 * dt; 		m2 = pm2 * dt;		h2 = ph2 * dt;

			k3 = dt * HH_calculo(V + (0.5 * k2), &pn3, &pm3, &ph3, n + (0.5 * n2), m + (0.5 * m2), h + (0.5 * h2),
				an1, an3I, am1, am3I, ah1, ah3N,
				bn1, bn3I, bm1, bm3N, bh3N);
			n3 = pn3 * dt; 		m3 = pm3 * dt;		h3 = ph3 * dt;

			k4 = dt * HH_calculo(V + k3, &pn4, &pm4, &ph4, n + n3, m + m3, h + h3,
				an1, an3I, am1, am3I, ah1, ah3N,
				bn1, bn3I, bm1, bm3N, bh3N);
			n4 = pn4 * dt; 		m4 = pm4 * dt;		h4 = ph4 * dt;

			resDEP[1][ptW] = n + (n1 + 2 * n2 + 2 * n3 + n4) * 0.16666666666666666666666666666666;
			resDEP[2][ptW] = m + (m1 + 2 * m2 + 2 * m3 + m4) * 0.16666666666666666666666666666666;
			resDEP[3][ptW] = h + (h1 + 2 * h2 + 2 * h3 + h4) * 0.16666666666666666666666666666666;

			resDEP[0][ptW] = V + (k1 + 2 * k2 + 2 * k3 + k4) * 0.16666666666666666666666666666666;

			// end of calculations


			T[ptW] = t + dt;
			Tint[ptW] = (int)(t * 25.0);	// (time step is 0.04)	
			deltaT[ptW] = dt; 
			repe[ptW] = false;
		}

		ptW = (ptW + 1) % LAT_PIPE;

	}

}


double calculations::HH_calculo(double V, double* dn, double* dm, double* dh, double n, double m, double h,
	double an1, double an3I, double am1, double am3I, double ah1, double ah3N,
	double bn1, double bn3I, double bm1, double bm3N, double bh3N) {



	const double ENa = 55.17;	// mv Na reversal potential
	const double EK = -72.14;	// mv K reversal potential
	const double El = -49.42;	// mv Leakage reversal potential
	const double gbarNa = 1.2;	// mS/cm^2 Na conductance
	const double gbarK = 0.36;	// mS/cm^2 K conductance
	const double gbarl = 0.003; // mS/cm^2 Leakage conductance
	const double I = 0.1;		// Applied Current
	const double CmI = 1.0 / 0.01;	// Membrane Capacitance

	const double an2 = 50.0;
	const double am2 = 35.0;
	const double ah2 = 60.0;
	const double bn2 = 50.0;
	const double bm2 = 50.0;
	const double bh2 = 50.0;
	const double bh1 = 1.0;

	double dydt, gNa, gK, gl, INa, IK, Il;

	gNa = gbarNa * (m * m * m) * h;
	gK = gbarK * (n * n * n * n);
	gl = gbarl;
	INa = gNa * (V - ENa);
	IK = gK * (V - EK);
	Il = gl * (V - El);

	dydt = CmI * (I - (INa + IK + Il));

	double an = (an1 * (V + an2) / (1 - exp(-(V + an2) * an3I)));
	double am = (am1 * (V + am2) / (1 - exp(-(V + am2) * am3I)));
	double ah = (ah1 * exp(ah3N * (V + ah2)));

	double bn = (bn1 * exp(-(V + bn2) * bn3I));
	double bm = (bm1 * exp(bm3N * (V + bm2)));
	double bh = (1.0 / (bh1 + exp(bm3N * (V + bh2))));


	*dn = an * (1.0 - n) - bn * n;
	*dm = am * (1.0 - m) - bm * m;
	*dh = ah * (1.0 - h) - bh * h;

	return dydt;
}
