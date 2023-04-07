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

// Function to compute the expression
int compute_expression(char input[100])
{
    int len_inp = strlen(input), i, currNum = 0, ans = 0;

    // Here we are storing + to just simply get the first number as ans is 0
    char ls_op = '+'; // Store last operator that was encountered

    input[len_inp] = '+';    // Added an extra operator at the end
    input[++len_inp] = '\0'; // so that we get for the last number at the end
    len_inp++;

    for (i = 0; i < len_inp; i++)
    {
        // If the encountered char is a digit
        if (input[i] >= '0' && input[i] <= '9')
            currNum = currNum * 10 + input[i] - '0';
        else
        {
            // We have the first number and can perform the op on ans
            switch (ls_op)
            {
            case '+':
                ans += currNum;
                break;
            case '-':
                ans -= currNum;
                break;
            case '*':
                ans *= currNum;
                break;
            case '/':
                ans /= currNum;
                break;
            default:
                return -1;
            }
            currNum = 0;      // Empty currNum to store next number
            ls_op = input[i]; // Update the last operator encountered
        }
    }
    return ans;
}

int main()
{
    char buff[100];                              // data buffer for sending & receiving
    int sockfd, newSockfd;                       // client socket
    unsigned int port = 50156;                   // port client will connect to
    struct sockaddr_in server_addr, client_addr; // server address
    int client_len;
    int opt = 1;
    // Get a stream socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Error opening socket\n");
        return 1;
    }

    // Put server information into server structure
    server_addr.sin_family = AF_INET;
    // The port must be put into network byte order
    server_addr.sin_port = htons(port);
    // Address for server is '127.0.0.1' so INADDR_ANY is used
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Binds this process to the port
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Error binding to port %d\n", port);
        return 1;
    }
    printf("Connected to PORT: %d\n", port);

    // Start listening on the port
    // supports upto 5 queded requests while process is blocked on accept syscall
    listen(sockfd, 5);

    while (1)
    {
        client_len = sizeof(client_addr);
        // The accept() system call accepts a client connection
        newSockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (newSockfd == -1)
        {
            printf("Accept error\n");
            return 1;
        }

        if (fork() == 0)
        {
            close(sockfd);
            // Receive the next expression
            if (recv(newSockfd, buff, 100, 0) < 0)
            {
                printf("Error receiving data\n");
                return 1;
            }
            printf("Received: %s\n", buff);

            int ans = compute_expression(buff);
            // Empties buff
            bzero(buff, 100);
            // Writes ans into buff
            snprintf(buff, sizeof(buff), "%d", ans);

            // Sends the result to the client process
            if (send(newSockfd, buff, 100, 0) < 0)
            {
                printf("Error sending data\n");
                return 1;
            }

            // Empties buff
            bzero(buff, 100);
            // Close connection with current client
            close(newSockfd);
            exit(0);
        }
        close(newSockfd);
    }

    return 0;
}