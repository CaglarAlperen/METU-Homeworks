#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define BUF_SIZE 500
#define RECV_PORT "5201"
#define WINDOW_SIZE 16
#define TIMEOUT 3

struct packet
{
    unsigned short checksum;
    unsigned short seq_num;
    unsigned short ack; // 1 is ACK, 0 is NAK, -1 is it is data
    unsigned short end;
    char data[8];
};

#pragma region Globals
char *host, *port;
int send_sfd, recv_sfd;
int send_base, recv_base, next_seqnum, expected_seqnum;
char inp_buf[BUF_SIZE], rcv_buf[BUF_SIZE];
int rcv_buf_indx;
struct packet *send_pkt[WINDOW_SIZE];
int ack_packet[WINDOW_SIZE];
struct packet *recv_pkt[WINDOW_SIZE];
clock_t timers[WINDOW_SIZE];
#pragma endregion Globals

#pragma region PacketDeclarations
void rdt_send(char *data, int ack);
void rdt_recv(struct packet *packet);
int is_notcorrupt(struct packet *packet);
int is_packet_recieved(struct packet *packet);
struct packet *make_packet(char *data, int ack);
char *extract_data(struct packet* packet);
unsigned short checksum(char *data, int count);
void flush_msg();
void print_packet(struct packet *pkt);
#pragma endregion PacketDeclarations

#pragma region PacketDefinitions

void rdt_sendpkt(struct packet *pkt)
{   
    send(send_sfd, (void *)pkt, sizeof(struct packet), 0);
}

void rdt_send(char *data, int ack)
{
    struct packet *pkt;

    printf("RDT_SEND\n");
    if (ack == 0) // if it is a data packet
    {
        int len = strlen(data);
        int indx = 0;

        printf("next_seqnum: %d, base: %d, max: %d\n", next_seqnum, send_base, send_base + WINDOW_SIZE);

        while (len > 0)
        {
            if (next_seqnum < send_base + WINDOW_SIZE)
            {
                char pkt_data[8];
                int i = 0;
                unsigned short end = 0; // to check if it's final package of a message
                do 
                {
                    if (data[indx] == '\n')
                        end = 1;
                    pkt_data[i++] = data[indx++];
                } while(i < 8);

                pkt = make_packet(pkt_data, ack);
                pkt->end = end;
                send_pkt[next_seqnum % WINDOW_SIZE] = pkt; // add packet to packet buffer

                rdt_sendpkt(pkt);
                print_packet(pkt);
                if (send_base == next_seqnum)
                    timers[next_seqnum % WINDOW_SIZE] = clock();
                next_seqnum++;
                len -= 8;
            }
        }
    }
    else // if it is an ACK packet
    {
        char pkt_data[8];
        for (int i = 0; i < 8; i++)
        {
            pkt_data[i] = ack;
        }
        pkt = make_packet(pkt_data, ack);
        rdt_sendpkt(pkt);
    }
}

void rdt_recv(struct packet *packet)
{
    printf("RDT_RECV\n");
    if (packet->ack == 0) // if it is a data packet
    {
        printf("Recieved data packet\n");
        if (is_notcorrupt(packet) && packet->seq_num >= recv_base
            && packet->seq_num < recv_base + WINDOW_SIZE)
        {
            printf("Packet recieved:\n");
            print_packet(packet);

            /* if this packet is not recieved early */
            if (!is_packet_recieved(packet))
            {
                printf("Not recieved early\n");
                recv_pkt[(packet->seq_num) % WINDOW_SIZE] = packet; // add packet to packet buffer

                rdt_send("ACK", 1); // send ACK packet
                expected_seqnum++;

                printf("Buffered without seg fault\n");
                if (packet->seq_num == recv_base)
                {
                    /* foreach sequentially buffered packets in recv_base */
                    for (int i = recv_base; i < recv_base+WINDOW_SIZE; i++)
                    {
                        printf("revc_base: %d, index: %d, max: %d\n", recv_base, i%WINDOW_SIZE, recv_base+WINDOW_SIZE);
                        if (recv_pkt[i % WINDOW_SIZE] != NULL)
                        {
                            for (int j = 0; j < 8; j++)
                            {
                                // put packet data to mesage buffer
                                rcv_buf[rcv_buf_indx++] = recv_pkt[i%WINDOW_SIZE]->data[j];
                            }
                            recv_base++;
                            if (recv_pkt[i%WINDOW_SIZE]->end)
                                flush_msg();
                            recv_pkt[i%WINDOW_SIZE] = NULL;
                        }
                        else {
                            break;
                        }
                    }
                }
            } 
        }
        else if (is_notcorrupt(packet) && packet->seq_num < recv_base)
        {
            struct packet *pkt;
            pkt = make_packet("ACK", 1);
            pkt->seq_num = packet->seq_num;
            rdt_sendpkt(pkt);
        }
    }
    else // if it is a ack packet
    {
        if (is_notcorrupt(packet) && packet->ack == 1)
        {
            printf("Recieved ack packet\n");
            ack_packet[packet->seq_num % WINDOW_SIZE] = 1; // mark as acknowledged
            if (packet->seq_num == send_base)
            {
                for (int i = send_base; i < send_base + WINDOW_SIZE; i++)
                {
                    if (ack_packet[i % WINDOW_SIZE] == 1)
                    {
                        send_pkt[i % WINDOW_SIZE] = NULL;
                        ack_packet[i % WINDOW_SIZE] = 0;
                        send_base++;
                    }
                    else{
                        break;
                    }
                }
            }
        }
    }
}
int is_notcorrupt(struct packet *packet)
{
    if (packet->checksum == checksum(packet->data, 8))
    {
        return 1;
    }
    return 0;
}
int is_packet_recieved(struct packet *packet)
{
    if (packet->seq_num < recv_base)
        return 1;
    else 
        return 0;
}
struct packet * make_packet(char *data, int ack)
{
    struct packet *pkt;
    pkt = malloc(sizeof(struct packet));
    pkt->checksum = checksum(data, 8);
    pkt->ack = ack;
    if (ack == 0)
        pkt->seq_num = next_seqnum;
    else 
        pkt->seq_num = expected_seqnum;
    for (int i = 0; i < 8; i++)
        pkt->data[i] = data[i];
    return pkt;
}
char * extract_data(struct packet *packet)
{
    return NULL;
}
unsigned short checksum(char *addr, int count)
{
    /* RFC1071 checksum algorithm */
    register long sum = 0;
    unsigned short checksum = 0;

    while( count > 1 )  {
    /*  This is the inner loop */
        sum += * (unsigned short *) addr++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if( count > 0 )
        sum += * (unsigned char *) addr;

    /*  Fold 32-bit sum to 16 bits */
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    checksum = ~sum;

    return checksum;
}
void flush_msg()
{
    printf("Flush message: %s", rcv_buf);
    memset(rcv_buf, 0, BUF_SIZE);
    rcv_buf_indx = 0;
}
void print_packet(struct packet *pkt)
{
    printf("Packet: %d, End: %d, Data: ", pkt->seq_num, pkt->end);
    for (int i = 0; i < 8; i++)
    {
        printf("%c", pkt->data[i]);
    }
    printf("\n");
}
int terminate(char *msg)
{
    if (msg[0] == 'B' && msg[1] == 'Y' && msg[2] == 'E')
    {
        return 1;
    }
    else 
        return 0;
}

#pragma endregion PacketDefinitions


void *create_send_sock(void *unused)
{
    printf("Thread called\n");
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    size_t len;
    int bytes_send;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(host, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully connect(2).
        If socket(2) (or connect(2)) fails, we (close the socket
        and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        send_sfd = socket(rp->ai_family, rp->ai_socktype,
                    rp->ai_protocol);
        if (send_sfd == -1)
            continue;

        if (connect(send_sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */

        close(send_sfd);
    }

    freeaddrinfo(result);           /* No longer needed */

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    // Connection established and ready to send packets

    for (;;) {
        memset(inp_buf, 0, BUF_SIZE);
        fgets(inp_buf, BUF_SIZE, stdin);

        len = strlen(inp_buf)+1;
        rdt_send(inp_buf, 0);

        if (terminate(inp_buf))
        {
            printf("-------ENDING---------");
            shutdown(send_sfd, SHUT_RDWR);
            exit(0);
        }
    }
}

void *create_recv_sock(void *unused)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */

    s = getaddrinfo(NULL, RECV_PORT, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully bind(2).
        If socket(2) (or bind(2)) fails, we (close the socket
        and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        recv_sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (recv_sfd == -1)
            continue;

        if (setsockopt(recv_sfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) 
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(recv_sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(recv_sfd);
    }

    freeaddrinfo(result);           /* No longer needed */

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    /* Read datagrams and echo them back to sender. */

    printf("Server is listening...\n");

    for (;;) {
        peer_addr_len = sizeof(peer_addr);
        nread = recv(recv_sfd, buf, BUF_SIZE, 0);
        if (nread == -1)
            continue;               /* Ignore failed request */

        struct packet *p;
        p = (struct packet *)buf;
        printf("Packet recieved.....\n");
        rdt_recv(p);
    }
}

void *create_timer(void *unused)
{
    for (;;)
    {
        clock_t now = clock();
        for (int i = 0; i < WINDOW_SIZE; i++)
        {
            if (send_pkt[i] != NULL)
            {
                double time_elapsed = (double)(now - timers[i]) / CLOCKS_PER_SEC;
                if (time_elapsed > TIMEOUT)
                {
                    rdt_sendpkt(send_pkt[i]);
                    timers[i] = now;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    pthread_t t_recv, t_send, t_timer;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s host port_snd port_rcv\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    host = argv[1];
    port = argv[2];
    send_base = 0;
    recv_base = 0;
    next_seqnum = 0;
    expected_seqnum = 0;
    rcv_buf_indx = 0;

    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        send_pkt[i] = NULL;
        ack_packet[i] = 0;
        recv_pkt[i] = NULL;
    }

    pthread_create(&t_send, NULL, create_send_sock, NULL);
    pthread_create(&t_recv, NULL, create_recv_sock, NULL);
    pthread_create(&t_timer, NULL, create_timer, NULL);

    pthread_join(t_send, NULL);
}