#ifndef LOOPER_h
#define LOOPER_h

#include "systemc.h"
#include "common.h"

// looper basically cancels validRecirc during the last iteration

SC_MODULE (looper) {
public:
	sc_in_clk			clk;
	sc_in<bool>			rst;

	sc_in<bool>			validIn;
	sc_out<bool>		validRecirc; 


void regs(){

	if(rst.read()){
		contador = 0; 
	}else{
		if(validIn.read())
			if(contador == ( (nPOINTS * NSOL * nISLAS) - 1) )
				contador = 0;
			else 
				contador = contador + 1; 
	}
	fire.write( !fire.read() ); 

}


void comb(){

	if( contador >= ( (nPOINTS +1) * NSOL * nISLAS) ) 
		validRecirc.write(false); 
	else
		validRecirc.write( validIn.read() ); 
}


SC_CTOR(looper) {
	cout << "looper: " << name() << endl;

	fire.write(0);

	SC_METHOD(regs);
	sensitive << clk.pos();

	SC_METHOD(comb);
	sensitive << validIn << fire;


  }

private:
	int contador; 
	sc_signal<bool> fire; 
}; 

#endif;