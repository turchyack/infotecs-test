#include "chat/types.hpp"
#include <cctype>
#include <cstdlib>
#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <unistd.h>
#include <charconv>

static uint16_t PORT = 10001; 
// 32 = 2^5

bool is_multiple_of_32(unsigned value) {
    const unsigned power10_5 = 100000;
    const unsigned power2_5 = 32;
    unsigned remainder = value % power10_5;
    return remainder % power2_5 == 0;
}
// return: 
// -1 - ошибка 
// > 0 - ошибок нет, длина строки в буфере -
// 0 - конец файла

int read_digits_line(int connection_sock, char* buffer, size_t buffer_size) { 
    for(size_t offset = 0; offset < buffer_size; offset++) {
        ssize_t n_bytes = read(connection_sock, buffer + offset, 1);
        if(n_bytes < 1) {
            if(n_bytes == -1) {
                perror("read failed");  
                return -1;  // ошибка ввода/вывода
            }
            return 0;  // конец файла
        }
        if(!std::isdigit(static_cast<int>(buffer[offset]))) {
            if(buffer[offset] == '\n') {
                if(offset == 0) {
                    errno = EINVAL;
                    perror("empty string");  // пустую строку считаем ошибкой 
                    return -1;  
                }
                return offset;  // прочитана строка цифр
            }
            perror("not digit");
            return -1;  // прочитана не цифра
        } 
    }
    errno = EMSGSIZE;
    perror("string too long");
    return -1;
}

int read_data(int connection_sock) {   
    const unsigned length_min = 3;
    const unsigned n_digits = 5;
    const size_t buffer_size = n_digits + 1;
    char buffer[buffer_size] = {0};  
    ssize_t line_length{};
    unsigned value{};
    while((line_length = read_digits_line(connection_sock, buffer, buffer_size)) > 0) {
        auto r = std::from_chars(buffer, buffer + line_length, value);
        if(r.ec != std::errc()) {
            perror("from_char: wrong number");
            return -1;
        }

        if(line_length < length_min) {
            printf("    error: expected length > %u, but received %u\n", length_min - 1, static_cast<unsigned>(line_length));
        }
        else if(!is_multiple_of_32(value)) {         
            printf("    error: value %u is not multiple of 32\n", value);
        }
        else {
            printf("    received: value is %u\n", value);
        }
    }
    return line_length;
}

int main() {
    const size_t buffer_size = 1024;
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1) {
        perror("socket failed");
        return -1;
    }

    SocketAddress server_address = {
        .sa_in = {
            .sin_family = AF_INET,
            .sin_port =  htons(PORT),
            .sin_addr = INADDR_ANY
        }
    };

    if(bind(server_sock, &server_address.sa, sizeof(server_address.sa_in)) == -1) {
        perror("bind failed");
        close(server_sock);
        return -1;
    }

    if(listen(server_sock, 3) == -1) {
		perror("listen");
        close(server_sock);
        return -1;
    }
    
    for(;;) {
        SocketAddress  remote_address = {};
        socklen_t addrlen = sizeof(server_address);
        printf("Wait for connections\n");
        int connection_sock = accept(server_sock, &remote_address.sa, &addrlen);
        if(connection_sock == -1) {
            perror("accept");
            close(server_sock);
            return -1;
	    }
        printf("Accepted connection from 0x%8.8x:%hu\n",
            ntohl(remote_address.sa_in.sin_addr.s_addr),
            ntohs(remote_address.sa_in.sin_port)
            );

        if(read_data(connection_sock) == -1) {
            perror("read_data failed!");
            close(connection_sock);
            continue;
        }
        if(close(connection_sock) == -1) {
            perror("close connection socket\n");
            close(server_sock);
            return -1;
        }        
        printf("Connection closed\n");  
    }
    return 0;
}