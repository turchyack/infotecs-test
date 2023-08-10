#include "chat/types.hpp"
#include <cctype>
#include <cstdlib>
#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <unistd.h>
#include <charconv>
#include <cstring>

static uint16_t PORT = 10001;

// функция
// проверяет делимость на 32 передаваемого ей числа
// return:
// true - если число делится на 32
// false - если число не делится на 32
//
bool is_multiple_of_32(unsigned value) {
    const unsigned power10_5 = 100000;
    const unsigned power2_5 = 32;
    unsigned remainder = value % power10_5;
    return remainder % power2_5 == 0;
}

// функция
// читает строку из соединения. Каждый символ строки должен быть цифрой(0-9),
// если это не так - возвращается ошибка.
// return:
// -1 - ошибка
// > 0 - ошибок нет, длина строки в буфере
// 0 - конец файла
//
int read_digits_line_remainder(int connection_sock, char* buffer, size_t buffer_size) {
    size_t offset = 0;
    ssize_t n_bytes = 0;
    if(buffer_size < 1) {
        errno = EINVAL;
        perror("buffer too small");
        return -1;
    }
    bool was_digits = true;
    while((n_bytes = read(connection_sock, buffer + offset, 1)) != 0) {
        if(n_bytes == -1) {
            perror("read failed");
            return -1; // ошибка ввода/вывода
        }
        if(buffer[offset] == '\n') {
            if(was_digits) {
                return offset;
            }
            return -2;   // ошибка данных - прочитана не цифра
        }
        if(was_digits) {
            if(!std::isdigit(buffer[offset])) {
                was_digits = false;
            }
        }
        if(offset >= buffer_size - 1) {
            std::memmove(&buffer[0], &buffer[1], sizeof(buffer[0]) * (buffer_size - 1));
        }
        else {
            offset++;
        }
    }
    return 0;
}

// функция вычитывет строки из соединения
// проверяет делимость на 32
// return:
// 0 - ошибок нет, достигнут
// -1
//
int read_data(int connection_sock) {
    const unsigned length_min = 3;
    const unsigned n_digits = 5;
    const size_t buffer_size = n_digits + 1;
    char buffer[buffer_size] = {0};
    ssize_t line_length{};
    unsigned value{};
    while((line_length = read_digits_line_remainder(connection_sock, buffer, buffer_size)) != 0) {
        if(line_length < 0) {
            if(line_length == -2) {
                printf("    error: received wrong number\n");
                continue;
            }
            return -1;
        }
        auto r = std::from_chars(buffer, buffer + line_length, value);
        if(r.ec != std::errc()) {
            printf("    error: received wrong number\n");
            continue;
        }

        if(line_length < length_min) {
            printf("    error: expected length > %u, but received %u\n", length_min - 1, static_cast<unsigned>(line_length));
        }
        else if(!is_multiple_of_32(value)) {
            printf("    error: value %u is not multiple of 32\n", value);
        }
        else {
            printf("    received number of multiple of 32\n");
        }
    }
    return 0;
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1) {
        perror("socket failed");
        return -1;
    }

    SocketAddress server_address {};
    server_address.sa_in.sin_family = AF_INET;
    server_address.sa_in.sin_port =  htons(PORT);
    server_address.sa_in.sin_addr.s_addr = INADDR_ANY;

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
        SocketAddress remote_address {};
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
