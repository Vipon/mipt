#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>


char buff_m = 0;
int count_m = 0;
int sig_m = 0;
int ex_m = 0;
int res_send = 0;

void one ()
{
    buff_m += ( 1 << count_m );
    ++count_m;
    ++sig_m;
}

void zero ()
{
    ++count_m;
    ++sig_m;
}

void f_ex ()
{
    ++ex_m;
}

void al ()
{
    if ( res_send == -1 ) {
        printf ( "My parent is died, good byu violent world\n" );
        exit ( EXIT_FAILURE );
    }
}

void up ()
{
}

int main ( int argc, char* argv[] )
{
    int bit = 0;
    pid_t pid = 0;
    pid_t p_pid = getpid ();
    int In_Fd = 0;
    sigset_t set;
    sigemptyset ( &set );

    struct sigaction act_1, act_0, act_ex, act_al;
    memset ( &act_1, 0, sizeof ( act_1 ) );
    memset ( &act_al, 0, sizeof ( act_al ) );
    memset ( &act_0, 0, sizeof ( act_0 ) );
    memset ( &act_ex, 0, sizeof ( act_ex ) );
    act_1.sa_handler = one;
    act_0.sa_handler = zero;
    act_ex.sa_handler = f_ex;
    act_al.sa_handler = up;

    sigfillset ( &act_1.sa_mask );
    sigfillset ( &act_al.sa_mask );
    sigfillset ( &act_0.sa_mask );
    sigfillset ( &act_ex.sa_mask );

    sigaction ( SIGALRM, &act_al, NULL );
    sigaction ( SIGUSR1, &act_1, NULL );
    sigaction ( SIGUSR2, &act_0, NULL );
    sigaction ( SIGCHLD, &act_ex, NULL );

    sigaddset ( &set, SIGALRM );
    sigaddset ( &set, SIGUSR1 );
    sigaddset ( &set, SIGUSR2 );
    sigaddset ( &set, SIGCHLD );
    sigprocmask ( SIG_BLOCK, &set, NULL );


    pid = fork ();
    sigprocmask ( SIG_UNBLOCK, &set, NULL );
    sigemptyset ( &set );
    if ( !pid ) { /* child */
        struct sigaction act_up, act_alarm;
        memset ( &act_up, 0, sizeof ( act_up ) );
        memset ( &act_alarm, 0, sizeof ( act_alarm ) );
        act_up.sa_handler = up;
        act_alarm.sa_handler = al;
        sigfillset ( &act_up.sa_mask );
        sigfillset ( &act_alarm.sa_mask );
        sigaction ( SIGUSR1, &act_up, NULL );
        sigaction ( SIGALRM, &act_alarm, NULL );

        In_Fd = open ( argv[1], O_RDWR );
        if ( In_Fd <= 0 ) {
            printf ( "I can't open Input File\n" );
            return -1;
        }

        int i = 0;
        while ( (read ( In_Fd, &buff_m, 1 ) > 0) && ( buff_m != EOF ) )
            for ( i = 0; i < 8; ++i ) {
                alarm(10);
                bit = ( buff_m & ( 1 << i ) );
                if ( bit ) {
                    res_send = kill ( p_pid, SIGUSR1 );
                } else {
                    res_send = kill ( p_pid, SIGUSR2 );
                }

                sigsuspend ( &set );
            }

    } else {/* parent */
        //int r = open ( "1.txt", O_RDWR );
        sigsuspend ( &set );
        while ( ex_m != 1 ) {
            if ( ( sig_m ) && ( count_m != 8 ) ) {
                --sig_m;
                kill ( pid, SIGUSR1 );
                sigsuspend ( &set );
            }

            if ( count_m == 8 ) {
                count_m = 0;
                write ( 1, &buff_m, 1 );
                buff_m = 0;
            }
        }

        // wait ( NULL );
    }

    return 0;
}
