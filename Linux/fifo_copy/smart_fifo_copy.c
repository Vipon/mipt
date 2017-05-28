#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO "Fifo"
#define LEN_NUM 7
#define MAXLINE 256

int main ( int argc, char *argv[] )
{
    int Fifo = 0;
    int U_Fifo = 0;
    int Write_Fd = 0;
    int Read_Fd = 0;
    int result_read = 1;
    int result_write = 1;
    char Buff[MAXLINE] = {};
    char My_Fifo[LEN_NUM] = {};
    int length = 0;
    pid_t Work_Pid = 0;

    switch ( argv[1][0] ) {
    case 'r':	//open output File and check
        Write_Fd = open ( argv[2], O_RDWR | O_CREAT, 0644 );
        if ( Write_Fd <= 0 ) {
            printf ( "ERROR, Don't open output File!\n" );
            return -1;
        }

        if ( ( mkfifo ( FIFO, 0644 ) < 0 ) && ( errno != EEXIST ) ) {
            close ( Read_Fd );
            printf ( "can't create %s\n", FIFO );
            return -1;
        }

        //open Fifo for Read and ckeck
        Fifo = open ( FIFO, O_RDONLY );
        while ( length != LEN_NUM ) {
            length = read ( Fifo, &My_Fifo, LEN_NUM );
            if ( length < 0 ) {
                printf ( "I don't read from %s", FIFO );
                return -1;
            }
        }

        U_Fifo = open ( My_Fifo, O_WRONLY | O_NDELAY );
        if ( U_Fifo >  0) {
            return -1;// if this My_Fifo has already had reader;
        }
        //sleep (10);
        U_Fifo = open ( My_Fifo, O_RDONLY | O_NDELAY );
        sleep (2);
        close ( Fifo );

        while (	result_read ) {
            result_read = read ( U_Fifo, &Buff, MAXLINE );
            if ( result_read > 0 )
                write ( Write_Fd, &Buff, result_read );

            if ( !result_read )
                printf ( "Disconnect to server\n" );
        }

        close ( Write_Fd );
        close ( U_Fifo );
        unlink ( My_Fifo );
        break;
    case 'w':	//open input File and Check
        Read_Fd = open ( argv[2], O_RDONLY );
        if ( Read_Fd <= 0 ) {
            printf ( "ERROR, Don't open input File!\n" );
            return -1;
        }

        //creat FIFO and Check
        if ( ( mkfifo ( FIFO, 0644 ) < 0 ) && ( errno != EEXIST ) ) {
            close ( Read_Fd );
            printf ( "can't create %s\n", FIFO );
            return -1;
        }

        //open Fifo for write and Check
        Fifo = open ( FIFO, O_WRONLY );
        if ( Fifo <= 0 ) {
            close ( Read_Fd );
            printf ( "Fifo ERROR\n" );
            return -1;
        }

        //getpid - return pid our process, we use it for create uniqeu fifo
        Work_Pid = getpid ();
        length = sprintf ( My_Fifo, "%d", Work_Pid );
        if ( length < 0 )
            return -1;

        for ( ; length < LEN_NUM; ++length )
            My_Fifo[length] = 'a';

        //create uniqeu fifo for exchange infomation
        if ( ( mkfifo ( My_Fifo, 0644 ) < 0 ) && ( errno != EEXIST ) ) {
            close ( Read_Fd );
            printf ( "can't create %s\n", My_Fifo );
            return -1;
        }

        write ( Fifo, &My_Fifo, LEN_NUM );

        sleep (1);
        U_Fifo = open ( My_Fifo, O_WRONLY | O_NDELAY );
        if ( U_Fifo < 0 )
            return -1;

        //sleep (10);
        close ( Fifo );
        int flags = fcntl(U_Fifo, F_GETFL, 0);
        flags &= ~O_NONBLOCK;
        fcntl(U_Fifo, F_SETFL, flags);

        while ( result_write ) {
            if ( result_write > 0 )
                result_read = read ( Read_Fd, &Buff, MAXLINE );

            result_write = write ( U_Fifo, &Buff, result_read );
        }

        close ( Read_Fd );
        while ( ( write ( U_Fifo, '\0', 1 ) ) && ( errno != EBADF ) )
            close ( U_Fifo );
        break;
    default :
        printf ( "I don't know that I must to do, maybe it's you wrong \'%c\'\n", argv[1][0] );
        return -1;
        break;
    }

    return 0;
}
