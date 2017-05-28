#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>

#define BUFF_SIZE 256

int SEM_OP ( int sem_num, int sem_op, int sem_flg, int sem_id, struct sembuf *Sem_Buf, int nSem_Buf )
{
    Sem_Buf -> sem_num = sem_num;
    Sem_Buf -> sem_op = sem_op;
    Sem_Buf -> sem_flg = sem_flg;

    if ( semop ( sem_id, Sem_Buf, nSem_Buf ) )
        return -1;

    return 0;
}


int main ( int argc, char * argv[] )
{
    int Fd_In = 0;
    int Fd_Out = 0;
    int Syn_Fd = 0;
    int mem_id = 0;
    int sem_id = 0;
    unsigned char Read_Res = 1;
    int i = 0;
    key_t key_mem;
    key_t key_sem;
    unsigned char Buff[BUFF_SIZE] = {};
    char *addr_m = 0;
    int VALS = 0;
    struct sembuf Sem_Buf[2];

    if ( argc != 4 ) {
        printf ( "I don't find some argument. Maybe you forgot give me it\n" );
        return -1;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    Syn_Fd = open ( argv[2], O_RDWR | O_CREAT , 0644 );
    if ( Syn_Fd < 0 ) {
        printf ( "I can't connect with %s\n", argv[2] );
        return -1;
    }

    close ( Syn_Fd );

    key_mem = ftok ( argv[2], 0 );
    mem_id = shmget ( key_mem, BUFF_SIZE, 0644 | IPC_CREAT );
    if ( mem_id < 0 ) {
        printf ( "I can't connect with SH_MEM\n" );
        return -1;
    }

    addr_m = (char*) shmat ( mem_id, NULL, 0 );

    /////////////////////////////////////////////////////////////////////////////////////////

    key_sem = ftok ( argv[2], 1 );
    sem_id = semget ( key_sem, 5, 0644 | IPC_CREAT );
    if ( sem_id < 0 ) {
        printf ( "I can't connect with sem\n" );
        return -1;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    switch ( argv[1][0] ) {
    case 'r':
        Sem_Buf[0].sem_num = 3;
        Sem_Buf[0].sem_op = 0;
        Sem_Buf[0].sem_flg = IPC_NOWAIT;

        Sem_Buf[1].sem_num = 3;
        Sem_Buf[1].sem_op = 1;
        Sem_Buf[1].sem_flg = SEM_UNDO;
        if ( semop ( sem_id, &Sem_Buf, 2 ) ) {
            printf ( "Reader already exist\n" );
            return -1;
        }

        semctl ( sem_id, 0, SETVAL, 0 );
        semctl ( sem_id, 1, SETVAL, 0 );
        semctl ( sem_id, 2, SETVAL, 0 );
        SEM_OP ( 0, 2, 0, sem_id, &Sem_Buf, 1 );
        SEM_OP ( 0, -2, SEM_UNDO, sem_id, &Sem_Buf, 1 );
        SEM_OP ( 1, 2, SEM_UNDO, sem_id, &Sem_Buf, 1 );
        SEM_OP ( 2, -1, 0, sem_id, &Sem_Buf, 1 );

        Fd_In = open ( argv[3], O_RDONLY );
        if ( Fd_In < 0 ) {
            printf ( "I can't open input fule - %s\n", argv[3] );
            return -1;
        }

        while ( Read_Res ) {
            Read_Res = read ( Fd_In, &Buff, BUFF_SIZE-1 );
            if ( Read_Res < 0 ) {
                printf ( "I have problem with read\n" );
                return -1;
            }

            addr_m[0] = Read_Res;
            for ( i = 1; i <= Read_Res; ++i)
                addr_m[i] = Buff[i-1];

            if ( SEM_OP ( 0, 2, 0, sem_id, &Sem_Buf, 1 ) ) {
                printf ( "I have problem with sem\n" );
                return -1;
            }

            VALS = semctl ( sem_id, 2, GETVAL, 0 );
            if ( !VALS ) {
                printf ( "My freand is die, good bye\n" );
                break;
            }

            if ( SEM_OP ( 0, 0, 0,  sem_id, &Sem_Buf, 1 ) ) {
                printf ( "I have problem with sem\n" );
                return -1;
            }

            VALS = semctl ( sem_id, 2, GETVAL, 0 );
            if ( !VALS ) {
                printf ( "My freand is die, good bye\n" );
                break;
            }
        }

        break;
    case 'w':
        Sem_Buf[0].sem_num = 4;
        Sem_Buf[0].sem_op = 0;
        Sem_Buf[0].sem_flg = IPC_NOWAIT;

        Sem_Buf[1].sem_num = 4;
        Sem_Buf[1].sem_op = 1;
        Sem_Buf[1].sem_flg = SEM_UNDO;
        if ( semop ( sem_id, &Sem_Buf, 2 ) ) {
            printf ( "Writer already exist\n" );
            return -1;
        }

        SEM_OP ( 1, -1, 0, sem_id, &Sem_Buf, 1 );
        SEM_OP ( 0, 2, SEM_UNDO, sem_id, &Sem_Buf, 1 );
        SEM_OP ( 0, -2, 0, sem_id, &Sem_Buf, 1 );
        SEM_OP ( 2, 2, SEM_UNDO, sem_id, &Sem_Buf, 1 );

        Fd_Out = open ( argv[3], O_WRONLY | O_CREAT );
        if ( Fd_In < 0 ) {
            printf ( "I can't open output fule - %s\n", argv[3] );
            return -1;
        }

        while ( Read_Res ) {
            if ( SEM_OP ( 0, -1, 0 , sem_id, &Sem_Buf, 1 ) ) {
                printf ( "I have problem with sem\n" );
                return -1;
            }

            Read_Res = addr_m[0];
            write ( Fd_Out, &addr_m[1], Read_Res );

            if ( SEM_OP ( 0, -1, 0 , sem_id, &Sem_Buf, 1 ) ) {
                printf ( "I have problem with sem\n" );
                return -1;
            }

            VALS = semctl ( sem_id, 1, GETVAL, 0 );
            if ( !VALS ) {
                printf ( "My friend is die, good bye\n" );
                break;
            }
        }
        break;
    default :
        printf ( "I have problem. I don't know that means comand %s\n", argv[1] );
        return -1;
    }

    unlink ( argv[2] );
    shmdt ( (void*) addr_m );
    shmctl ( mem_id, IPC_RMID, NULL );
    //semctl ( sem_id, IPC_RMID, NULL );
    return 0;
}
