#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>

#define BUF_SIZE 500
#define SEND_PORT "5201"
#define WINDOW_SIZE 16
#define TIMEOUT 3

struct packet
{
    unsigned short checksum;
    unsigned short seq_num;
    unsigned short ack; // 1 for ack packet, -1 for NAK packet, 0 for data packet
    unsigned short end;
    char data[8];
};

#pragma region Globals
char *host, *port;
int send_sfd, recv_sfd, sfd;
int send_base, recv_base, next_seqnum, expected_seqnum;
char inp_buf[BUF_SIZE], rcv_buf[BUF_SIZE];
int rcv_buf_indx;
struct packet *send_pkt[WINDOW_SIZE];
int ack_packet[WINDOW_SIZE];
struct packet *recv_pkt[WINDOW_SIZE];
clock_t timers[WINDOW_SIZE];
struct sockaddr_storage peer_addr;
socklen_t peer_addr_len;
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
    sendto(sfd, (void *)pkt, sizeof(struct packet), 0, (struct sockaddr *) &peer_addr, peer_addr_len);
}

void rdt_send(char *data, int ack)
{
    struct packet *pkt;

    if (ack == 0) // if it is a data packet
    {
        int len = strlen(data);
        int indx = 0;

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
    if (packet->ack == 0) // if it is a data packet
    {
        if (is_notcorrupt(packet) && packet->seq_num >= recv_base
            && packet->seq_num < recv_base + WINDOW_SIZE)
        {
            /* if this packet is not recieved early */
            if (!is_packet_recieved(packet))
            {
                recv_pkt[(packet->seq_num) % WINDOW_SIZE] = packet; // add packet to packet buffer

                rdt_send("ACK", 1); // send ACK packet
                expected_seqnum++;

                if (packet->seq_num == recv_base)
                {
                    /* foreach sequentially buffered packets in recv_base */
                    for (int i = recv_base; i < recv_base+WINDOW_SIZE; i++)
                    {
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
    printf("%s", rcv_buf);
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

void *create_recv_sock(void *unused)
{
    for (;;) {
        peer_addr_len = sizeof(peer_addr);
        nread = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (nread == -1)
            continue;               /* Ignore failed request */

        struct packet *p;
        p = (struct packet *)buf;
        rdt_recv(p);
    }
}

void *create_send_sock(void *unused)
{
    char msg[BUF_SIZE];
    size_t len;
    int bytes_send;

    for (;;) {
        memset(msg, 0, BUF_SIZE);
        fgets(msg, BUF_SIZE, stdin);

	    //len = strlen(msg)+1;
	    //rdt_send(msg, 0);

        if (terminate(msg))
        {
            shutdown(sfd, SHUT_RDWR);
            exit(0);
        }

	    len = strlen(msg)+1;
	    rdt_send(msg, 0);
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

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully bind(2).
        If socket(2) (or bind(2)) fails, we (close the socket
        and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(sfd);
    }

    freeaddrinfo(result);           /* No longer needed */

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    /* Read datagrams and echo them back to sender. */

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = argv[1];
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

    pthread_create(&t_recv, NULL, create_recv_sock, NULL);
    pthread_create(&t_send, NULL, create_send_sock, NULL);
    pthread_create(&t_timer, NULL, create_timer, NULL);

    pthread_join(t_send, NULL);
}
