#include <iostream>
#include <thread>
#include "circularbuffer.h"

CircularBuffer<int> buffer(DEFAULT_CIRCULAR_BUFFER_SIZE);

bool running = true;

void writeThread() {
    int meter = 0;

    while (running) {
        buffer.write(meter++);
    }
}

void readThread() {
    while (running) {
        int value = buffer.read();
#if 1
        printf("%d\n", value);
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
