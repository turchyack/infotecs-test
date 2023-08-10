#include <errno.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <condition_variable>

const size_t DATA_SIZE_MAX = 64;


// return
// -1 - ошибка ввода/вывода
// -2 - ошибка - введена пустая строка
// -3 - ошибка - строка слишком длинная
// -4 - ошибка - не цифра
// 0 - конец потока
// >0 - принята строка цифр, значение представляет длину строки
//
ssize_t input_digits_line(FILE* stream, char* buffer, size_t buffer_size) {
    printf("input: ");
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

// return
// > 0 - длина результирующей строки('\0' не добавляется)
// -1 ошибка - недостаточно места в буфере
//
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
ssize_t transform_data(const char* line, size_t line_length, char* result_buffer, size_t result_buffer_size) {
    if(result_buffer_size < line_length) {
        return -1;
    }
    memcpy(result_buffer, line, line_length * sizeof(line[0]));
    std::sort(result_buffer, &result_buffer[line_length], std::greater<char>());

    return subtitude_even_digits(result_buffer,  result_buffer_size, line_length);
}

unsigned calc_sum(char* data, size_t data_length) {
    unsigned sum = 0;
    for(size_t i = 0; i < data_length; ++i) {
        if(isdigit(data[i])) {
            sum += data[i] -'0';
        }
    }
    return sum;
}

class Sender {
public:
    Sender();
    ~Sender();
    void stop();
    void signal();
    void send(char* buffer, size_t buffer_size);
private:
    static void main_wrapper(Sender* self) {
        self->main();
    }
    void main();
    static const size_t buffer_size = DATA_SIZE_MAX*2;
    char shared_buffer[buffer_size] = {0};
    size_t shared_data_length = 0;
    bool sender_work;
    std::thread thread;
    std::mutex m;
    std::condition_variable cv;
};

Sender::Sender() :  sender_work(true), thread(main_wrapper, this) {
}

Sender::~Sender() {
    if(sender_work) {
        stop();
    }
}

void Sender::main() {
    char send_data[buffer_size] = {};
    size_t length = 0;
    std::unique_lock lk(m);

    while(sender_work) {
       // fprintf(stderr, "debug wait for cv\n");
        cv.wait(lk);
        if(shared_data_length !=0) {
            memcpy(send_data, shared_buffer, shared_data_length);
            length = shared_data_length;
            memset(shared_buffer, 0, shared_data_length*sizeof(shared_buffer[0]));
            shared_data_length = 0;
        }
        else {
            continue;
        }

        lk.unlock();
        fprintf(stderr, "do long processing\n");
        {
            fprintf(stderr, "sender input data: '%.*s'\n", static_cast<unsigned>(length), send_data);
            unsigned sum = calc_sum(send_data, length);   // тут ставим соединение
            fprintf(stderr, "data sum = %u\n", sum);


            sleep(1);

        }

        lk.lock();
    }


    fprintf(stderr,"debug end of thread\n");
}

void Sender::stop() {
    fprintf(stderr,"stop\n");
    sender_work = false;
    cv.notify_one();
    thread.join();
}

void Sender::send(char* data, size_t data_length) {

    if(buffer_size < data_length) {
        fprintf(stderr,"buffer too small\n");
    }
    std::unique_lock lk(m);

    memcpy(shared_buffer, data, data_length);
    this->shared_data_length = data_length;
    lk.unlock();
    cv.notify_one();

}


int main() {

    Sender sender;

    // обработка данных потоком №1
    const size_t buffer_size = DATA_SIZE_MAX;
    char buffer[buffer_size] = {};
    const size_t result_buffer_size = buffer_size * 2;
    char result_buffer[result_buffer_size] = {};
    ssize_t line_length = 0;
    bool stop_flag = false;
    while(!stop_flag && (line_length = input_digits_line(stdin, buffer, buffer_size)) != 0) {
        if(line_length < 0) {
            switch(line_length) {
            case -1:
                fprintf(stderr, "error: IO error - %s\n", strerror(errno));
                return -1;
            case -2:
                fprintf(stderr, "error: input empty string\n");
                stop_flag = true;
                continue;
            case -3:
                fprintf(stderr, "error: too long string\n");
                continue;
            default:
                fprintf(stderr, "error: wrong number\n");
                continue;
            }

        }

        ssize_t transformed_length = transform_data(buffer,  line_length, result_buffer, result_buffer_size);
        if(transformed_length == -1) {
            return -1;
        }
        sender.send(result_buffer, transformed_length);
    }
    fprintf(stderr, "debug: wait for join\n");
    sender.stop();
    fprintf(stderr, "debug: joined\n");
    return 0;
}
