#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO1 "Fifo"
#define MAXLINE 1024

int main ( int argc, char *argv[] )
{
    int Fifo = 0;
    int Write_Fd = 0;
    int Read_Fd = 0;
    char buff = 0;

    switch ( argv[1][0] ) {
    case 'r':	// open output File and check
        Write_Fd = open ( argv[2], O_RDWR | O_CREAT, 0644 );
        if ( !Write_Fd ) {
            printf ( "ERROR, Don't open output File!\n" );
            return -1;
        }

        // open Fifo for Read and ckeck
        while ( Fifo <= 0 ) {
            Fifo = open ( FIFO1, O_RDONLY );
            if ( Fifo < 0 ) {
                printf ( "You Don't open write chanel\n" );
                printf ( "Open write chanel and push 'y' or 'n' if you want to exit\n" );

                buff = getchar ();
                switch ( buff ) {
                case 'y': case 'Y':
                    break;
                case 'n': case 'N':
                    close ( Write_Fd );
                    printf ( "Fifo ERROR\n" );
                    return -1;
                    break;
                }
            }
        }

        while (	read ( Fifo, &buff, 1 ) > 0 )
            write ( Write_Fd, &buff, 1 );

        close ( Write_Fd );
        break;

    case 'w':	//open input File and Check
        Read_Fd = open ( argv[2], O_RDONLY );
        if ( !Read_Fd ) {
            printf ( "ERROR, Don't open input File!\n" );
            return -1;
        }

        //creat FIFO and Check
        if ( ( mkfifo ( FIFO1, 0644 ) < 0 ) && ( errno != EEXIST ) ) {
            close ( Read_Fd );
            printf ( "can't create %s\n", FIFO1 );
            return -1;
        }

        while ( errno == EEXIST ) {
            printf ( "Fifo is using, please wait, and enter 'y'\n" );
            buff = getchar ();
            if ( (buff == 'y') || (buff == 'Y') ) {
                unlink ( FIFO1 );
                if ( ( mkfifo ( FIFO1, 0644 ) < 0 ) && ( errno != EEXIST ) ) {
                    close ( Read_Fd );
                    printf ( "can't create %s\n", FIFO1 );
                    return -1;
                }
            }
        }

        //open Fifo for write and Check
        Fifo = open ( FIFO1, O_WRONLY );
        if ( !Fifo ) {
            close ( Read_Fd );
            printf ( "Fifo ERROR\n" );
            return -1;
        }

        while ( read ( Read_Fd, &buff, 1 ) > 0 )
            write ( Fifo, &buff, 1 );

        close ( Read_Fd );
        close ( Fifo );
        break;

    default :
        break;
    }

    unlink ( FIFO1 );
    return 0;
}
