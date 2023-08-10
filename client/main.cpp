#include <errno.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>

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

ssize_t subtitude_even_digits(char* result_buffer,  size_t result_buffer_size, size_t line_length) {
    const char subst_fragment[] = "KB";
    size_t fragment_size = sizeof(subst_fragment) -1;
    size_t result_length = line_length;

    for(size_t i = 0; i< result_length; ++i) {
        if(static_cast<unsigned>(result_buffer[i] - '0') % 2 == 0) {
            memmove(&result_buffer[i + fragment_size-1],&result_buffer[i], sizeof(result_length - i));
            if(result_buffer_size - result_length < 2) {
                return -1;
            }
            memcpy(&result_buffer[i], subst_fragment, fragment_size);
            i += fragment_size - 1;
            result_length += fragment_size -1;
        }
    }

    return result_length;
}

// return
// line_length - длина строки
// -1 - ошибка. заданый буфер имеет недостаточный размер
//
ssize_t transform_line(const char* line, size_t line_length, char* result_buffer, size_t result_buffer_size) {
    if(result_buffer_size < line_length) {
        return -1;
    }
    memcpy(result_buffer, line, line_length * sizeof(line[0]));
    std::sort(result_buffer, &result_buffer[line_length], std::greater<char>());

    return subtitude_even_digits(result_buffer,  result_buffer_size, line_length);
}

void transfer_line_to_sender(char* result_buffer, size_t result_buffer_size) {
    printf("переданная строка[%u]: '%.*s'\n",
            static_cast<unsigned>(result_buffer_size),
            static_cast<unsigned>(result_buffer_size),
            result_buffer
        );

}

int main() {
    const size_t buffer_size = 64;
    char buffer[buffer_size] = {};
    const size_t result_buffer_size = buffer_size * 2;
    char result_buffer[result_buffer_size] = {};
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
        printf("Debug: принята строка[%u]: '%.*s'\n", static_cast<unsigned>(line_length), static_cast<unsigned>(line_length), buffer);
        ssize_t transformed_length = transform_line(buffer,  line_length, result_buffer, result_buffer_size);
        if(transformed_length == -1) {
            return -1;
        }
        transfer_line_to_sender(result_buffer, transformed_length);
    }
    return 0;
}
