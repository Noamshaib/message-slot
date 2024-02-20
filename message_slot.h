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

#endif
