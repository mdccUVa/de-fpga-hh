#ifndef MEM64DP_H
#define MEM64DP_H

#include "systemc.h"

#define memElements (1 << addrBits)


// DIFFERENT MEMORY MODULES
// ALL OF THEM CAN BE IMPLEMENTED USING XILINX'S BRAM	


SC_MODULE(mem64dp) {
public:

	sc_in<bool> clk;

	sc_in<sc_uint<addrBits>>	addrA, addrB;
	sc_in<double>		dataIn;
	sc_in<bool>			we;
	sc_out<double>	dataA, dataB;

	void puertoA() {

		double dato;
		sc_uint<addrBits> dir;
		
		dir = addrA->read();
		if (we->read()) {
			dato = dataIn->read();
			mem[dir] = dato;
		}
		else
			dataA->write(mem[dir]);
	}

	void puertoB() {
		double dato;
		sc_uint<addrBits> dir;

		dir = addrB->read();
		dataB->write(mem[dir]);
	}

	void dump(int number) {	// DUMPS the first "number" of pieces of data (for debugging purposes) 
		cout << name() << " ";
		for (int i = 0; i < number; ++i)
			cout << mem[i] << ";";
		cout << endl;
	}

	SC_CTOR(mem64dp) {

		cout << "mem64dp: " << name() << endl;
		SC_METHOD(puertoA);
		sensitive << clk.pos();

		SC_METHOD(puertoB);
		sensitive << clk.pos();
	}


private:
	double mem[memElements];
};



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////



SC_MODULE(mem64sp) {
public:

	sc_in<bool> clk;

	sc_in<sc_uint<addrBits>>	addr;
	sc_in<double>		dataIn;
	sc_in<bool>			we;
	sc_out<double>	dataOut;

	void puerto() {

		double dato;
		sc_uint<addrBits> dir;

		dir = addr->read();
		if (we->read()) {
			dato = dataIn->read();
			mem[dir] = dato;
		}
		else
			dataOut->write(mem[dir]);
	}

	void dump(int number) {
		cout << name() << " ";
		for (int i = 0; i < number; ++i) 
			cout << mem[i] << " ";
		cout << endl;
	}


	SC_CTOR(mem64sp) {
		cout << "mem64sp: " << name() << endl;
		SC_METHOD(puerto);
		sensitive << clk.pos();
	}


private:
	double mem[memElements];
};



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////


SC_MODULE(mem8x512) {
public:

	sc_in<bool> clk;

	sc_in<sc_uint<9>>	addrRead, addrWrite;
	sc_in<sc_uint<8>>	dataIn;
	sc_in<bool>			we;
	sc_out<sc_uint<8>>	dataOut;

	void puertoRead() {

		sc_uint<9> dir;

		dir = addrRead->read();
		dataOut->write(mem[dir]);
	}

	void puertoWrite() {

		sc_uint<8> dato;
		sc_uint<9> dir;

		if (we->read()) {
			dir = addrWrite->read();
			dato = dataIn->read();
			mem[dir] = (unsigned char)(dato.to_uint());
		}
	}


	void dump(int number) {
		cout << name() << " ";
		for (int i = 0; i < number; ++i)
			printf("%d ", mem[i]);
		printf("\n");
	}


	SC_CTOR(mem8x512) {
		for (int i = 0; i < 512; ++i)	mem[i] = i;	// unnecessary initialization

		cout << "mem8x512: " << name() << endl;
		SC_METHOD(puertoRead);
		sensitive << clk.pos();
		SC_METHOD(puertoWrite);
		sensitive << clk.pos();

	}


private:
	unsigned char mem[512];
};



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////


SC_MODULE(mem64x512) {
public:

	sc_in<bool> clk;

	sc_in<sc_uint<9>>	addrRead, addrWrite;
	sc_in<double>		dataIn;
	sc_in<bool>			we;
	sc_out<double>		dataOut;

	void puertoRead() {

		sc_uint<9> dir;

		dir = addrRead->read();
		dataOut->write(mem[dir.to_uint()]);
	}

	void puertoWrite() {

		double dato;
		sc_uint<9> dir;

		if (we->read()) {
			dir = addrWrite->read();
			dato = dataIn->read();
			mem[dir.to_uint()] = dato;
		}
	}


	void dump(int number) {
		cout << name() << " ";
		for (int i = 0; i < number; ++i)
			cout << mem[i] << " ";
		cout << endl;
	}


	SC_CTOR(mem64x512) {
		for (int i = 0; i < 512; ++i)	mem[i] = i;	// unnecessary initialization

		cout << "mem64x512: " << name() << endl;
		SC_METHOD(puertoRead);
		sensitive << clk.pos();
		SC_METHOD(puertoWrite);
		sensitive << clk.pos();

	}


private:
	double mem[512];
};





#endif;