#ifndef MSGSLOT_H
#define MSGSLOT_H

#include <linux/ioctl.h>

// The major device number.
// We don't rely on dynamic registration
// any more. We want ioctls to know this
// number at compile time.
#define MAJOR_NUM 235
#define MAX_MINORS 256

// Set the message of the device driver
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)

#define BUFFER_SIZE 128 // Assuming max message size is 128 bytes

#define DEVICE_RANGE_NAME "message_slot"


// Help structs definition
typedef struct channel_node {
    int minor;
    int channel_id; 
    char msg_buffer[BUFFER_SIZE]; 
    int msg_size;
    struct channel_node* next;
} channel_node;

typedef struct file_private_info {
    unsigned int channel_id;
    int minor;
} file_private_info;

#endif
