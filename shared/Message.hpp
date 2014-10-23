#ifndef MESSAGE_INCLUDE
#define MESSAGE_INCLUDE

#include <cstdint>

typedef struct {
    uint32_t message_number;
    char user_name[8];
    char message[138];
} message_t;

#endif //MESSAGE_INCLUDE
