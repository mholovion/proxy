#include "message_container.hpp"
#include <cstring>

message_container::message_container(size_t capacity)
    : capacity_(capacity) {
    slots_ = new slot[capacity_];
    for (size_t i = 0; i < capacity_; ++i) {
        slots_[i].message_id.store(0, std::memory_order_relaxed);
    }
}

message_container::~message_container() {
    delete[] slots_;
}

size_t message_container::hash(uint64_t message_id) const {
    return message_id % capacity_;
}

bool message_container::find_slot(uint64_t message_id, size_t& idx, bool find_empty) {
    size_t index = hash(message_id);

    for (size_t i = 0; i < capacity_; ++i) {
        idx = (index + i) % capacity_;
        uint64_t current_id = slots_[idx].message_id.load(std::memory_order_acquire);

        if (current_id == message_id) {
            return true;
        } else if (current_id == 0) {
            if (find_empty) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

bool message_container::insert(const message& msg) {
    size_t idx;
    if (!find_slot(msg.message_id, idx, true)) {
        return false;
    }

    uint64_t expected = 0;
    if (slots_[idx].message_id.compare_exchange_strong(
            expected, msg.message_id, std::memory_order_acq_rel)) {
        std::memcpy(&slots_[idx].msg, &msg, sizeof(message));
        return true;
    } else if (expected == msg.message_id) {
        return false;
    } else {
        return insert(msg);
    }
}

bool message_container::get(uint64_t message_id, message& msg) {
    size_t idx;
    if (find_slot(message_id, idx, false)) {
        std::memcpy(&msg, &slots_[idx].msg, sizeof(message));
        return true;
    }
    return false;
}

bool message_container::exists(uint64_t message_id) {
    size_t idx;
    return find_slot(message_id, idx, false);
}
