/*
	Name - Vishal Sharma
	Roll No. - 20CS8156
*/

#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 

#define PORT 50156 
#define MAX_LEN 1024 

int main(void) 
{ 
    int socket_desc; 
    struct sockaddr_in server; 
    char buffer[MAX_LEN]; 
    
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0); 
    if (socket_desc == -1) { 
        printf("Could not create socket"); 
        return 1; 
    } 
    
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server.sin_port = htons(PORT); 
    
    printf("Enter the directory name: "); 
    scanf("%s", buffer); 
    
    sendto(socket_desc, buffer, strlen(buffer), 0, (struct sockaddr*)&server, sizeof(server)); 
    memset(buffer, 0, MAX_LEN); 
    recvfrom(socket_desc, buffer, MAX_LEN, 0, NULL, NULL); 
    printf("Directory listing: \n%s", buffer); 
    
    close(socket_desc); 
    return 0; 
}
