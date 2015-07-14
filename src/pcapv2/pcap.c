/*
 * Command line pcap logger
 *
 * Invokes the command on the command line, and logs everything to a timestamped
 * PCAP file.  If std_in is a socket, includes real information on the source socket
 * (IPv4 and IPv6 compatible).  Otherwise, fakes IPv4 from 1.1.1.1 to 2.2.2.2.
 *
 * When run on the command line (vs xinetd), will write the name of the pcap file
 * to /dev/tty.  This is safe, as long as we don't go out or our way to create
 * a tty for the binary.
 *
 * This binary makes the following environment variables available in the child's
 * address space, all of which are in string form.
 *
 *      - LOCAL_HOST
 *      - LOCAL_PORT
 *      - REMOTE_HOST
 *      - REMOTE_PORT
 */
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <pcap/pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>

#include "proxy.h"

//******************************************************************************
//                                PACKET HEADERS
//******************************************************************************
#pragma pack(push, 1)
typedef struct
{
    struct ip       ip;
    struct tcphdr   tcp;
    char            data[];
} packet4_t;

typedef struct
{
    struct ip6_hdr  ip;
    struct tcphdr   tcp;
    char            data[];
} packet6_t;

typedef struct
{
    union
    {
        packet4_t   p4;
        packet6_t   p6;
        char        __data[1024];
    };
} packet;
#pragma pack(pop)

packet  pkt;
char    *buffer;
const size_t buffer_size = sizeof(pkt.__data);

// PCAP objects
pcap_t  * pcap;
pcap_dumper_t *dumper;

//******************************************************************************
//                      IPV4 AND IPV6 COMPATIBLE SOCKADDR
//******************************************************************************

/**
 * Captures information about a proxied file descriptor pair, as well as information
 * about the "source" side of the connection.
 */
typedef struct _proxied_socket
{
    int     source;                 //!< Source file descriptor
    int     sink;                   //!< Sink file descriptor
    tcp_seq sequence;               //!< Sequence number
    union
    {
        struct sockaddr_in  sa4;    //!< IPv4 sockaddr for "source"
        struct sockaddr_in6 sa6;    //!< IPv6 sockaddr for "source"
        struct sockaddr     sa;     //!< Generic sockaddr for "source"
    };

    packet  packet;                 //!< Pre-filled packet, for data that comes in a particular direction
    char    *buffer;                //!< Pointer into packet where the data should be stored
    size_t  buffer_offset;          //!< Offset of our data buffer from the global buffer
} proxied_socket;

struct _proxied_socket  std_out;
struct _proxied_socket  std_in;

//******************************************************************************
//                                  PROTOTYPES
//******************************************************************************
int
record_and_forward
(
    proxied_socket  *in,
    proxied_socket  *out
);


void
build_and_save_packet
(
    char*           data,
    size_t          data_length,
    proxied_socket  *in,
    proxied_socket  *out,
    int             syn,
    int             ack
);


void
setup_environment
(
    char            *prefix,
    proxied_socket  *s
);


void
initialize_packet
(
    proxied_socket  *in,
    proxied_socket  *out
);


void
get_local_remote_info
(
);

//******************************************************************************
//                                SIGNAL HANDLER
//******************************************************************************
void
ctrl_c
(
    int dontcare
)
{
    std_out.sequence    = 0;
    dprintf(2, "Got Ctrl+C, exiting\n");
    dontcare            = 0;
    close(std_out.source);
    close(std_in.sink);
}



//******************************************************************************
//                               IMPLEMENTATIONS
//******************************************************************************

//------------------------------------ MAX -------------------------------------
static int
max
(
    int a,
    int b
)
{
    return (a > b) ? a : b;
}

int on_stdin(int fd,
                        void* ctx,
                        void**  buf,
                        size_t* buf_used,
                        size_t* buf_allocated_size)
{
    build_and_save_packet(*buf, *buf_used, &std_in, &std_out, 0, 1);
    return CB_OKAY;
}

//------------------------------------ MAIN ------------------------------------
int
main
(
    int     argc,
    char    ** argv
)
{
    if (argc < 3)
    {
        printf("usage: %s <file.pcap> argv0 argv1 ...", argv[0]);
        exit(1);
    }


    // Save off information about the std_in socket
    get_local_remote_info();

    // Set up environment variables so that they're available from the child process.
    setup_environment("LOCAL", &std_out);
    setup_environment("REMOTE", &std_in);

    // Create the child process, and initialize various file descriptors
    int child_pid = create_pipes_fork_and_exec(&argv[2]);

    // Clean up nicely on Ctrl+C
    signal(SIGINT, ctrl_c);

    //
    // Open up a pcap session file
    //
    char *dstfile = argv[1];
    char *tempfile = 0;

    asprintf(&tempfile, "%s/.%s", dirname(dstfile), basename(dstfile));

    pcap    = pcap_open_dead(DLT_RAW, 0x10000);
    dumper  = pcap_dump_open(pcap, tempfile);

    // Initialize packets
    initialize_packet(&std_out, &std_in);
    initialize_packet(&std_in, &std_out);


    //
    // Fake the three-way TCP handshake
    //
    std_in.sequence     = 0x1000;
    std_out.sequence    = 0x2000;
    build_and_save_packet(NULL, 0, &std_in, &std_out, 1, 0);      // SYN
    std_in.sequence++;
    build_and_save_packet(NULL, 0, &std_out,  &std_in, 1, 1);     // SYN+ACK
    std_out.sequence++;
    build_and_save_packet(NULL, 0, &std_in, &std_out, 0, 1);      // ACK

    register_io_callback(STDIN_FILENO, on_stdin, 0);
    register_io_callback(STDOUT_FILENO, on_stdout, 0);
    register_io_callback(STDERR_FILENO, on_stdout, 0);


    //
    // Main event loop.
    //
    // This continues until std_out on the child process gets closed.
    //
    while (1)
    {
        int     n;
        fd_set  fdset;

        FD_ZERO(&fdset);

        FD_SET(std_in.source, &fdset);
        FD_SET(std_out.source, &fdset);

        n = select(max(std_out.source, std_in.source) + 1, &fdset, 0, 0, 0);

        if (n <= 0)
        {
            break;
        }


        // Has new data arrived on std_in?
        if (  FD_ISSET(std_in.source, &fdset)
           && !record_and_forward(&std_in, &std_out))
        {
            // If std_in has closed/EOF, there may still be data avialable for
            // is to read from std_out.  Close std_in on the child, and continue
            // receiving data.  Generally this causes a SIGPIPE on the child and
            // it exits, but there may be e.g. buffered data to read out.
            close(std_in.sink);
        }

        // Has new data arrived on std_out?
        if (  FD_ISSET(std_out.source, &fdset)
           && !record_and_forward(&std_out, &std_in))
        {
            break;
        }
    }

    // Clean up
    pcap_dump_close(dumper);
    pcap_close(pcap);

    rename(tempfile, dstfile);
    free(tempfile);
}



//----------------------------- INITIALIZE PACKET ------------------------------
/**
 * To avoid having to copy around *all* of the fields for each little bit of
 * I/O, we initialize as much as possible in the forged packets.
 */
void
initialize_packet
(
    proxied_socket  *in,
    proxied_socket  *out
)
{
    struct tcphdr   *tcp;
    packet          * P = &in->packet;

    if (in->sa.sa_family == AF_INET)
    {
        P->p4.ip.ip_hl  = sizeof(P->p4.ip) / 4;
        P->p4.ip.ip_v   = 4;
        P->p4.ip.ip_p   = IPPROTO_TCP;
        P->p4.ip.ip_ttl = 5; // Must be 5+, or Wireshark colors red!

        memcpy(&P->p4.ip.ip_src, &in->sa4.sin_addr.s_addr, 4);
        memcpy(&P->p4.ip.ip_dst, &out->sa4.sin_addr.s_addr, 4);

        tcp             = &P->p4.tcp;
        tcp->th_sport   = in->sa4.sin_port;
        tcp->th_dport   = out->sa4.sin_port;
    }
    else // AF_INET6
    {
        P->p6.ip.ip6_vfc    = 6;
        P->p6.ip.ip6_nxt    = IPPROTO_TCP;

        memcpy(&P->p6.ip.ip6_src, &in->sa6.sin6_addr, sizeof(P->p6.ip.ip6_src));
        memcpy(&P->p6.ip.ip6_dst, &out->sa6.sin6_addr, sizeof(P->p6.ip.ip6_dst));

        tcp                 = &P->p6.tcp;
        tcp->th_sport       = in->sa6.sin6_port;
        tcp->th_dport       = out->sa6.sin6_port;
    }

    tcp->th_off = sizeof(*tcp) / 4;
    tcp->th_win = htons(2048);

    in->buffer  = (char *)(tcp + 1);
    in->buffer_offset = buffer - in->buffer;
}



//--------------------------- BUILD_AND_SAVE_PACKET ----------------------------
/**
 * We have received some data over the wire.  It is already loaded inside of the
 * appropriate packet structure, at the appropriate offset.
 * Fix up the length and sequence fields as required, and save the packet to
 * the pcap file.
 */
void
build_and_save_packet
(
    char            *data,
    size_t          data_length,
    proxied_socket  *in,
    proxied_socket  *out,
    int             syn,
    int             ack
)
{
    size_t  header_size = 0;
    struct tcphdr *tcp  = NULL;
    packet  * P         = &in->packet;




    size_t used = *buf_used;
    char*  buffer = *buf;
    while(used > std_in.buffer_size) {
        memcpy(std_in.buffer, buffer, std_in.buffer_size);
        build_and_save_packet(std_in.buffer_size, &std_in, &std_out, 0, 1);
        used -= std_in.buffer_size;
    }






    // Reallocate the buffer to be large enough
    if(length > buffer_size) {
        ptrdiff_t offset = in->buffer - buffer;

        buffer_size = length * 2;
        buffer = realloc(buffer, buffer_size);

        if(!buffer) {
            puts();
            exit(1);
        }

        in->buffer = buffer + offset;
        out->buffer = buffer + offset;
    }

    if (in->sa.sa_family == AF_INET)
    {
        tcp             = &P->p4.tcp;
        P->p4.ip.ip_len = htons(sizeof(P->p4) + length);
        header_size     = sizeof(P->p4);
    }
    else // AF_INET6
    {
        tcp = &P->p6.tcp;
        P->p6.ip.ip6_plen = htons(sizeof(*tcp) + length);
        header_size = sizeof(P->p6);
    }

    tcp->th_seq     = htonl(in->sequence);
    tcp->th_ack     = htonl(out->sequence);
    tcp->th_flags   |= syn ? TH_SYN : 0;
    tcp->th_flags   |= ack ? TH_ACK : 0;

    in->sequence    += length;

    struct pcap_pkthdr pcap_hdr;
    gettimeofday(&pcap_hdr.ts, 0);
    pcap_hdr.caplen = length + header_size;
    pcap_hdr.len    = pcap_hdr.caplen;

    pcap_dump((uint8_t *) dumper, &pcap_hdr, (uint8_t *) P);
    pcap_dump_flush(dumper);
}



//----------------------------- RECORD_AND_FORWARD -----------------------------
/**
 * Receive data from the source, record it, send it to the sink.
 *
 * Returns 0 if an error occurs.
 */
int
record_and_forward
(
    proxied_socket  *in,
    proxied_socket  *out
)
{
    ssize_t bwrite  = 0;
    ssize_t bread   = read(in->source, in->buffer, buffer_size);

    if (bread <= 0)
    {
        return 0;
    }

    build_and_save_packet(bread, in, out, 0, 1);

    while (bwrite < bread)
    {
        ssize_t n = write(in->sink, &in->buffer[bwrite], bread - bwrite);

        if (n < 0)
        {
            return 0;
        }

        bwrite += n;
    }
    return 1;
}



//----------------------------- SETUP_ENVIRONMENT ------------------------------
/**
 * Creates several environment variables for consumption by other tools in
 * the toolchain.
 */
void
setup_environment
(
    char            *prefix,
    proxied_socket  *sa
)
{
    char    hostname[INET6_ADDRSTRLEN];
    char    *env_host, *env_port, *port_str;
    void    *addr = 0;

    switch (sa->sa.sa_family)
    {
     case AF_INET:
         addr   = &sa->sa4.sin_addr;
         break;
     case AF_INET6:
         addr   = &sa->sa6.sin6_addr;
         break;
    }


    asprintf(&env_host, "%s_HOST", prefix);
    asprintf(&env_port, "%s_PORT", prefix);
    asprintf(&port_str, "%d", ntohs(sa->sa4.sin_port));

    inet_ntop(AF_INET, addr, hostname, INET6_ADDRSTRLEN);

    setenv(env_host, hostname, 1);
    setenv(env_port, port_str, 1);

    free(env_host);
    free(env_port);
    free(port_str);
}



//--------------------------- GET LOCAL REMOTE INFO ----------------------------
/**
 * Just a wrapper around getsockname / getpeername, and to default to fake
 * IP and ports if we don't get sane values.
 */
void
get_local_remote_info
(
)
{
    socklen_t size = sizeof(std_out);

    if (  -1 == getsockname(0, &std_out.sa, &size)
       || (  std_out.sa.sa_family != AF_INET
          && std_out.sa.sa_family != AF_INET6))
    {
        std_out.sa4.sin_port        = htons(0);
        std_out.sa4.sin_family      = AF_INET;
        std_out.sa4.sin_addr.s_addr = inet_addr("1.1.1.1");
    }

    size = sizeof(std_in);

    if (  -1 == getpeername(0, &std_in.sa, &size)
       || (  std_in.sa.sa_family != AF_INET
          && std_in.sa.sa_family != AF_INET6))
    {
        std_in.sa4.sin_port         = htons(0);
        std_in.sa4.sin_family       = AF_INET;
        std_in.sa4.sin_addr.s_addr  = inet_addr("0.0.0.0");
    }
}
