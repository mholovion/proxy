#ifndef MESSAGE_CONTAINER_HPP
#define MESSAGE_CONTAINER_HPP

#include "message.hpp"
#include <cstdint>
#include <cstddef>
#include <atomic>

class message_container {
public:
    explicit message_container(size_t capacity);
    ~message_container();

    bool insert(const message& msg);
    bool get(uint64_t message_id, message& msg);
    bool exists(uint64_t message_id);

private:
    struct slot {
        std::atomic<uint64_t> message_id;
        message msg;
    };

    slot* slots_;
    size_t capacity_;

    size_t hash(uint64_t message_id) const;

    bool find_slot(uint64_t message_id, size_t& idx, bool find_empty);

};

#endif
