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

    bool read(T* buf, size_t size) {
        std::unique_lock<std::mutex> lock(mutex);

        while (readableSize() < size && running) {
            cond.wait(lock, [this, size] () {return (readableSize() >= size || running == false);});

            if (!running) {
                return false;
            }

            if (readableSize() < size) {
                continue;
            }
        }

        for (size_t i = 0; i < size; ++i) {
            buf[i] = buffer[readPos];
            readPos = (readPos + 1)% capacity;
        }

        cond.notify_one();

        return true;
    }

    bool tryRead(T* buf, size_t size) {
        std::unique_lock<std::mutex> lock(mutex);

        if (readableSize() < size) {
            return false;
        }

        for (size_t i = 0; i < size; ++i) {
            buf[i] = buffer[readPos];
            readPos = (readPos + 1) % capacity;
        }

        cond.notify_one();
        return true;
    }

    bool write(T* buf, size_t size) {
        std::unique_lock<std::mutex> lock(mutex);

        while (writableSize() < size && running) {
            cond.wait(lock, [this, size] () {return (writableSize() > size || running == false);});

            if (!running) {
                return false;
            }

            if (writableSize() < size) {
                continue;
            }
        }

        for (size_t i = 0; i < size; ++i) {
            buffer[writePos] = buf[i];
            writePos = (writePos + 1) % capacity;
        }
        cond.notify_one();

        return true;
    }

    bool tryWrite(T* buf, size_t size) {
        std::unique_lock<std::mutex> lock(mutex);

        if (writableSize() < size) {
            return false;
        }

        for (size_t i = 0; i < size; ++i) {
            buffer[writePos] = buf[i];
            writePos = (writePos + 1) % capacity;
        }
        cond.notify_one();

        return true;
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
