
#include <cstdio>
#include <cstring>


// return
// 0 - ошибок нет
// -1 - ошибка ввода/вывода
// -2 - EOF
int skip_line(FILE* stream) {
    int c = 0;
    while((c = getc(stream)) != EOF) {
        if(static_cast<char>(c)=='\n') {
            return 0;
        }
    }
    if(ferror(stream)!=0) {
        return -1;
    }
    return -2;
}



int main() {

    const size_t input_chars_max  = 4;
    const size_t buffer_size = input_chars_max +2; // +2 - \n\0
    char buffer[buffer_size];

    printf("введите строку: ");
    fflush(stdout);
// 1) короткая строка будет \n, \0
// 2) влезла в буфер не будет \n будет \0  fgets вернёт хвост
// 3) не влезла в буфер  будет \0 не будет \n    fgets вернёт хвост


    while(fgets(buffer, buffer_size, stdin) !=0) {
        size_t line_length = strlen(buffer);
        if(buffer[line_length -1] == '\n') { // случай №1
            buffer[line_length -1] = '\0';
            printf("buffer - '%.*s'\n", static_cast<int>(buffer_size), buffer);
            // TODO: process data 
        }
        else {  // случай 2-3
            
            if(skip_line(stdin)==-1) {
                perror("skip_line");
                return -1;
            }
            printf("ошибка: превышено количество символов(>%u)\n", static_cast<unsigned>(input_chars_max));

        }
        printf("введите строку: ");
        fflush(stdout);
    }
    return 0;
}