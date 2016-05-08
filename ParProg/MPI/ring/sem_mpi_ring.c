#include <mpi.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

/********************************************************************
* Using for function semctl();
********************************************************************/
union semun  {
		int val; 				// используется при SETVAL
		struct semid_ds *buf;	// используется IPC_SET IPC_STAT
		ushort *array;			// используется GETALL SETALL
	};

/*******************************************************************/
const int CREAT_SEM_F = IPC_CREAT | IPC_EXCL;
const int ACCESS_SEM_F = 0666;

int Create_Sem ( key_t key_sem, unsigned num, int flag )
{
	// IPC_CREAT - создаёт новый, или возвращает id существующего
    // IPC_EXCL - если установлен вместе с IPC_CREAT, то успешное выполнение только при первом создании
    int sem_id = semget ( key_sem, num, flag );
    if (  errno == EEXIST  ) {
      	printf ( "This program is already running\n" );
		MPI_Abort ( MPI_COMM_WORLD, -1 );
    }
    else if (  sem_id < 0 ) {
       	printf ( "ERROR: can't create semaphore\n" );
		MPI_Abort ( MPI_COMM_WORLD, -1 );
    }

    // SETVAL - устанавливает значение семафора
   	union semun arg;
	arg.val = 0;

	semctl ( sem_id, 0, SETVAL, arg );

	printf ( "val = %d\n", semctl(sem_id, 0, GETVAL) );
	fflush(stdout);

	return sem_id;
}

/*******************************************************************/
int Connect_to_Sem ( key_t key_sem, unsigned num, int flag )
{
	int sem_id = semget ( key_sem, num, flag );
    if (  sem_id < 0 ) {
       	printf ( "ERROR: can't create semaphore\n" );
		MPI_Abort ( MPI_COMM_WORLD, -1 );
    }

	return sem_id;
}

/*******************************************************************/
int main ( int argc, char *argv[] )
{
    MPI_Init ( &argc, &argv );

	int Fd = open ( "t.txt", O_RDWR | O_CREAT | O_APPEND, 0666 );

    /* определяет rank(id)
    *  MPI_COMM_WORLD - глобальный коммуникатор
    */
    int rank = 0;
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );

    int m_rank = 0;
    MPI_Comm_size ( MPI_COMM_WORLD, &m_rank);

    // Начало создания и открытия семафора System V & shared memory
	int sem_id = 0;

    key_t key_sem = ftok ( argv[0], 1 );
    if ( key_sem < 0 ) {
		printf ( "ERROR: can't take key\n" );
        return -1;
    }

	if ( rank == 0 )
    	sem_id = Create_Sem ( key_sem, 1, ACCESS_SEM_F | CREAT_SEM_F );

	MPI_Barrier ( MPI_COMM_WORLD );
	if ( rank )
		sem_id = Connect_to_Sem ( key_sem, 1, ACCESS_SEM_F );
	// Конец создания и открытия семафора System V & shared memory


    struct sembuf sem_buf[1];
    sem_buf[0].sem_num = 0;
    sem_buf[0].sem_op = 0 - rank;
    sem_buf[0].sem_flg = SEM_UNDO;

    if ( semop ( sem_id, &sem_buf[0], 1 ) ) {
		semctl ( sem_id, 0, IPC_RMID);
        return -1;
    }

    printf ( "Goodbuy, my rank = %d\n", rank );
    fflush ( stdout );

	printf ( "prank = %d\n", rank );

	char a = '0'+rank;
	write ( Fd, &a, 1 );
    sem_buf[0].sem_num = 0;
    sem_buf[0].sem_op = rank + 1;
    sem_buf[0].sem_flg = SEM_UNDO;

    if ( semop ( sem_id, &sem_buf[0], 1 ) ) {
      	semctl ( sem_id, 0, IPC_RMID);
		return -1;
    }

    if ( rank == m_rank-1 ) {
        // IPC_RMID - удаляет семафор
        semctl ( sem_id, 0, IPC_RMID);
    }

    MPI_Finalize();
    return 0;
}
