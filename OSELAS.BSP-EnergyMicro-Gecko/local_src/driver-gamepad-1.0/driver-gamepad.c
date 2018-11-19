/*
 * This is a Linux kernel module for the gamepad.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

/* dev_t structure */
#include <linux/types.h>

/* Functions for obtaining device numbers */
#include <linux/fs.h>

/* cdev structure and its associated helper functions */
#include <linux/cdev.h>

#include <linux/device.h>

/* Functions for resource allocators for I/O ports. */
#include <linux/ioport.h>

/* Functions for interrupt handling */
#include <linux/interrupt.h>

/*
 * Set up driver to handle the platform device“tdt4258”,
 * Special platform device set up for this course and represents the GPIO and sound.
 */
 
static const struct of_device_id my_of_match [] = {
	{.compatible = ”tdt4258 ”,},
	{ },	
};

MODULE_DEVICE_TABLE(of , my_of_match);

static struct platform_driver my_driver = {
	.probe = gamepad_probe,
	.remove = gamepad_remove,
	.driver= {
		.name = ”my”,
		.owner = THIS_MODULE,
		.of_match_table = my_of_match,
	},
};

/*
 * Prototypes
 */
 static int __init gamepad_init(void);
 static int gamepad_probe(struct platform device ∗dev);
 static int gamepad_remove(struct platform device ∗dev);
 static void __exit gamepad_cleanup(void);
 int gamepad_open(struct inode *inode, struct file *filp);
 int gamepad_release(struct inode *inode, struct file *filp);
 static ssize_t read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
 static ssize_t write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
 static int gamepad_fasync(int fd, struct file *filp, int mode);
 static int gamepad_kill_fasync(struct fasync_struct **fa);
 static irqreturn_t gp_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);

DEV_NAME = "Gamepad"
DEV_FIRSTMINOR = 0;
DEV_NUM_COUNT = 1;
GPIO_EVEN_IRQ = 17;
GPIO_ODD_IRQ = 18;

/*
 * Name of device class
 * Each device is represented with a structure of type cdev
 * Device number
 * Fasync structure
 */

struct class ∗cl;
struct cdev gamepad_cdev;
static dev_t gamepad_num;
struct fasync_struct async_queue;

/*
 * File operations initialization
 * The file_operations structure is how a char driver connects operations to device numbers.
 * We migh need to add/remove some of them
 */
 
struct file_operations gamepad_fops = {
	.owner = THIS_MODULE,
	.read = gamepad_read,
	.write = gamepad_write,
	.open = gamepad_open,
	.release =  gamepad_release,
	.fasync = gamepad_fasync,
	.kill_fasync = gamepad_kill_fasync
};

/*
 * gamepad_init: function to insert this module into kernel space
 *
 * Returns 0 if successfull, otherwise -1
 * The module init function will be called immediately
 * The module init function will register its driver details by calling the platform_driver_register() function
 * If there is a matching platform device in the system, the kernel will call the driver probe function
 */

static int __init gamepad_init(void)
{
	platform_driver_register(&my_driver)
	return 0;
}

/*
 * gamepad_probe - function to set up device
 *
 * Requests information from the kernel about I/O register base address, irq numbers etc.
 * Initialize everything
 */
 
static int gamepad_probe(struct platform device ∗dev)
{

/* 
 * Dynamically allocate device numbers
 * 
 * dev is an output-only parameter that will, on successful completion, hold the first number in your allocated range
 * firstminor should be the requested first minor number to use; it is usually 0
 * count is the total number of contiguous device numbers you are requesting
 * Finally, name is the name of the device that should be associated with this number range; it will appear in /proc/devices and sysfs
 * See other file for info on minor and major numbers
 */

// int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name);

alloc_chrdev_region(&gamepad_num, DEV_FIRSTMINOR, DEV_NUM_COUNT, DEV_NAME);

/* 
 * Register the cdev structure that was allocated
 * (If you want to embed the cdev structure within a device-specific structure of your own)
 * The kernel uses structures of type struct cdev to represent char devices internally.
 */
 
 // void cdev_init(struct cdev *cdev, struct file_operations *fops);

 cdev_init(&gamepad_cdev, &gamepad_fops);

/*
 * Set the struct cdev owner field to THIS_MODULE.
 */

cdev.owner = THIS_MODULE;
 
/*
 * Make the device visible to user space
 * This will allow the user program to communicate with the driver by opening a special file in the /dev directory that represents the driver.
 * The user space program can then read and write data to the driver using normal file I/O functions, and the driver will get (or put) the data through the fops defined earlier
 */

cl = class_create(THIS_MODULE, DEV_NAME);
device_create(cl, NULL, gamepad_num, NULL, DEV_NAME);
 
/*
 * Ask for access before using the hardware to prevent several drivers from accessing the same hardware
 * 
 * request_region(unsigned long first, unsigned long n, const char *name);
 * first port
 * number of ports
 * name is the name of the device
 * 
 * Returns NULL if failure, non-NULL if success
 * TODO: handle error, check regions
 */

mem_available_err = request_mem_region(GPIO_PC_BASE, GPIO_PC_LENGTH, DEV_NAME);
if (mem_available_err == NULL) {
	print(KERN_INFO, "Memory region not available")
}

mem_interrupt_available_err = request_mem_region(GPIO_INTERRUPT, GPIO_INTERRUPT_LENGTH, DEV_NAME);
if (mem_interrupt_available_err == NULL) {
	print(KERN_INFO, "Memory region not available")
}

/*
 * Map allocated memory regions to virtual memory
 * Not strictly necessary for this project, but it is convention
 *
 * TODO: handle error
 */

ioremap_nocache(GPIO_PC_BASE, GPIO_PC_LENGTH)
ioremap_nocache(GPIO_INTERRUPT_BASE, GPIO_INTERRUPT_LENGTH);

/*
 * Configure input for the buttons on the gamepad
 */

/* Set pins 0-7 on port C to input */
iowrite32(0x33333333, GPIO_PC_MODEL);

/* Enable internal pull-up */
iowrite32(0xff,GPIO_PC_DOUT);

/*
 * Setup interrupt handling
 */

/* Set interrupt source to pins 0-7 of port C (gamepad buttons) */
iowrite32(0x22222222, GPIO_EXTIPSELL);

/* Set interrupt on 1->0 transition */
iowrite32(0x000000ff, GPIO_EXTIFALL);
/* Set interrupt on 0->1 transition */
iowrite32(0x000000ff, GPIO_EXTIRISE);

/* 
 * Register even and odd interrupt handlers
 *
 * even: irq 17, odd: irq 18
 *
 * int request_irq(unsigned int irq, irqreturn_t (*handler)(), unsigned long flags, const char *dev_name, void *dev_id);
 * 
 * Returns negative number if failure, 0 if success
 * TODO: handle error, make variables for odd and even irq?
 */

request_irq(GPIO_EVEN_IRQ, (irq_handler_t)gp_interrupt_handler, 0, DRIVER_NAME, &driver_cdev);
request_irq(GPIO_ODD_IRQ, (irq_handler_t)gp_interrupt_handler, 0, DRIVER_NAME, &driver_cdev);

/* Enable interrupt generation */
iowrite32(0x000000ff, GPIO_IEN);

/* Clear interrupt flags */
uint32_t clear_value = ioread32(GPIO_IF);
iowrite32(clear_value, GPIO_IFC);

/*
 * Tell the kernel about the cdev structure
 *
 * cdev_add_err = cdev_add(struct cdev *dev, dev_t num, unsigned int count);
 * dev is the cdev structure
 * num is the first device number to which this device responds
 * count is the number of device numbers that should be associated with the device, usually 1

 * Returns negative number if failure
 * You should not call cdev_add until your driver is completely ready to handle operations on the device as it usually succeeds and is momentarily live
 * TODO: handle error
 */

int cdev_add_err;

cdev_add_err = cdev_add(&gamepad_cdev, gamepad_num, 1);

if cdev_add_err
{
	print(KERN_NOTICE, "Error %d adding device%d", err, count)
}

}

/*
 * gamepad_remove - function to cleanup this module from kernel space
 */

static int gamepad_remove(struct platform device ∗dev)
{

/* Disable interrupts and free irq */
iowrite32(0x0, GPIO_IEN);

//void free_irq(unsigned int irq, void *dev_id);
free_irq(GPIO_EVEN_IRQ, &driver_cdev);
free_irq(GPIO_ODD_IRQ, &driver_cdev);

/* Free device numbers */

// void unregister_chrdev_region(dev_t first, unsigned int count);
unregister_chrdev_region(gamepad_num, DEV_NUM_COUNT);

/* Return I/O ports to the system */

//void release_region(unsigned long start, unsigned long n);
release_region();

/* Remove device from the system */

// void cdev_del(struct cdev *dev);
cdev_del(&gamepad_cdev);

device_destroy(cl, device_number);
class_destroy(cl);
}

/*
 * gamepad_cleanup - function to cleanup this module from kernel space
 *
 * This is the second of two exported functions to handle cleanup this
 * code from a running kernel
 */

static void __exit gamepad_cleanup(void)
{
	platform_driver_unregister(&my_driver)
}

/*
 * The open method is provided for a driver to do any initialization in preparation for later operations.
 * In most drivers, open should perform the following tasks:
 * Check for device-specific errors (such as device-not-ready or similar hardware problems)
 * Initialize the device if it is being opened for the first time
 * Update the f_op pointer, if necessary
 * Allocate and fill any data structure to be put in filp->private_data
 */
 
int gamepad_open(struct inode *inode, struct file *filp)
{
	print(KERN_NOTICE, "What should this method do?")
	
	return 0 //success!
}

/*
 *The role of the release method is the reverse of open. Sometimes you’ll find that the method implementation is called device_close instead of device_release. Either way, the device method should perform the following tasks:
 *Deallocate anything that open allocated in filp->private_data
 *Shut down the device on last close
 */

int gamepad_release(struct inoded *inode, struct file *flip)
{
	print(KERN_NOTICE, "What should this method do?")
	
	return 0 //success!
}

/*
 * The task of the read method is to copy data from the device to user space (using copy_to_user),
 * while the write method must copy data from user space to the device (using copy_from_user).
 * Each read or write system call requests transfer of a specific number of bytes, but the driver is free to transfer less data
 * the exact rules are slightly different for reading and writing and are described later in this chapter.
 * For both methods, filp is the file pointer and count is the size of the requested data transfer.
 * The buff argument points to the user buffer holding the data to be written or the empty buffer where the newly read data should be placed.
 * Finally, offp is a pointer to a “long offset type” object that indicates the file position the user is accessing.
 * The return value is a “signed size type”; its use is discussed later.
 * Both the read and write methods return a negative value if an error occurs.
 */

static ssize_t read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	/* Read from GPIO_PC_DIN to get status of input pins */
	uint32_t reading = ioread32(GPIO_PC_DIN);
	
	/*
	 * Copy reading from kernel space to user space
	 * TODO: error handling?
	 */
	
	//unsigned long copy_to_user(void __user *to, const void *from, unsigned long count);
	copy_to_user(buff, &reading, 1)
	
	// return the amount of memory still to be copied p. 23 chapter 3
	
}

// Do we need write?
static ssize_t write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
}

static int gamepad_fasync(int fd, struct file *filp, int mode)
{
	// int fasync_helper(int fd, struct file *filp, int mode, struct fasync_struct **fa);
	return fasync_helper(fd, filp, mode, &async_queue);
}

static int gamepad_kill_fasync(struct fasync_struct **fa)
{
	return kill_fasync(fa, SIGIO, POLL_IN);
}

/*
 * Interrupt handler
 */

static irqreturn_t gp_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	/* Find source of interrupt by reading the GPIO_IF register (DONT KNOW ITF NEEDED) */
	ioread32(GPIO_IF)
	
	/* Clear interrupt flags */
	uint32_t clear_value = ioread32(GPIO_IF);
	iowrite32(clear_value, GPIO_IFC);
	
	if (async_queue) {
		gamepad_kill_fasync(&async_queue);
	}
	
	return IRQ_HANDELED
}

/* Let the kernel know which functions are init and exit */
module_init(gamepad_init);
module_exit(gamepad_cleanup);

/* Licence */
MODULE_DESCRIPTION("Gamepad module.");
MODULE_LICENSE("GPL");

