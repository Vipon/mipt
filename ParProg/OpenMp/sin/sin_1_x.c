#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_STEP 100000

double normalize ( double arg )
{
	double d_pi = 2 * M_PI;
	while ( arg > d_pi )
		arg -= d_pi;

	return arg;
} 

double fact ( long long count )
{
	double f = 1;
	long long i = 0;

	for ( i = 0; i < count; )
		f *= ++i; 

	return f;
}

int main ( int argc, char *argv[] )
{
	if ( argc < 3 ) {
		printf ( "I need two arguments:\n" );
		printf ( "[NUM_THREADS] [sin(1/x) ARGUMENT]\n" );
		return -1;
	}

	int i = 0;
	int numThreads = atoi ( argv[1] );
	double sum = 0;
	double arg = atof ( argv[2] );
	if ( arg == 0 ) {
		printf ( "arg is zero\n" );
		return -1;
	}
	arg = normalize ( 1 / arg );

	omp_set_num_threads ( numThreads );
	#pragma omp parallel for reduction(+:sum) schedule(dynamic)
	for ( i = 0; i < MAX_STEP; ++i )
		sum +=  pow ( -1, i ) * pow ( arg, (2*i + 1) ) / fact( 2*i + 1 );
	
	printf ( "%.50f\n", sum );	
}
