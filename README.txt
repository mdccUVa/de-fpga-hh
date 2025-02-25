/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Authors: Roberto Rodriguez Osorio, University of a Corunna, Spain.         //
//           Manuel de Castro Caballero, University of Valladolid, Spain.      //
//                                                                             //
//  Submitted as an artifact to FCCM conference 2025 for the review process.   //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

The included code implements the simulation of a hardware architecture for 
Differential Evolution to optimize the choice of parameters for the Hodgkin-Huxley model. 

Hardware architecture is modelled using SystemC. The reader can set up the SystemC libraries
and then compile and run the included code. 

Simulation is slow so, despite the program will report every time a new solution outperforms
and existing one, the user should not expect any real convergence in a reasonable time. 

VHLD or Verilog code can be derived from the SystemC one, as it is cycle accurate. 
We have implemented the DE architecture using VHDL, and the floating point operations using
AMD/Xilinx Vivado HLS. The resulting code, however, is not provided. 

The user may try to convert SystemC to any HDL language and synthesize it. Considering that 
BRAM primitives are intended to mimic Xilinx's ones, some adaptation will be required when 
targetting Intel/Altera devices. 

Lastly, our SystemC implementation does not include the code to extract and update the sets 
of solutions after a number of iterations. 

