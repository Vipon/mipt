#include <stdio.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

struct w_buff
{
    int w_fd;
    int r_fd;
    int size;
    char *buff;
    int l_tran;
};

void shft_left ( char *buff, int start, int end )
{
    int i = 0;
    for ( i = 0; i < ( end - start ); ++i)
        buff[i] = buff[i+start];
}

int max_fd ( int fd1, int fd2 )
{
    if ( fd1 > fd2 )
        return fd1;
    else
        return fd2;
}

int main ( int argc, char *argv[] )
{
    if ( argc != 4 ) {
        printf ( "ERROR: you forgot to enter the number\n" );
        return -1;
    }

    fd_set rd_set, wr_set;
    int r_fd, w_fd;
    int buff_size;
    int pipefd1[2];
    int pipefd2[2];
    int status;
    int length;
    int n = atoi ( argv[1] );
    int i = 0;
    int fd_max = 0;

    struct w_buff *buf =  ( struct w_buff* ) malloc ( sizeof ( struct w_buff ) * n );
    if ( buf <= 0 ) {
        printf ( "ERROR: can not allocate memory\n" );
        return -1;
    }
    for ( i = 0; i < n; ++i ) { /* preparation */
        buf[i].size = (n-i) * 3000;
        buf[i].buff = ( char* ) malloc ( buf[i].size );
        buf[i].l_tran = 0;

        pipe ( pipefd1 );
        pipe ( pipefd2 );
        buff_size = buf[i].size;
        status = fork();

        if ( status ) { //parent
            close ( pipefd1[0] );
            close ( pipefd2[1] );
            buf[i].w_fd = pipefd1[1];
            buf[i + 1].r_fd = pipefd2[0];

            if ( fd_max < max_fd ( pipefd1[1], pipefd2[0] ) )
                fd_max = max_fd ( pipefd1[1], pipefd2[0] );

            fcntl ( buf[i+1].r_fd, F_SETFL , O_NONBLOCK );
            fcntl ( buf[i].w_fd, F_SETFL , O_NONBLOCK );
        } else { //child
            close( pipefd1[1] );
            close( pipefd2[0] );
            if ( i == 0 ) {
                close ( pipefd1[0] );
                printf ( "open %s for read\n", argv[2] );
                r_fd = open ( argv[2], O_RDONLY );
            } else
                r_fd = pipefd1[0];

            if ( i == (n-1) ) {
                close ( pipefd2[1] );
                printf ( "open %s for write\n", argv[3] );
                w_fd = open ( argv[3],  O_WRONLY | O_CREAT , 0644 );
            }
            else
                w_fd = pipefd2[1];

            int j;
            for( j = 0; j < i; j++) {
                close ( buf[j].w_fd );
                close ( buf[j + 1].r_fd );
            }

            break;
        }
    }

    //transmission
    if ( status ) { //parent
        while ( status ) {
            FD_ZERO ( &rd_set );
            FD_ZERO ( &wr_set );
            status = 0;

            for( i = 0 ; i <= n ; i ++) {
                if ( buf[i].r_fd && ( buf[i].size != buf[i].l_tran ) ) {
                    FD_SET( buf[i].r_fd, &rd_set );
                    ++status;
                }

                if ( buf[i].w_fd && buf[i].l_tran ) {
                    FD_SET( buf[i].w_fd, &wr_set );
                    ++status;
                }
            }

            if (status == 0)
                break;

            status = select( fd_max + 1, &rd_set, &wr_set, NULL, NULL );
            if ( status < 1) break;

            for( i = 0 ; i <= n ; i ++)
                if ( FD_ISSET ( buf[i].r_fd, &rd_set ) ) { // reading
                    length = read ( buf[i].r_fd, buf[i].buff + buf[i].l_tran, buf[i].size - buf[i].l_tran );

                    if ( length > 0 )
                        buf[i].l_tran += length;

                    if ( length < 1 ) {
                        close ( buf[i].r_fd );
                        buf[i].r_fd = 0;
                    }
                }

            for( i = 0 ; i <= n ; i ++)
                if ( FD_ISSET ( buf[i].w_fd, &wr_set ) ) { //writing
                    length = write ( buf[i].w_fd, buf[i].buff, buf[i].l_tran );
                    if ( length > 0 ) {
                        shft_left ( buf[i].buff, length, buf[i].l_tran );
                        buf[i].l_tran -= length;
                    }

                    if ( ( length < 1 ) && ( buf[i].r_fd == 0 ) && ( buf[i].l_tran == 0 ) ) {
                        close ( buf[i].w_fd );
                        buf[i].w_fd = 0;
                    }
                }

            for( i = 0 ; i <= n ; i ++)
                if ( buf[i].w_fd && ( buf[i].r_fd == 0) && ( buf[i].l_tran == 0 ) ) {
                    close ( buf[i].w_fd );
                    buf[i].w_fd = 0;
                }
        }

    } else { //child
        char buff[buff_size];
        length = 1;
        while( length > 0 ) {
            length = read ( r_fd, buff, buff_size );
            write ( w_fd, buff, length );
        }

        close( r_fd );
        close( w_fd );
    }

    return 0;
}
