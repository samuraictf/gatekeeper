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
    int             c;
    int             mode_buffer = -1;
    int             mode_echo = 0;
    int             mode_flush = 0;

    while ((c = getopt(argc, argv, "uelfz")) != -1) 
    {
        switch(c)
        {
            case 'u':   // unbuffered
                mode_buffer = _IONBF;
                break;

            case 'l':   // line buffered
                mode_buffer = _IOLBF;
                break;

            case 'f':
                mode_buffer = _IOFBF;
                break;

            case 'e':
                mode_echo = 1;
                break;

            case 'z':
                mode_flush = 1;
                break;


        }
    }

    if( mode_buffer != -1 )
    {
        setvbuf( stdin, NULL, mode_buffer, 0 );
        setvbuf( stdout, NULL, mode_buffer, 0 );
    }

    printf("{START}: %d\n", mode_buffer);
    fflush(stdout);
    
    if( 0==mode_echo )
    {
        printf("Disabling echo\n");
        fflush(stdout);
        tcgetattr( 0, &termios_old );
        termios_new = termios_old;
        termios_new.c_lflag &= ( ~ICANON & ~ECHO );
        //termios_new.c_lflag &=  ~ECHO ;

        tcsetattr( 0, TCSANOW, &termios_new );
    }
        


    do
    {
        ch = getc(stdin);
        if( ch == EOT || ch==EOF || ch==-1 )
        {
            break;
        }
        //ch ^= 0x20;
        fprintf( stdout, "(%x)", ch);
        if( mode_flush )
            fflush(stdout);
    } while (1);

    tcsetattr( 0, TCSANOW, &termios_old );
    printf("{STOP}\n");

    return 0;
}