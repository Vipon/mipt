#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

struct msg
{
    long mtype;
    long num;
};

int main ( int argc, char *argv[] )
{
    int i = 0;
    int n = atoi ( argv[1] );
    int pid = 0;
    int msgid = 0;
    int num_my = 0;
    int stat = 0;
    struct msg msg1 = { sizeof ( long ), 0 };
    struct msg msg2 = { sizeof ( long ), 0 };

    msgid = msgget ( IPC_PRIVATE, 0664 );
    if ( msgid < 0 ) {
        printf ( "EEROR!\n" );
        return -1;
    }

    for ( i = 1; i <= n; ++i ) {
        pid = fork ();
        if ( pid < 0 ) {
            printf ( "Fail #%d fork ()\n", i );
            return -1;
        }

        if ( !pid ) {
            num_my = i;
            break;
        }
    }

    if ( !pid ) {
        if ( num_my == 1) {
            printf ( "#%d", num_my, getpid() );
            fflush ( stdout );

            msg1.num = 2;
            msg1.mtype = 2;
            msgsnd ( msgid, &msg1, ( sizeof( struct msg ) - sizeof ( long ) ), 0 );
        } else {
            msgrcv ( msgid, &msg2, ( sizeof ( struct msg ) - sizeof ( long ) ), num_my, 0 );

            printf ( "#%d  ", num_my );
            fflush ( stdout );
            msg1.num = num_my + 1;
            msg1.mtype = num_my + 1;
            msgsnd ( msgid, &msg1, ( sizeof( struct msg ) - sizeof ( long ) ), 0 );
            return 0;
        }
    } else {
        msgrcv ( msgid, &msg2, ( sizeof ( struct msg ) - sizeof ( long ) ), n+1, 0 );
        putchar ( '\n' );
    }

    return 0;
}

