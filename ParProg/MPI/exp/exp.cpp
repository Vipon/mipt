#include <mpi.h>
#include <iostream>
#include <cstdlib>

long double MFact( int cur, int prev, long double last )
{ 
	long double f = last;
	for ( int i = prev+1; i <= cur; ++i )
		f *= i;
	
	return f;
}

int main(int argc, char *argv[])
{
	MPI :: Init( argc, argv );
	MPI :: COMM_WORLD.Set_errhandler( MPI :: ERRORS_THROW_EXCEPTIONS );	

	try {
		unsigned n = atoi(argv[1]);
		/*	void MPI::Comm::Bcast(void* buffer, int count,
    							  const MPI::Datatype& datatype, int root) const = 0*/
		MPI :: COMM_WORLD.Bcast( &n, 1, MPI::INT, 0 ); 

		int    	rank    = MPI :: COMM_WORLD.Get_rank();
		int   	numproc = MPI :: COMM_WORLD.Get_size();
		double	start 	= 0.0;
		double	end 	= 0.0;

		if ( rank == 0 )
			start = MPI :: Wtime();

		long double peace_of_exp = 0.0;
		long double last = 1;
		int prev = 1;
		for ( int i = rank; i <= n; i += numproc )
		{
			last = MFact(i, prev, last);
		//	std::cout << "last " << last << " prev " << prev << " cur " << i << "\n";
			peace_of_exp += 1/last;
			prev = i;
		}

		long double exp = 0.0;
		/*	void MPI::Intracomm::Reduce(const void* sendbuf, void* recvbuf, int count, 
 										const MPI::Datatype& datatype, const MPI::Op& op,
    									int root) const*/
		MPI :: COMM_WORLD.Reduce( &peace_of_exp, &exp, 1, MPI :: LONG_DOUBLE, MPI :: SUM, 0 );

		if ( rank == 0 ) {
			end = MPI :: Wtime ();
			std :: cout.precision(30);
			std :: cout << exp << std :: endl;
			std :: cout << "Time of calculation: " << end - start << "sec\n";  
		}
	}
	catch ( MPI :: Exception e ) {
		std :: cerr << "MPI ERROR: " << e.Get_error_code() \
		<< " - " << e.Get_error_string() << std :: endl;
	}
 
	MPI :: Finalize();
	return 0;
}
