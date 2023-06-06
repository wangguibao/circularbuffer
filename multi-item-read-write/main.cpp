include <iostream>
#include <thread>
#include "circularbuffer.h"

CircularBuffer<int> buffer(DEFAULT_CIRCULAR_BUFFER_SIZE);

bool running = true;

void writeThread() {
    int meter = 0;
    int buf[10];

    while (running) {
        for (size_t i = 0; i < 10; ++i) {
            buf[i] = meter++;
        }

        buffer.write(buf, 10);
    }
}

void readThread() {
    while (running) {
        int buf[10];
        buffer.read(buf, 10);
#if 1
        for (size_t i = 0; i < 10; ++i) {
            printf("%d\n", buf[i]);
        }
#endif
    }
}

int main()
{
    std::thread t0 = std::thread(writeThread);
    std::thread t1 = std::thread(writeThread);
    std::thread t2 = std::thread(readThread);
    std::thread t3 = std::thread(readThread);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    running = false;

    buffer.stop();

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    return 0;
}
