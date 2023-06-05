#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <sys/types.h>
#include <mutex>
#include <condition_variable>
#include <iostream>

const int DEFAULT_CIRCULAR_BUFFER_SIZE = 64;

template<typename T>
class CircularBuffer
{
public:
    explicit CircularBuffer(size_t capacity) : capacity(capacity) {
        buffer = new T[capacity];
    }

    ~CircularBuffer() {
        std::cout << "~CircularBuffer" << std::endl;
        if (buffer) {
            delete buffer;
        }
    }

    T read() {
        std::unique_lock<std::mutex> lock(mutex);

        while (readableSize() < 1 && running) {
            cond.wait(lock, [this] () {return (readableSize() > 0 || running == false);});

            if (!running) {
                return T();
            }

            if (readableSize() < 1) {
                continue;
            }
        }

        T v = buffer[readPos];
        readPos = (readPos + 1)% capacity;

        cond.notify_one();

        return v;
    }

    bool tryRead(T& v) {
        std::unique_lock<std::mutex> lock(mutex);

        if (readableSize() < 1) {
            return false;
        }

        v = buffer[readPos];

        readPos = (readPos + 1) % capacity;
        cond.notify_one();
        return true;
    }

    void write(T v) {
        std::unique_lock<std::mutex> lock(mutex);

        while (writableSize() < 1 && running) {
            cond.wait(lock, [this] () {return (writableSize() > 0 || running == false);});

            if (!running) {
                return;
            }

            if (writableSize() < 1) {
                continue;
            }
        }

        buffer[writePos] = v;

        writePos = (writePos + 1) % capacity;
        cond.notify_one();
    }

    bool tryWrite(T v) {
        std::unique_lock<std::mutex> lock(mutex);

        if (writableSize() < 1) {
            return false;
        }
        buffer[writePos] = v;

        writePos = (writePos + 1) % capacity;
        cond.notify_one();
    }

    // Ugly
    void stop() {
        running = false;
        cond.notify_all();
    }

public:
    inline size_t readableSize() {
        int size = writePos - readPos;
        if (size < 0) {
            size += capacity;
        }

        return size;
    }

    inline size_t writableSize() {
        int size = readPos - writePos - 1;
        if (size < 0) {
            size += capacity;
        }
        return size;
    }

private:
    size_t capacity = 0;

    T* buffer = nullptr;
    std::mutex mutex;
    std::condition_variable cond;
    size_t writePos = 0;
    size_t readPos = 0;

    bool running = true;
};

#endif // CIRCULARBUFFER_H
