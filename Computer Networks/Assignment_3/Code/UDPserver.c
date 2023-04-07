/*
	Name - Vishal Sharma
	Roll No. - 20CS8156
*/

#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <dirent.h> 

#define PORT 50156 
#define MAX_LEN 1024 

void list_dir(char *dir, char *buffer) 
{ 
    DIR *d; 
    struct dirent *dir_entry; 
    d = opendir(dir); 
    if (d) { 
        while ((dir_entry = readdir(d)) != NULL) { 
            strcat(buffer, dir_entry->d_name); 
            strcat(buffer, "\n"); 
        } 
        closedir(d); 
    } 
} 

int main(void) 
{ 
    int socket_desc, client_sock, read_size; 
    struct sockaddr_in server, client; 
    char buffer[MAX_LEN]; 
    
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0); 
    if (socket_desc == -1) { 
        printf("Could not create socket"); 
        return 1; 
    } 
    
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons(PORT); 
    
    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) { 
        printf("bind failed"); 
        return 1; 
    } 
    printf("Server ready\n"); 
    
    int
 slen = sizeof(client); 
    while (1) { 
        memset(buffer, 0, MAX_LEN); 
        recvfrom(socket_desc, buffer, MAX_LEN, 0, (struct sockaddr*)&client, &slen); 
        printf("Received request for directory: %s\n", buffer); 
        list_dir(buffer, buffer); 
        sendto(socket_desc, buffer, strlen(buffer), 0, (struct sockaddr*)&client, slen); 
    } 
    close(socket_desc); 
    return 0; 
}



