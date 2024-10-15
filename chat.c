#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_RETRY 5
#define BUF_SIZE 65536
#define USER_NUM 1
#define PORT_NUM 54324
#define SOURCE_IP "10.1.11.253"

struct IPRecords {
	char name[50];
	char ip[16];
};

struct IPRecords iprec[USER_NUM];

void setUsers() {
	strcpy(iprec[0].name, "Jana");
    strcpy(iprec[0].ip, "10.1.11.241");
}

struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};

unsigned short csum(unsigned short *ptr, int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)ptr;
        sum += oddbyte;
    }

    sum = (sum & 0xffff) + (sum >> 16);
    sum = sum + (sum >> 16);
    answer = (short)~sum;

    return answer;
}

void* send_udp_packets(void* arg) {
    char datagram[4096], source_ip[32], *data;
    struct iphdr *iph;
    struct udphdr *udph;
    struct sockaddr_in sin;
    struct pseudo_header psh;

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (s < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    int sndbuf = BUF_SIZE;
    setsockopt(s, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));

    memset(datagram, 0, 4096);

    iph = (struct iphdr*) datagram;
    udph = (struct udphdr*) (datagram + sizeof(struct iphdr));

    while (1) {
        char msg[1024],dummy;
        
        while(scanf("%c",&dummy)) {
			if(dummy == '\n')
				break;
		}
		
        printf("Me : ");
        fgets(msg, sizeof(msg), stdin);
        msg[strcspn(msg, "\n")] = 0;
        
        for(int i=0;i<USER_NUM;i++) {
 			data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
		    strcpy(data, msg);
		    strcpy(source_ip, SOURCE_IP);

		    sin.sin_family = AF_INET;
		    sin.sin_port = htons(PORT_NUM);
		    sin.sin_addr.s_addr = inet_addr(iprec[i].ip);

		    iph->ihl = 5;
		    iph->version = 4;
		    iph->tos = 0;
		    iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
		    iph->id = htonl(12345);
		    iph->frag_off = 0;
		    iph->ttl = 255;
		    iph->protocol = IPPROTO_UDP;
		    iph->check = 0;
		    iph->saddr = inet_addr(source_ip);
		    iph->daddr = sin.sin_addr.s_addr;
		    iph->check = csum((unsigned short*)datagram, iph->tot_len);

		    udph->source = htons(6666);
		    udph->dest = htons(PORT_NUM);
		    udph->len = htons(8 + strlen(data));
		    udph->check = 0;

		    psh.source_address = inet_addr(source_ip);
		    psh.dest_address = sin.sin_addr.s_addr;
		    psh.placeholder = 0;
		    psh.protocol = IPPROTO_UDP;
		    psh.udp_length = htons(8 + strlen(data));

		    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
		    char *pseudogram = malloc(psize);
		    memcpy(pseudogram, (char*)&psh, sizeof(struct pseudo_header));
		    memcpy(pseudogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr) + strlen(data));
		    udph->check = csum((unsigned short*)pseudogram, psize);

		    free(pseudogram);

		    int retry = 0;
		    while (retry < MAX_RETRY) {
		        if (sendto(s, datagram, iph->tot_len, 0, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		            perror("Sending failed, retrying...");
		            retry++;
		            usleep(1000);
		        } else {
		            break;
		        }
		    }
		    if (retry == MAX_RETRY) {
		        printf("Failed to send packet after %d attempts.\n", MAX_RETRY);
		    }
		    usleep(10000);
		}
    }

    close(s);
    pthread_exit(NULL);
}

void* receive_udp_packets(void* arg) {
    int data_size;
    unsigned char *buffer = (unsigned char*)malloc(BUF_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        pthread_exit(NULL);
    }

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        perror("Socket creation failed");
        free(buffer);
        pthread_exit(NULL);
    }

    int reuse = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    int rcvbuf = BUF_SIZE;
    setsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT_NUM);
    sin.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("Bind failed");
        close(s);
        free(buffer);
        pthread_exit(NULL);
    }

	struct sockaddr_in sender_addr;
    socklen_t sin_len = sizeof(sender_addr);

    while (1) {
        data_size = recvfrom(s, buffer, BUF_SIZE, 0, (struct sockaddr*)&sender_addr, &sin_len);
        if (data_size <= 0) {
            perror("Failed to receive packet");
            break;
        }
        for(int i=0;i<USER_NUM;i++) {
		    if(sender_addr.sin_addr.s_addr == inet_addr(iprec[i].ip)) {
		    	buffer[data_size] = '\0';
		    	printf("\n%s : %s\n", iprec[i].name, buffer);
		    	usleep(10000);
		    	break;
			}
		}
    	
    }

    free(buffer);
    close(s);
    pthread_exit(NULL);
}

int main() {
	setUsers();
	
    pthread_t send_thread, receive_thread;

    pthread_create(&send_thread, NULL, send_udp_packets, NULL);
    pthread_create(&receive_thread, NULL, receive_udp_packets, NULL);

    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    return 0;
}
