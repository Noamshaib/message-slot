// Declare what kind of code we want
// from the header files. Defining __KERNEL__
// and MODULE allows us to access kernel-level
// code not usually available to userspace programs.
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>     // for kmalloc and kfree

//Our custom definitions of IOCTL operations
#include "message_slot.h"

MODULE_LICENSE("GPL");

// Prototypes for device functions
static int device_open(struct inode* inode, struct file*  file);
static int device_release(struct inode* inode, struct file*  file);
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset);
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset);
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param);

// Prototypes for help functions
struct channel_node* add_channel_node(struct channel_node** channel_slots, int minor, int channel_id);
struct channel_node* search_channel_node(struct channel_node** channel_slots, int minor, int channel_id);
int free_linked_lst(struct channel_node* head);

// Global array of channel node pointers
channel_node* channel_slots[MAX_MINORS] = {NULL};


//================== DEVICE FUNCTIONS ===========================

static int device_open(struct inode *inode, struct file *file){
  file_private_info* priv_data;
  int the_minor = iminor(inode);

  // Allocate memory for file-specific private data
  priv_data = kmalloc(sizeof(file_private_info), GFP_KERNEL);
  if (priv_data == NULL) {
    return -ENOMEM; // Out of memory
  }

  // Initialize private data
  priv_data->minor = the_minor;
  priv_data->channel_id = 0;  // Default channel ID

  // Store the private data in the file structure
  file->private_data = priv_data;

  return 0; // Success
}

static int device_release(struct inode* inode, struct file*  file)
{
    /* printk("%s: Initiating 'device_open'.\n", DEVICE_FILE_NAME); */
    kfree(file -> private_data);
    return 0; // Success
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset){
  int i;
  unsigned int channel_id;
  struct channel_node* node;
  struct file_private_info* priv_data;
  ssize_t msg_size;

  // Retrieve private data (minor number and channel ID)
  priv_data = (struct file_private_info*)file->private_data;
  channel_id = priv_data->channel_id;

  if (channel_id == 0 || buffer == NULL) {
    return -EINVAL; // Invalid request
  }

  // search for the corresponding channel node
  node = search_channel_node(channel_slots, priv_data->minor, channel_id);
  if (node == NULL) {
    return -EWOULDBLOCK; // No message available
  }

  msg_size = node->msg_size;
  if (msg_size == 0) {
    return -EWOULDBLOCK; // No message in the channel
  }

  if (length < msg_size) {
    return -ENOSPC; // User buffer too small
  }

  // Copy message to user buffer
  for (i = 0; i < msg_size; i++) {
    if (put_user(node->msg_buffer[i], &buffer[i]) != 0) {
      return -EFAULT; // Failed to copy data to user space
    }
  }

  // Return the number of bytes read
  return i;
}


//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset){
  int i;
  unsigned int channel_id;
  struct channel_node* node;
  struct file_private_info* priv_data;

  // Retrieve private data (minor number and channel ID)
  priv_data = (struct file_private_info*)file->private_data;
  channel_id = priv_data->channel_id;

  if (channel_id == 0) {
    return -EINVAL; // Invalid channel ID
  }

  if (buffer == NULL) {
    return -EINVAL; // Null buffer is invalid
  }

  if (length <= 0 || length > BUFFER_SIZE) {
    return -EMSGSIZE; // Invalid message size
  }


  // search for or create the corresponding channel node
  node = (channel_node*) search_channel_node(channel_slots, priv_data->minor, channel_id);
  if (node == NULL) {
    node = (channel_node*) add_channel_node(channel_slots, priv_data->minor, channel_id);
    if (node == NULL) {
      return -ENOMEM; // Failed to add node
    }
  }

  // Write message to channel node's message buffer
  for (i = 0; i < length; i++) {
    if (get_user(node->msg_buffer[i], &buffer[i]) != 0) {
      return -EFAULT; // Failed to copy data from user space
    }
  }

  // Update message size
  node->msg_size = i;

  // Return the number of bytes written
  return i;
}


//----------------------------------------------------------------
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param){
  struct file_private_info *priv_data;

  // Ensure the IOCTL command is correct
  if (ioctl_command_id == MSG_SLOT_CHANNEL) {
    // Validate the IOCTL parameter (channel_id should not be 0)
    if (ioctl_param == 0) {
      return -EINVAL; // Invalid argument
    }

    // Retrieve private data from the file structure
    priv_data = (struct file_private_info *)file->private_data;
    if (!priv_data) {
      return -EINVAL; // Invalid file private data
    }

    // Set the channel ID in the private data
    priv_data->channel_id = ioctl_param;

    return 0; // Success
  }

  return -EINVAL; // Invalid IOCTL command
}


//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops = {
  .owner	        = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  .release        = device_release,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device

static int __init device_init(void){
  int rc = -1;

  // Register the character device
  rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);

  // Check for error in registration
  if (rc < 0) {
    //printk(KERN_ERR "%s registration failed for %d\n", DEVICE_RANGE_NAME, MAJOR_NUM);
    return rc;
  }

  //printk(KERN_INFO "%s: Device registered successfully with major number %d\n", DEVICE_RANGE_NAME, MAJOR_NUM);
  return 0; // Success
}

static void __exit device_cleanup(void){
  int i;

  // Free all channel nodes in the channel_slots array
  for (i = 0; i < MAX_MINORS; i++) {
    free_linked_lst(channel_slots[i]); 
  }

  // Unregister the character device
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
  //printk(KERN_INFO "%s: Device unregistered successfully\n", DEVICE_RANGE_NAME);
}

//========================= HELP FUNCS =========================

// add a new channel node for a specific minor and channel_id
struct channel_node* add_channel_node(struct channel_node** channel_slots, int minor, int channel_id){
    struct channel_node* node = kmalloc(sizeof(struct channel_node), GFP_KERNEL);
    if (node == NULL) {
        // Memory allocation failed
        return NULL;
    }
    node->minor = minor;
    node->channel_id = channel_id;
    node->msg_size = 0;
    memset(node->msg_buffer, 0, BUFFER_SIZE); // Initialize message buffer

    // add the new node at the beginning of the list
    node->next = channel_slots[minor];
    channel_slots[minor] = node;

    return node;
}

// search for channel node based on minor and channel_id
struct channel_node* search_channel_node(struct channel_node** channel_slots, int minor, int channel_id){
    struct channel_node* curr_node = channel_slots[minor];
    while (curr_node != NULL) {
        if (curr_node->channel_id == channel_id) {
            return curr_node;
        }
        curr_node = curr_node->next;
    }
    return NULL;
}

// Free the linked list of channel nodes
int free_linked_lst(struct channel_node* head) {
    struct channel_node* curr_node = head;
    struct channel_node* next_node;

    while (curr_node != NULL) {
        next_node = curr_node->next;
        kfree(curr_node);
        curr_node = next_node;
    }

    return 0; // Success
}


//---------------------------------------------------------------
module_init(device_init);
module_exit(device_cleanup);



