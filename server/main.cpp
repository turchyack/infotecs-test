#include "chat/types.hpp"

#include <cstdlib>
#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <unistd.h>


static uint16_t PORT = 10001; 


int main() {
    const size_t buffer_size = 1024;

    printf("DEBUG: socket\n");
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1) {
        perror("socket failed");
        return -1;
    }
    printf("DEBUG: socket done, server_sock = %d\n", server_sock);
        SocketAddress  server_address = {
        .sa_in = {
            .sin_family = AF_INET,
            .sin_port =  htons(PORT),
            .sin_addr = INADDR_ANY
        }
    };
    printf("DEBUG: bind\n");
    if(bind(server_sock, &server_address.sa, sizeof(server_address.sa_in)) == -1) {
        perror("bind failed");
        close(server_sock);
        return -1;
    }
    printf("DEBUG: bind done, listen\n");
    
    if(listen(server_sock, 3) == -1) {
		perror("listen");
        close(server_sock);
        return -1;
    }
    printf("DEBUG: listen done\n");
    
    for(;;) {

        SocketAddress  remote_address = {};
        
        socklen_t addrlen = sizeof(server_address);
        printf("DEBUG: accept, wait connections\n");
    
        int connection_sock = accept(server_sock, &remote_address.sa, &addrlen);
        if(connection_sock == -1) {
            perror("accept");
            close(server_sock);
            return -1;
	    }
        printf("DEBUG: accepted connection (%d) from 0x%8.8x\n",connection_sock, ntohl(remote_address.sa_in.sin_addr.s_addr));
        sleep(3);
        if(shutdown(connection_sock, SHUT_RDWR) == -1) {

            perror("shutdown");
            close(connection_sock);
            close(server_sock);
            return -1;
        }
        sleep(1);
        if(close(connection_sock) == -1) {
            perror("close connection socket\n");
            close(server_sock);
            return -1;
        }        
        
        printf("DEBUG: connection closed\n");
        
    }






    return 0;
}