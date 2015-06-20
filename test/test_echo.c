////////////////////////////////////////////////////////////////////////////
// This is just a simple test service for gatekeeper
//
////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#define EOT     0x4

int main( int argc, char* argv[] )
{

    unsigned char   packet[1500];
    struct termios  termios_old;
    struct termios  termios_new;
    int             ch;

    tcgetattr( 0, &termios_old );
    termios_new = termios_old;
    termios_new.c_lflag &= ( ~ICANON & ~ECHO );
    tcsetattr( 0, TCSANOW, &termios_new );

    do
    {
        ch = getc(stdin);
        if( ch == EOT )
        {
            break;
        }
        //ch ^= 0x20;
        fprintf( stdout, "(%x)", ch);
    } while (1);

    tcsetattr( 0, TCSANOW, &termios_old );

    return 0;
}