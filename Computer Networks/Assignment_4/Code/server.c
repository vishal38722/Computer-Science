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

#define PORT 50156 // Port to listen on

void handle_tcp_connection(int sockfd) {
    char buffer[1024];
    int n = read(sockfd, buffer, sizeof(buffer));
    if (n < 0) {
        perror("read");
        return;
    }
    buffer[n] = '\0';

    // Check if the string is a palindrome
    int i, j;
    for (i = 0, j = strlen(buffer) - 2; i <= j; i++, j--) {
    	// printf("%d %d\n", i, j);
        if (tolower(buffer[i]) != tolower(buffer[j])) {
            write(sockfd, "Not a palindrome\n", 17);
            return;
        }
    }
    write(sockfd, "Palindrome\n", 11);
}

void handle_udp_connection(int sockfd) {
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
    if (n < 0) {
        perror("recvfrom");
        return;
    }
    buffer[n] = '\0';

    // Check if the string is a palindrome
    int i, j;
    for (i = 0, j = strlen(buffer) - 2; i < j; i++, j--) {
    	// printf("%d %d\n", i, j);
        if (tolower(buffer[i]) != tolower(buffer[j])) {
            sendto(sockfd, "Not a palindrome\n", 17, 0, (struct sockaddr*)&client_addr, client_addr_len);
            return;
        }
    }
    sendto(sockfd, "Palindrome\n", 11, 0, (struct sockaddr*)&client_addr, client_addr_len);
}

int main() {
    int tcp_sockfd, udp_sockfd, max_fd, activity, tcp_connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    fd_set readfds;

    // Create TCP socket
    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // Create UDP socket
    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind TCP socket to server address
    if (bind(tcp_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Bind UDP socket to server address
    if (bind(udp_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Listen for TCP connections
    if (listen(tcp_sockfd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    // Initialize the set of file descriptors to monitor
    FD_ZERO(&readfds);
    FD_SET(tcp_sockfd, &readfds);
    FD_SET(udp_sockfd, &readfds);
    max_fd = (tcp_sockfd > udp_sockfd) ? tcp_sockfd : udp_sockfd;

    while (1) {
        // Wait for activity on any of the file descriptors
        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            exit(1);
        }

        // Handle TCP connection
        if (FD_ISSET(tcp_sockfd, &readfds)) {
            //tcp_connfd = accept(tcp_sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr));
            tcp_connfd = accept(tcp_sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
            if (tcp_connfd < 0) {
                perror("accept");
                continue;
            }
            handle_tcp_connection(tcp_connfd);
            close(tcp_connfd);
        }

        // Handle UDP connection
        if (FD_ISSET(udp_sockfd, &readfds)) {
            handle_udp_connection(udp_sockfd);
        }

        // Clear the set of file descriptors to monitor
        FD_ZERO(&readfds);
        FD_SET(tcp_sockfd, &readfds);
        FD_SET(udp_sockfd, &readfds);
    }

    return 0;
}
