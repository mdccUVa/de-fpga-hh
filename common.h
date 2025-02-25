#ifndef COMMON_H
#define COMMON_H

/* number of :

NSOL solutions per island
NVARS independent variables, which values are obtained through DE
NVARS dependent variables, their values are obtained from the independent ones
NVARSdepCost dependen variables that matter for cost evaluation (NVARSdepCost <= NVARSdep, and NVARSdepCost should go first in the list)
nISLAS islands of solutions
nBitsIslas number of bits for addressing islands (it could be obtained using logarithm)
*/

#define NSOL 16
#define NVARS 11 
#define NVARSdep 4
#define NVARSdepCost 1
#define nISLAS 4
#define nBitsIslas 2
//#define nITER 512

// n bits for address 
// latencies of differente FP units

#define addrBits 9
#define LAT_HIB_1 15
#define LAT_HIB_2 6
#define LAT_LIM 10
#define LAT_DEP 130
#define LAT_PIPE 531
#define LAT_COMP 2
#define LAT_MIN_SQ 10
#define LAT_DIV 5

#define FACTOR (double)(0.9)
#define deltaTinicial 0.04
#define granularidadTiempo 0.04
#define nPOINTS 255
#define minPoints (int)(nITER/2.0)
#define maxCoste 1e20



#endif;

