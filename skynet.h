#include <stdlib.h>


typedef struct _Skynet
{
    double      cutoff;
    size_t      packetNumber;
    char*       configDir;
} Skynet;


Skynet*
Skynet_new( double cutoff );

void
Skynet_delete( Skynet* self );

int
Skynet_processPacket( Skynet* self, int fd, char buf[], size_t nbytes );
