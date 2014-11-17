#ifndef MESSAGE_INCLUDE
#define MESSAGE_INCLUDE

#include <cstdint>

typedef struct {
    uint32_t message_number;
    char user_name[8];
    char message[138];
} __attribute__((packed)) messagePacket;

// __attribute__((packed)) ensures, that the compiler adds no padding bytes to the struct.
// It's guaranteed to be 150 bytes long.

#endif //MESSAGE_INCLUDE
