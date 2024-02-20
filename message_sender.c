#include "message_slot.h"    

#include <fcntl.h>  /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>      /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
    int fd;
    unsigned int channel_id;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <device_file_path> <channel_id> <message>\n", argv[0]);
        return 1;
    }

    // Convert channel ID from string to unsigned int
    channel_id = (unsigned int) strtoul(argv[2], NULL, 10);

    // Open the device file
    fd = open(argv[1], O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device file");
        return 1;
    }

    // Set the channel ID
    if (ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0) {
        perror("Failed to set the channel ID");
        close(fd);
        return 1;
    }

    // Write the message
    if (write(fd, argv[3], strlen(argv[3])) < 0) {
        perror("Failed to write the message");
        close(fd);
        return 1;
    }

    // Close the device file
    if (close(fd) < 0) {
        perror("Failed to close the device file");
        return 1;
    }

    exit(0);
}
