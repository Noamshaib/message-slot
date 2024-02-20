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
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <device_file_path> <channel_id>\n", argv[0]);
        return 1;
    }

    // Convert channel ID from string to unsigned int
    channel_id = (unsigned int)strtoul(argv[2], NULL, 10);

    // Open the device file
    fd = open(argv[1], O_RDWR);
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

    // Read the message to a buffer
    memset(buffer, 0, BUFFER_SIZE); // Clear the buffer
    bytes_read = read(fd, buffer, BUFFER_SIZE - 1); // Leave room for null terminator
    if (bytes_read < 0) {
        perror("Failed to read the message");
        close(fd);
        return 1;
    }

    // Close the device file
    if (close(fd) < 0) {
        perror("Failed to close the device file");
        return 1;
    }

    // Print the message
    if (write(STDOUT_FILENO, buffer, bytes_read) < 0) {
        perror("Failed to write the message to stdout");
        return 1;
    }

    
    exit(0);
}
