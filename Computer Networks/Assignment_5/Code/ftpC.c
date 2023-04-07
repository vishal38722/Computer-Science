/*
	Name - Vishal Sharma
	Roll No. - 20CS8156
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdint.h>
#include <wait.h>

#define PORTX 50000
#define MAX_CHAR 100
#define MAX_TOKENS 10


void tokenise(char command[], char tokens[MAX_TOKENS][MAX_CHAR], int *n)
{
    (*n) = 0;
    char *pch;
    pch = strtok(command, " ");
    while (pch != NULL)
    {
        strcpy(tokens[(*n)++], pch);
        pch = strtok(NULL, " ");
    }
    return;
}

int get_int(int fd, int *num)
{
    int16_t ret;
    char *data = (char *)&ret;
    int left = sizeof(ret);
    int rc;
    do
    {
        rc = read(fd, data, left);
        if (rc <= 0)
        { /* instead of ret */
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // use select() or epoll() to wait for the socket to be readable again
            }
            else if (errno != EINTR)
            {
                return -1;
            }
        }
        else
        {
            data += rc;
            left -= rc;
        }
    } while (left > 0);
    *num = ntohs(ret);
    return 0;
}

int send_int(int num, int fd)
{
    int16_t conv = htons(num);
    char *data = (char *)&conv;
    int left = sizeof(conv);
    int rc;
    do
    {
        rc = write(fd, data, left);
        if (rc < 0)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // use select() or epoll() to wait for the socket to be writable again
            }
            else if (errno != EINTR)
            {
                return -1;
            }
        }
        else
        {
            data += rc;
            left -= rc;
        }
    } while (left > 0);
    return 0;
}

void sendfile(char *name, int sock_fd)
{
    int fd = open(name, O_RDONLY);
    if (fd == -1)
    {
        // perror("[ERROR] File can't be open\n");
        close(sock_fd);
        exit(1);
        return;
    }
    else
    {
        int byte_read = 0;
        do
        {
            char read_buffer[MAX_CHAR];
            byte_read = read(fd, read_buffer, MAX_CHAR - 1);
            read_buffer[byte_read] = '\0';
            if (byte_read == MAX_CHAR - 1)
            {
                send(sock_fd, "N", 1, 0);
                send_int(byte_read+1,sock_fd);
            }
            else
            {
                send(sock_fd, "L", 1, 0);
                send_int(byte_read + 1, sock_fd);
            }
            
            send(sock_fd, read_buffer, strlen(read_buffer)+1, 0);
            // printf("%s",read_buffer);
        } while (byte_read == MAX_CHAR - 1);
        send(sock_fd, "\0", strlen("\0") + 1, 0);
        close(sock_fd);
        close(fd);
        exit(0);
        return;
    }
}

void receivefile(char *name, int sock_fd)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        // perror("[ERROR] Can't create a file\n");
        close(sock_fd);
        exit(1);
        return;
    }
    else
    {
        int total = 0;
        char buf[2];
        while(1)
        {
           int r =  recv(sock_fd, buf, 1, 0);
           if(r==0)
           {
               remove(name);
               break;
           }
            buf[1] = '\0';
            
            if(!strcmp(buf,"L"))
            {
                int num;
                get_int(sock_fd, &num);
                // printf("%d %s++\n", num, buf);
                char buffer[num];
                total+=num;
                recv(sock_fd, buffer, num, 0);
                write(fd, buffer, num-1);
                break;
            }
            else
            {
                int num;
                get_int(sock_fd, &num);
                // printf("%d %s\n",num,buf);
                char buffer[num];
                total += num;
                recv(sock_fd, buffer, num, 0);
                write(fd, buffer, num - 1);
            }
        }
        if(total==0)
        {
            remove(name);
            exit(1);
        }
        close(sock_fd);
        close(fd);
        exit(0);
        return;
    }
}

void error(int x,int code,int sock_fd)
{
    if(x==502)
    {
        printf(": Invalid command\n");
    }
    else if(x==501)
    {
        printf(": Please enter commnd with correct input\n");
    }
    else if (x == 503)
    {
        printf(": Enter port command first and correctly\n");
        close(sock_fd);
        exit(0);
    }
    else if(code == 0)
    {
        if(x==503)
        {
            printf(": Enter port command first and correctly\n");
            close(sock_fd);
            exit(0);
        }
        else if (x == 550)
        {
            printf(": Unable to send command\n");
            close(sock_fd);
            exit(0);
        }
        else if(x == 200)
        {
            printf(": Success\n");
        }
    }
    else if(code == 1)
    {
        if (x == 501)
        {
            printf(": Unable to execute\n");
        }
        else if (x == 200)
        {
            printf(": Success\n");
        }
    }
    else if (code == 2||code == 3)
    {
        if (x == 550)
        {
            printf(": File can't be found or unable to read/write\n");
        }
        else if (x == 250)
        {
            printf(": Success\n");
        }
    }
    else
    {
        printf(": Exit\n");
        exit(0);
    }
    
}
void PORT(char* command,char tokens[MAX_TOKENS][MAX_CHAR],int n,int sock_fd)
{
    send(sock_fd, command, strlen(command) + 1, 0);
}

void GET(char* command, char tokens[MAX_TOKENS][MAX_CHAR],int n, int sockc_fd,int port)
{
    pid_t clinet_d;
    if((clinet_d=fork())==0)
    {
        int sockd_fd, newsockd_fd;
        socklen_t servd_len;
        struct sockaddr_in clid_add,serv_add;

        if ((sockd_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("[ERROR] Unable to create the socket\n");
            exit(0);
        }

        memset(&clid_add, 0, sizeof(clid_add));
        memset(&serv_add,0,sizeof(serv_add));
        clid_add.sin_family = AF_INET;
        clid_add.sin_addr.s_addr = INADDR_ANY;
        clid_add.sin_port = htons(port);

        //Bind the socket
        if (bind(sockd_fd, (const struct sockaddr *)&clid_add, sizeof(clid_add)) < 0)
        {
            perror("[ERROR] Unable to bind the socket Y\n");
            exit(0);
        }
        //Listen
        // printf("Port : %d\n", port);
        // printf("Listening ...\n");
        listen(sockd_fd, 5);
        servd_len = sizeof(serv_add);
        newsockd_fd = accept(sockd_fd, (struct sockaddr *)&serv_add, &servd_len);
        receivefile(tokens[1],newsockd_fd);
        close(sockd_fd);
        exit(0);
    }
    else
    {
        send(sockc_fd, command, strlen(command) + 1, 0);
        int status;
        waitpid(clinet_d, &status, 0);
    }
}

void PUT(char *command, char tokens[MAX_TOKENS][MAX_CHAR], int n, int sockc_fd, int port)
{
    {
        pid_t clinet_d;
        if ((clinet_d = fork()) == 0)
        {
            int sockd_fd, newsockd_fd;
            socklen_t servd_len;
            struct sockaddr_in clid_add, serv_add;

            if ((sockd_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("[ERROR] Unable to create the socket\n");
                exit(0);
            }

            memset(&clid_add, 0, sizeof(clid_add));
            memset(&serv_add, 0, sizeof(serv_add));
            clid_add.sin_family = AF_INET;
            clid_add.sin_addr.s_addr = INADDR_ANY;
            clid_add.sin_port = htons(port);

            //Bind the socket
            if (bind(sockd_fd, (const struct sockaddr *)&clid_add, sizeof(clid_add)) < 0)
            {
                perror("[ERROR] Unable to bind the socket Y\n");
                exit(0);
            }
            //Listen
            // printf("Port : %d\n", port);
            // printf("Listening ...\n");
            listen(sockd_fd, 5);
            servd_len = sizeof(serv_add);
            newsockd_fd = accept(sockd_fd, (struct sockaddr *)&serv_add, &servd_len);
            sendfile(tokens[1], newsockd_fd);
            close(sockd_fd);
            exit(0);
        }
        else
        {
            send(sockc_fd, command, strlen(command) + 1, 0);
            int status;
            waitpid(clinet_d, &status, 0);
        }
    }
    
    return;
}

int main()
{
    int sockc_fd;
    struct sockaddr_in servc_add;
    char command[MAX_CHAR];
    if ((sockc_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[ERROR] Unable to create the socket\n");
        exit(0);
    }

    servc_add.sin_family = AF_INET;
    servc_add.sin_addr.s_addr = INADDR_ANY;
    servc_add.sin_port = htons(PORTX);
    // making connection to server
    if (connect(sockc_fd, (struct sockaddr *)&servc_add, sizeof(servc_add)) < 0)
    {
        perror("[ERROR] Unable to connect to server\n");
        close(sockc_fd);
        exit(0);
    }
    int port = 1024;
    while(1)
    {
        printf("> ");
        scanf("%[^\n]%*c", command);

        char temp[MAX_CHAR],tokens[MAX_TOKENS][MAX_CHAR];
        int n,code;
        strcpy(temp,command);
        tokenise(temp,tokens,&n);
        
        if(!strcmp(tokens[0],"PORT"))
        {
            port = atoi(tokens[1]);
            PORT(command,tokens,n,sockc_fd);
            code =0;
        }
        else if(!strcmp(tokens[0],"get"))
        {
            code = 2;
            if(port != 1024)
            {
                GET(command, tokens, n, sockc_fd, port);
            }
            else
            {
                send(sockc_fd, command, strlen(command) + 1, 0);
            }
            
        }
        else if (!strcmp(tokens[0], "put"))
        {
            code = 3;
            if(port!=1024)
            {
                PUT(command, tokens, n, sockc_fd, port);
            }
            else
            {
                send(sockc_fd, command, strlen(command) + 1, 0);
            }
            
        }
        else
        {
            send(sockc_fd, command, strlen(command) + 1, 0);
            code = (!strcmp(tokens[0], "cd") ? 1: 4);
        }

        int error_code;
        sleep(0.01);
        get_int(sockc_fd,&error_code);
        printf("%d",error_code);
        error(error_code, code, sockc_fd);
        fflush(STDIN_FILENO);

    }
    return 0;
}
