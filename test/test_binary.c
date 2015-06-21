////////////////////////////////////////////////////////////////////////////
// This is just a simple test service for gatekeeper
//
////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>


#define EOT     0x4

int main( int argc, char* argv[] )
{

    struct termios termios_old;
    struct termios termios_new;

    tcgetattr( 0, &termios_old );
    termios_new = termios_old;
    termios_new.c_lflag &= ~ICANON; 
    termios_new.c_lflag &= ~ECHO ;
    tcsetattr( 0, TCSANOW, &termios_new );


    unsigned int    seed;
    int             fd;
    ssize_t         nbytes;


    fd = open( "/dev/urandom", O_RDONLY, NULL );
    if( -1 == fd )
    {
        perror("unable to open /dev/urandom");
        return -1;
    }

    nbytes = read( fd, &seed, sizeof(seed) );
    if( 4 != nbytes )
    {
        perror("Unable to get seed from /dev/urandom.");
        return -2;
    }

    srand( seed );
    close(fd);

    printf("{START}\n");

    for( int i=0; i<100; i++ )
    {
        uint32_t    mask;
        uint32_t    value;
        uint32_t    value_read;
        ssize_t     nbytes;

        value = rand();
        mask = rand();

        write( 1, &value, sizeof(uint32_t) );
        //write( 1, &mask,  sizeof(uint32_t) );
        read( 0, &value_read, sizeof(value_read) );
        if( value_read==value )
        {
            printf("WIN!!!\n`");
        }
        else
        {
            printf("not win\n");
        }
    }

    printf("{STOP}\n");

    return 0;
}