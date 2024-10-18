#ifndef MESSAGE_QUEUE_HPP
#define MESSAGE_QUEUE_HPP

#include <cstdint>
#include <cstddef>
#include <atomic>

class message_queue {
public:
    explicit message_queue(size_t capacity = 1024);
    ~message_queue();

    bool push(uint64_t message_id);
    bool pop(uint64_t& message_id);

private:
    uint64_t* buffer_;
    size_t capacity_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

#endif