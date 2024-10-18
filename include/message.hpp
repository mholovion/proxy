#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint> 

#pragma pack(push, 1)
struct message {
    uint16_t message_size;
    uint8_t message_type;
    uint64_t message_id;
    uint64_t message_data;
};
#pragma pack(pop)


#endif
