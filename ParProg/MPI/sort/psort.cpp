#include <iostream>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

long int *RgenerateIntArr( long n ); 

template < typename T >
void SShell( T *ar, long n ); 
template < typename T >
void PrintArr( T *ar, long n );
template < typename T >
void CopyArr( T *sar, T *dar, long n );
template < typename T >
void Merge( T *sar, T *tar, long ls, long lt, T *dar); 
template < typename T >
bool CheckSort( T *ar, long n);

const int ROOT = 0;

int cmp( const void *a, const void *b )
{
	if ( *((long*)a) > *((long*)b)) return 1;
	if ( *((long*)a) < *((long*)b)) return -1;
	return 0;
}

int main( int argc, char *argv[] )
{
	MPI :: Init( argc, argv );
	MPI :: COMM_WORLD.Set_errhandler( MPI :: ERRORS_THROW_EXCEPTIONS );
	
	long int rank = MPI :: COMM_WORLD.Get_rank();
	long int size = MPI :: COMM_WORLD.Get_size();
	double start = 0.0;
	double end = 0.0;

	long int num = atoll(argv[1]);
	MPI :: COMM_WORLD.Bcast( &num, 1, MPI::LONG,  0 );
	if ( (num % size) != 0 ) {
		long int ext = size - num%size;
		num += ext;
	}
	long int mn = num/size;
	long int *war = new long int[mn];

	long int *arr;
	if( rank == ROOT ) { 
		arr = RgenerateIntArr(num);
		//PrintArr(arr, num); 
	}
 	
	MPI :: COMM_WORLD.Scatter ( arr, mn, MPI::LONG, 
								war, mn, MPI::LONG, ROOT);

	MPI :: COMM_WORLD.Barrier();
	if( rank == ROOT )
		start = MPI :: Wtime(); 
			
	//SShell( war, mn );
	qsort(war, mn, sizeof(long), cmp);
	//PrintArr(war, mn);
	
	const long int TWO = 2;
	const long int ONE = 1;
	long int *rarr;
	long int *carr;
	long int rn = 0;
	for ( long int i = 0; (ONE<<i) < size; ++i)
	{
		if ( (rank % (TWO<<i)) == 0 ) {
			if ( (rank+(ONE<<i)) >= size )
				continue;
			MPI :: COMM_WORLD.Recv(&rn, ONE, MPI::LONG,
									rank+(ONE<<i), ONE);
			rarr = new long int[rn];
			MPI :: COMM_WORLD.Recv(rarr, rn, MPI::LONG,
									rank+(ONE<<i), TWO);
			mn += rn;
			carr = new long int[mn];
			Merge( war, rarr, mn-rn, rn, carr);
			delete [] rarr;
			delete [] war;
			war = carr; 
		}
		else {
			if ( rank == ROOT )
				break;
			MPI :: COMM_WORLD.Send(&mn, ONE, MPI::LONG,
									rank-(ONE<<i), ONE);	
			MPI :: COMM_WORLD.Send(war, mn, MPI::LONG,
									rank-(ONE<<i), TWO);	
			break;
		}
	}

	
	MPI :: COMM_WORLD.Barrier();
	if( rank == ROOT ) {
		end = MPI :: Wtime();
		//PrintArr(war,num);
		std :: cout << "Sort is " << (CheckSort( war, num )? "success\n": "fail\n")
					<< "SortTime: " << end-start << "sec\n";
		delete [] arr;
	}

	delete [] war;
	MPI :: Finalize();	
	return 0;
}

//********************************************************************
template < typename T >
void Merge( T *sar, T *tar, long ls, long lt, T *dar)
{
	long int is = 0;
	long int it = 0;
	long int id = 0;
	while ( ( is < ls ) || ( it < lt ) )
	{
		if ( is == ls ) 
			dar[id++] = tar[it++];
		else if ( it == lt ) 
			dar[id++] = sar[is++];
		else if ( sar[is] < tar[it] ) 
			dar[id++] = sar[is++];
		else 
			dar[id++] = tar[it++];
	}	
}

//********************************************************************
template < typename T >
void CopyArr( T *sar, T *dar, long n )
{
	for( long i = 0; i < n; ++i)
		dar[i] = sar[i];	
}

//********************************************************************
long int *RgenerateIntArr( long n )
{
	long int *array = new long int[n];	
	srand48( time(NULL) );
	for( long i = 0; i < n; ++i )
		array[i] = mrand48();
	
	return array; 
}

//********************************************************************
template < typename T >
void SShell( T *ar, long n)
{
	long h = 1;
	while( h <= n/3 )
		h = 3*h + 1;

	for(; h > 0; h = (h-1)/3 )
		for( long i = h; i < n; ++i)
		{
			T temp = ar[i];
			long j = i;
			for(; (j > h-1) && (ar[j-h] >= temp); j -= h )
				ar[j] = ar[j-h];

			ar[j] = temp;
		}  
} 

//********************************************************************
template < typename T >
bool CheckSort( T *ar, long n)
{
	bool flag = true;
	switch(n) {
	case 0:
		// need throw
		break;
	case 1:
		break;
	default:
		for( long i = 1; i < n; ++i )
			if( ar[i] < ar[i-1] )
				flag = false;
		break;
	}

	return flag; 
}

//********************************************************************
template < typename T >
void PrintArr( T *ar, long n )
{
	for ( long i = 0; i < n; ++i )
		std::cout << ar[i] << " ";
	std :: cout << "\n";
} 

