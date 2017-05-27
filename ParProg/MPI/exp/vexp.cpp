#include <mpi.h>
#include <iostream>
#include <cstdlib>

const unsigned ROOT = 0;

int main(int argc, char *argv[])
{
	MPI :: Init( argc, argv );
	MPI :: COMM_WORLD.Set_errhandler( MPI :: ERRORS_THROW_EXCEPTIONS );

	try {
		if ( argc != 2 ) {
			std :: cout << "Need precision\n";
			return -1;
		}
		unsigned long n = atol(argv[1]);
		//unsigned long long n = 10000000000;
		/*	void MPI::Comm::Bcast(void* buffer, int count,
					  const MPI::Datatype& datatype, int root) const = 0*/
		MPI :: COMM_WORLD.Bcast( &n, 1, MPI::INT, 0 );

		int				rank			= MPI :: COMM_WORLD.Get_rank();
		int				numproc			= MPI :: COMM_WORLD.Get_size();
		double			start			= 0.0;
		double			end				= 0.0;
		double			exp				= 0.0;
		double 			peace_of_exp	= 0.0;
		unsigned long 	bound_0			= rank * n / numproc;
		unsigned long	bound_1			= (rank + 1) * n / numproc;
		double			last			= 0;
		if ( rank == 0 ) {
			start = MPI :: Wtime();
			last = 1;
		}
		else
			last = 1/bound_0;
		//std :: cout << "rank = " << rank << " bound_0 = " << bound_0 << " bound_1 = " << bound_1 << std::endl;

		unsigned long i = bound_0;
		while ( i < bound_1 )
		{
			peace_of_exp 	+= last;
			++i;
			last			/= i;
		}

		if ( rank == ROOT ) {
			double p = 0.0;
			double l = 0.0;

			for ( i = numproc-1; i > 0; --i )
			{
				MPI :: COMM_WORLD.Recv ( &l, 1, MPI :: DOUBLE, i, 1 );
				MPI :: COMM_WORLD.Recv ( &p, 1, MPI :: DOUBLE, i, 1 );
				exp *= l;
				exp += p;
			}
			exp *= last;
			exp += peace_of_exp;
		} else {
			MPI :: COMM_WORLD.Send ( &last, 1, MPI :: DOUBLE, ROOT, 1 );
			MPI :: COMM_WORLD.Send ( &peace_of_exp, 1, MPI :: DOUBLE, ROOT, 1 );
		}

		if ( rank == 0 ) {
			end = MPI :: Wtime ();
			std :: cout.precision(50);
			std :: cout << exp << std :: endl;
			std :: cout << "Time of calculation: " << end - start << "sec\n";
		}
	} catch ( MPI :: Exception e ) {
		std :: cerr << "MPI ERROR: " << e.Get_error_code() \
		<< " - " << e.Get_error_string() << std :: endl;
	}

	MPI :: Finalize();
	return 0;
}
