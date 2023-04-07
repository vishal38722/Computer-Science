/*
	Name - Vishal Sharma
	Roll No. - 20CS8156
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>

#define SERVER_IP "127.0.0.1" // IP address of server
#define SERVER_PORT "50156" // Port number of server
#define MAX_BUFFER_SIZE 1024 // Maximum size of buffer to send/receive data

int main() {
    int sockfd, num_bytes;
    struct addrinfo hints, *res;
    char buffer[MAX_BUFFER_SIZE];

    // Initialize hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Use IPv4
    hints.ai_socktype = SOCK_DGRAM; // Use UDP

    // Get address info for server
    int status = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    // Create socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // Send data to server
    printf("Enter a string: ");
    fgets(buffer, MAX_BUFFER_SIZE, stdin);
    num_bytes = sendto(sockfd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);
    if (num_bytes < 0) {
        perror("sendto");
        exit(1);
    }

    // Receive response from server
    num_bytes = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE - 1, 0, NULL, NULL);
    if (num_bytes < 0) {
        perror("recvfrom");
        exit(1);
    }
    buffer[num_bytes] = '\0';

    // Print response from server
    printf("Server response: %s\n", buffer);

    // Clean up
    freeaddrinfo(res);
    close(sockfd);

    return 0;
}
