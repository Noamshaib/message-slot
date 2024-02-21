# Message Slot Kernel Module

## Overview

This project is part of an operating systems course, focusing on kernel programming and inter-process communication (IPC). It involves the implementation of a kernel module that provides a new IPC mechanism known as a message slot, which is a character device file allowing processes to communicate through multiple message channels. The project includes a kernel module and user-space applications, written in C, designed to run on a Linux system (tested on VirtualBox) and requires root permissions for certain operations.

## Components

### 1. `message_slot.c`
   - The core kernel module file.
   - Implements the message slot as a character device.
   - Manages multiple message channels, allowing processes to communicate via these channels.

### 2. `message_slot.h`
   - Header file for the kernel module.
   - Contains necessary definitions and declarations for `message_slot.c`.

### 3. `message_sender.c`
   - A user-space application to send messages to a specified channel of the message slot device.
   - Accepts command line arguments for the device file path, target message channel ID, and the message to be sent.

### 4. `message_reader.c`
   - A user-space application to read messages from a specified channel of the message slot device.
   - Accepts command line arguments for the device file path and the target message channel ID.

### 5. `Makefile`
   - Used to compile the kernel module and user-space applications.
   - Includes targets for building and cleaning the project.

## Language

- The project is implemented in C.

## Error Handling

- The code robustly handles errors and sets the `errno` variable appropriately in various failure scenarios, such as invalid arguments or memory allocation failures.

## Running Environment

- The project is designed to be run on a virtualized Linux environment using VirtualBox.
- Root permissions are required for loading the kernel module and creating device files.

## Assignment Description

The assignment aims to deepen understanding of kernel module development, IPC mechanisms, and device drivers in Linux. The message slot device uses a hard-coded major number (235) and manages different message slots using minor numbers. It supports essential file operations like `open`, `ioctl`, `read`, and `write`, each with specific behaviors and error handling as per the assignment's specifications.

## Example Usage

1. Load the kernel module.
2. Create message slot device files.
3. Change file permissions as necessary.
4. Use `message_sender` to send messages to a channel.
5. Use `message_reader` to read messages from a channel.

---



## Test Instructions for Message Slot Kernel Module

### Setting Up and Running Tests

1. **Clone the Project**:
   - Clone the project repository to a Linux environment.
   - Open a command line interface in the directory where the project is cloned.

2. **Compile the Kernel Module**:
   - Run `make` in the command line.
   - This should compile the kernel module and create `message_slot.ko`.

3. **Insert the Kernel Module**:
   - Execute `sudo insmod message_slot.ko` to insert the kernel module into the kernel.
   
4. **Verify Module Insertion**:
   - Check if the module is correctly inserted by running `lsmod | grep message_slot`.
   - This command should list the `message_slot` module, indicating successful insertion.

5. **Create Device Nodes**:
   - Create device nodes using `mknod`:
     `sudo mknod /dev/test0 c 235 0`
     `sudo mknod /dev/test1 c 235 1`
   - These commands create two device nodes, `/dev/test0` and `/dev/test1`, with major number 235.

6. **Set Permissions**:
   - Change permissions for the device nodes to allow read/write operations:
     `sudo chmod 0777 /dev/test0`
     `sudo chmod 0777 /dev/test1`

7. **Verify Device Node Permissions**:
   - Check the permissions of the device nodes:
     `ls -ls /dev/test0`
     `ls -ls /dev/test1`
   - You should see full permissions (rwx for user, group, and others) for both device nodes.

8. **Compile the Test Program**:
   - Compile your test program (assuming `hw3_tester.c` is your test script):
     `gcc -O3 -Wall -std=c11 hw3_tester.c -o tester`

9. **Run the Test Program**:
   - Execute the test program by running `./tester`.
   - The test should pass all 14 tests and print `DONE!` upon successful completion.

### Cleaning Up After Tests

1. **Remove the Kernel Module**:
   - To unload the kernel module, run `sudo rmmod message_slot.ko`.

2. **Remove Device Nodes**:
   - Delete the created device nodes:
     `sudo rm /dev/test0`
     `sudo rm /dev/test1`

### Notes

- Ensure that you have root permissions for operations that require it, such as inserting modules, creating device nodes, and changing file permissions.
- The kernel module and test program should be executed in a controlled environment, preferably a virtual machine, to prevent any unintended system issues.
- Make sure to follow all steps in sequence to ensure accurate testing.

---
