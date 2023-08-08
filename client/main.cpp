#include <errno.h>
#include <cstdio>
#include <cstring>
#include <cctype>

// return
// -1 - ошибка ввода/вывода
// -2 - ошибка - введена пустая строка
// -3 - ошибка - строка слишком длинная
// -4 - ошибка - не цифра
// 0 - конец потока
// >0 - принята строка цифр, значение представляет длину строки
// 
ssize_t input_digits_line(FILE* stream, char* buffer, size_t buffer_size) {
    printf("введите строку: ");
    fflush(stdout);
    int c = 0;
    size_t line_length = 0;
    bool was_digits = true;
    while((c = getc(stream)) != EOF) {
        if(c == '\n') {
            if(line_length == 0) {
                return -2;
            }
            if(!was_digits) {
                return -4;
            }
            if(line_length > buffer_size) {
                return -3;
            }
            
            return line_length;
        }
        if(was_digits) {
            if(std::isdigit(c)) {
                if(line_length < buffer_size) {
                    buffer[line_length] = c;
                }
            }
            else {
                was_digits = false;
            }
        }
        line_length++;
    }
    if(ferror(stream)) {
        return -1;  // ошибка ввода/вывода
    }
    return 0;   // конец потока  
}

int main() {
    const size_t buffer_size = 64; 
    char buffer[buffer_size] = {};
    ssize_t line_length = 0;
    while((line_length = input_digits_line(stdin, buffer, buffer_size)) != 0) {
        if(line_length < 0) {
            switch(line_length) {
            case -1: 
                fprintf(stderr, "error: IO error - %s\n", strerror(errno));
                return -1;
            case -2:
                fprintf(stderr, "error: input empty string\n");
                continue;
            case -3:
                fprintf(stderr, "error: too long string\n");
                continue;
            default:
                fprintf(stderr, "error: wrong number\n");
                continue;       
            }
        }
        printf("принята строка[%u]: '%.*s'\n", static_cast<unsigned>(line_length), static_cast<unsigned>(line_length), buffer);
    }
    return 0;
}