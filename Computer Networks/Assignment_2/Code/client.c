/*
    Name - Vishal Sharma
    Roll No. - 20CS8156
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Libraries for socket programing
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main()
{
    char buff[100];            // data buffer for sending & receiving
    unsigned int port = 50156; // port client will connect to
    struct sockaddr_in server; // server address
    int sockfd;                // client socket

    // Put server information into server structure
    server.sin_family = AF_INET;
    // The port must be put into network byte order
    server.sin_port = htons(port);
    // Address for server
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1)
    {
        // Enter an expression
        printf("Enter an expression / -1 to exit: ");
        fgets(buff, 100, stdin);

        // Removes all whitespace characters in input
        buff[strcspn(buff, "\n")] = '\0';
        for (int i = 0; i < strlen(buff); i++)
            if (buff[i] == ' ')
                for (int j = i; j < strlen(buff); j++)
                    buff[j] = buff[j + 1];

        // If user enters -1, exit
        if (strcmp(buff, "-1") == 0 || strcmp(buff, "") == 0)
            return 0;

        // Get a stream socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            printf("Error opening socket\n");
            return 1;
        }
        // Connect to the server
        if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            printf("Connection Failed\n");
            return 1;
        }

        // Send the expression to the server
        if (send(sockfd, buff, 100, 0) == -1)
        {
            printf("Error in sending data\n");
            return 1;
        }
        // Empties buff
        bzero(buff, 100);

        // Receive the result from the server
        if (recv(sockfd, buff, 100, 0) < 0)
        {
            printf("Error receiving data\n");
            return 1;
        }
        printf("The result is: %s\n", buff);
        // Empties buff
        bzero(buff, 100);
        // Close connection with server
        close(sockfd);
    }

    return 0;
}