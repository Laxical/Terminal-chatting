#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <ifaddrs.h>

int main() {
    struct ifaddrs *addresses;
    if(getifaddrs(&addresses) == -1) {
        printf("getifaddrs call failed\n");
        return -1;
    }

    char ap[100] = {0};
    char *myIp;
    struct ifaddrs *address = addresses;
    while(address) {
        int family = address->ifa_addr->sa_family;
        if(family == AF_INET) {
            char ap[100];
            if(strcmp(address->ifa_name, "wlo1") == 0) {
                const int family_size = sizeof(struct sockaddr_in);
                getnameinfo(address->ifa_addr, family_size, ap, sizeof(ap), 0, 0, NI_NUMERICHOST);
                myIp = malloc(strlen(ap) + 1);
                strcpy(myIp, ap);
            }
        }
        address = address->ifa_next;
    }
    freeifaddrs(addresses);

    printf("Configuring ip address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(myIp, "6969", &hints, &bind_address);
    free(myIp);

    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(bind_address->ai_addr, bind_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST | NI_NUMERICSERV);
    printf("ipaddr: %s\tport: %s\n", address_buffer, service_buffer);

    printf("Creating socket...\n");
    int socket_listen;
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if (socket_listen < 0) {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return 1;
    }


    printf("Binding socket to local address...\n");
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", errno);
        return 1;
    }
    freeaddrinfo(bind_address);


    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", errno);
        return 1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    int max_socket = socket_listen;

    printf("Waiting for connections...\n");


    while(1) {
        fd_set reads;
        reads = master;
        if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", errno);
            return 1;
        }

        for(int i = 1; i <= max_socket; ++i) {
            if (FD_ISSET(i, &reads)) {
                if (i == socket_listen) {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    int socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);
                    if (socket_client < 0) {
                        fprintf(stderr, "accept() failed. (%d)\n",
                                errno);
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)
                        max_socket = socket_client;

                    char address_buffer[100];
                    getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
                    printf("New connection from %s\n", address_buffer);
                } else {
                    char read[1024];
                    int bytes_received = recv(i, read, 1024, 0);
                    if (bytes_received < 1) {
                        FD_CLR(i, &master);
                        close(i);
                        continue;
                    }

                    for (int j = 1; j <= max_socket; ++j) {
                        if(FD_ISSET(j, &master)) {
                            if(j == socket_listen || j == i) continue;
                            else send(j, read, bytes_received, 0);
                        }
                    }
                }

            }
        }
    } 

    printf("Closing listening socket...\n");
    close(socket_listen);

    printf("Finished.\n");

    return 0;
}