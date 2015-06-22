#include "skynet.h"

#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <stdio.h>
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
Skynet*
Skynet_new( double cutoff )
{
    Skynet* self;
    self = malloc(sizeof(Skynet));
    if( self==NULL )
        return NULL;
    memset( self, 0, sizeof(*self) );
    self->cutoff        = cutoff;
    self->configDir     = "./";
    return self;
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void
Skynet_delete( Skynet* self )
{
    if( self )
        free(self);
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
static int
Skynet_savePacket( Skynet* self, char buf[], size_t nbytes )
{
    struct timeval  timeVal;
    char            filePath [PATH_MAX];
    int             ret;

    gettimeofday( &timeVal, NULL );
    snprintf( 
        filePath, sizeof(filePath), 
        "%s/%ld_%ld_%ld.sn", 
        self->configDir,
        timeVal.tv_usec,
        self->packetNumber,
        nbytes
    );

    int fd = open( filePath, O_CLOEXEC | O_CREAT | O_WRONLY ,  S_IRUSR);
    if( fd<0 )
        return -1;

    ret = write( fd, buf, nbytes );
    close(fd);
    return ret;

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
int
Skynet_processPacket( Skynet* self, int fd, char buf[], size_t nbytes )
{
    self->packetNumber++;

    if( self->cutoff>0.0 )
    //// Armed mode ///////////////////////////////
    {

    }
    else 
    //// Training mode ////////////////////////////
    {
        Skynet_savePacket( self, buf, nbytes );

    }
    return 0;
}





#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

/*
int 
damlev2( char* a, size_t a_d,  char* b, size_t b_d )
{
    int d[a_d][b_d]; 


    int i;
    int j;
    int cost = 0;

    for( i=0; i<a_d; i++ )
    {
        d[i][0] = i;
    }
    for( j=1; j<b_d; j++ )
    {
        d[0][j] = j;
    }

    for( i=0; i<a_d; i++)
    {
        for( j=0; j<b_d; j++)
        {
            if( a[i] == b[j] )
                cost = 0;
            else
                cost = 1;
            d[i][j] = MIN3(
                d[i-1][j  ] + 1,    // deletion
                d[i  ][j-1] + 1,    // insertion
                d[i-1][j-1] + cost  // substitution
            );
            if( i>0 && j>0 && a[i]==b[j-1] && a[i-1]==b[j] )
            {
                d[i][j] = MIN2( d[i][j], d[i-2][j-2]+cost); // transposition
            }
        }
    }
    return d[a_d][b_d];
}
*/

 
int levenshtein(char *a, size_t a_len, char *b, size_t b_len ) {
    size_t x, y, lastdiag, olddiag;
    a_len = strlen(a);
    b_len = strlen(b);
    unsigned int column[a_len+1];
    for (y = 1; y <= a_len; y++)
        column[y] = y;
    for (x = 1; x <= b_len; x++) {
        column[0] = x;
        for (y = 1, lastdiag = x-1; y <= a_len; y++) {
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y-1] + 1, lastdiag + (a[y-1] == b[x-1] ? 0 : 1));
            lastdiag = olddiag;
        }
    }
    return(column[a_len]);
}

void
damlev_test()
{
    typedef struct 
    {
        char*   a;
        char*   b;
        int     ld;
    } Pair;

    Pair pairs[] = 
    {
        { "aa", "aa", 0 },
        { "aa", "ba", 1 },
        { "abcd", "Xabcd", 1 }, 
        { "abcd", "ABCD", 4 },
        { NULL, NULL, 0 }
    };

    for( int i=0; pairs[i].a!=NULL; i++)
    {
        char* a = pairs[i].a;
        char* b = pairs[i].b;
        int ld = levenshtein(a, strlen(a),  b, strlen(b) );
        printf("%d, %d: %s, %s\n", pairs[i].ld, ld, a, b );
    }
  


}
