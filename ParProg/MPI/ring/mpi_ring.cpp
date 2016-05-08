// mpi libs
#include <mpi.h>

// C++ std lib
#include <iostream>


int main( int argc, char *argv[] ) 
{
	MPI :: Init( argc, argv );
	// устанавливаем обработчик для исключений
	// MPI :: ERRORS_THROW_EXCEPTIONS - стандартный обработчик
	MPI :: COMM_WORLD.Set_errhandler( MPI :: ERRORS_THROW_EXCEPTIONS );
	
	try {
		int rank = MPI :: COMM_WORLD.Get_rank();
		int n_rank = rank + 1;
		int p_rank = rank - 1;
		int m_rank = MPI :: COMM_WORLD.Get_size() - 1;
		
		if ( !rank  ) {
			std :: cerr << "I am " << rank << std :: endl;
			
			if ( rank != m_rank )
				// void Comm::Send(const void* buf, int count, const Datatype&
				//     datatype, int dest, int tag) const			
				MPI :: COMM_WORLD.Send ( &rank, 1, MPI :: INT, n_rank, 1);
		}	
		else {
			int prev = 0;
			MPI :: COMM_WORLD.Recv ( &prev, 1, MPI :: INT, p_rank, 1);	
			std :: cerr << "I am " << rank << " prev = " << prev <<  std :: endl;
			
			if ( rank != m_rank )
				MPI :: COMM_WORLD.Send ( &rank, 1, MPI :: INT, n_rank, 1);
		} 
	}
	catch ( MPI :: Exception e ) {
	 	std::cout << "MPI ERROR: " << e.Get_error_code() \
		<< " - " << e.Get_error_string() << std :: endl;
	}

	MPI :: Finalize();
	return 0;
}

