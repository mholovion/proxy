#include "message_queue.hpp"

message_queue::message_queue(size_t capacity)
    : capacity_(capacity), head_(0), tail_(0) {
    buffer_ = new uint64_t[capacity_];
}

message_queue::~message_queue() {
    delete[] buffer_;
}

bool message_queue::push(uint64_t message_id) {
    size_t tail = tail_.load(std::memory_order_relaxed);
    size_t head = head_.load(std::memory_order_acquire);

    size_t next_tail = (tail + 1) % capacity_;

    if (next_tail == head) {
        return false;
    }

    buffer_[tail] = message_id;
    tail_.store(next_tail, std::memory_order_release);
    return true;
}

bool message_queue::pop(uint64_t& message_id) {
    size_t head = head_.load(std::memory_order_relaxed);
    size_t tail = tail_.load(std::memory_order_acquire);

    if (head == tail) {
        return false;
    }

    message_id = buffer_[head];
    size_t next_head = (head + 1) % capacity_;
    head_.store(next_head, std::memory_order_release);
    return true;
}
