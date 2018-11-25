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
#include <linux/io.h>

/* Functions for interrupt handling */
#include <linux/interrupt.h>

#include "driver-gamepad.h"
#include <linux/platform_device.h>
#include <linux/signal.h>
#include <asm/uaccess.h>


/*#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <asm/uaccess.h>
#include <asm/siginfo.h>
#include <linux/fcntl.h>
#include <linux/ioctl.h>*/

/* Prototypes */
static int gamepad_probe(struct platform_device *dev);
static int gamepad_remove(struct platform_device *dev);
static int gamepad_open(struct inode *inode, struct file *filp);
static int gamepad_release(struct inode *inode, struct file *filp);
static ssize_t gamepad_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
static int gamepad_fasync(int fd, struct file *filp, int mode);
static irqreturn_t gp_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);

/* Module parameters */
#define DEV_NAME "gamepad"
#define DEV_FIRSTMINOR 0
#define DEV_NUM_COUNT 1

/* Module variables */
static struct class *gamepad_cl;
static struct cdev *gamepad_cdev;
static dev_t gamepad_num;
static struct fasync_struct *gamepad_async_queue;
struct resource *res;
volatile void *gpio_start;

/* File operations initialization */
 
static struct file_operations gamepad_fops = {
	.owner = THIS_MODULE,
	.read = gamepad_read,
	.open = gamepad_open,
	.release = gamepad_release,
	.fasync = gamepad_fasync,
};

/*
 * Set up driver to handle the platform device“tdt4258”,
 * Special platform device set up for this course and represents the GPIO and sound.
 */
 
static const struct of_device_id gamepad_of_match[] = {
		{.compatible = "tdt4258",},
		{},
};

MODULE_DEVICE_TABLE(of, gamepad_of_match);

static struct platform_driver gamepad_driver = {
		.probe = gamepad_probe,
		.remove = gamepad_remove,
		.driver = {
			.name = "gamepad",
			.owner = THIS_MODULE,
			.of_match_table = gamepad_of_match,
		},
};

/*
 * gamepad_init: function to insert this module into kernel space
 *
 * Returns 0 if successfull, otherwise -1
 */

static int gamepad_init(void)
{	
	int pdr;
	
	if ((pdr = platform_driver_register(&gamepad_driver)) == -1) {
		printk(KERN_INFO "ERROR: %d: Failed to register platform device.", pdr);
		return pdr;
	}
	
	return 0;
}

/*
 * gamepad_probe: function to set up device
 */
 
static int gamepad_probe(struct platform_device *dev)
{
	int retval;
	unsigned long clear_value;
	int gpio_even_irq;
	int gpio_odd_irq;

	/* Dynamically allocate device numbers */

	// int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name); Returns negative number if unsuccessful.
	if ((retval = alloc_chrdev_region(&gamepad_num, DEV_FIRSTMINOR, DEV_NUM_COUNT, DEV_NAME)) < 0) {
		printk(KERN_INFO "Error %d: Failed to allocate char device region.", retval);
	}

	/*
	 * Register the cdev structure that was allocated
	 * (If you want to embed the cdev structure within a device-specific structure of your own)
	 * The kernel uses structures of type struct cdev to represent char devices internally.
	 */

	 // void cdev_init(struct cdev *cdev, struct file_operations *fops);
	 cdev_init(gamepad_cdev, &gamepad_fops);

	/* Set the struct cdev owner field to THIS_MODULE */

	gamepad_cdev.owner = THIS_MODULE;
	gamepad_cdev.ops = &gamepad_fops;
	
	/* Tell the kernel about the cdev structure */

	// cdev_add_err = cdev_add(struct cdev *dev, dev_t num, unsigned int count);
	if ((retval = cdev_add(gamepad_cdev, gamepad_num, 1)) < 0) {
		printk(KERN_INFO "Error %d: Failed to add cdev.", retval);
		return retval;
	}

	/* Make the device visible to user space to allow the user program to communicate with the driver */

	gamepad_cl = class_create(THIS_MODULE, DEV_NAME);
	device_create(gamepad_cl, NULL, gamepad_num, NULL, DEV_NAME);

	/* Find the I/O register base address for the TDT4258 device */
	
	// struct resource ∗res = platform get resource(dev, IORESOURCEMEM, index)
	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	gpio_start = (volatile void *) res->start;

	/*
	 * Ask for access before using the hardware to prevent several drivers from accessing the same hardware
	 *
	 * Returns NULL if failure, non-NULL if success
	 * TODO: handle error
	 */
	
	// request_region(unsigned long first, unsigned long n, const char *name);
	request_mem_region(res->start, resource_size(res), DEV_NAME);
	
	/*
	 * Map allocated memory regions to virtual memory
	 * Not strictly necessary for this project, but it is convention
	 * 
	 * TODO: handle error
	 */
	
	// ioremap(resource_size_t offset, unsigned long size)
	ioremap_nocache((phys_addr_t)res->start, resource_size(res));
		
	/* Configure input for the buttons on the gamepad */

	/* Set pins 0-7 on port C to input */
	iowrite32(0x33333333, gpio_start + GPIO_PC_MODEL);

	/* Enable internal pull-up */
	iowrite32(0xff, gpio_start + GPIO_PC_DOUT);

	/* Setup interrupt handling */

	/* Set interrupt source to pins 0-7 of port C (gamepad buttons) */
	iowrite32(0x22222222, gpio_start + GPIO_EXTIPSELL);

	/* Set interrupt on 1->0 transition */
	iowrite32(0xff, gpio_start + GPIO_EXTIFALL);

	/*
	 * Register even and odd interrupt handlers
	 * IRQ index is 0 for gpio even and 1 for gpio odd
	 * TODO: handle error, is this shared interrupt handler??
	 */
	
	//int irq = platform get irq(dev, index)
	gpio_even_irq = platform_get_irq(dev, 0);
	gpio_odd_irq = platform_get_irq(dev, 1);

	// int request_irq(unsigned int irq, irqreturn_t (*handler)(), unsigned long flags, const char *dev_name, void *dev_id);
	if ((retval = request_irq(gpio_even_irq, (irq_handler_t)gp_interrupt_handler, 0, DEV_NAME, dev)) < 0) {
		printk(KERN_INFO "Error %d: Failed to register even irq.", retval);
	}

	if ((retval = request_irq(gpio_odd_irq, (irq_handler_t)gp_interrupt_handler, 0, DEV_NAME, dev)) < 0) {
		printk(KERN_INFO "Error %d: Failed to register odd irq.", retval);
	}

	/* Enable interrupt generation
	 * TODO: check if correct!
	 */
	iowrite32(0xff, gpio_start + GPIO_IEN);

	/* Clear interrupt flags */
	clear_value = ioread32(gpio_start + GPIO_IF);
	iowrite32(clear_value, gpio_start + GPIO_IFC);

	return 0;

}

/*
 * gamepad_remove: function to cleanup this module from kernel space
 *
 * Returns 0 if successfull, otherwise -1
 */

static int gamepad_remove(struct platform_device *dev)
{
	int gpio_even_irq;
	int gpio_odd_irq;
	
	/* Disable interrupts and free irq */
	iowrite32(0x0, gpio_start + GPIO_IEN);

	//void free_irq(unsigned int irq, void *dev_id);
	gpio_even_irq = platform_get_irq(dev, 0);
	gpio_odd_irq = platform_get_irq(dev, 1);
	
	free_irq(gpio_even_irq, dev);
	free_irq(gpio_odd_irq, dev);

	/* Return I/O ports to the system */
	
	//void release_region(unsigned long start, unsigned long n);
	release_region(res->start, resource_size(res));
	
	/* Unmap virtual memory */
	
	// iounmap(PC)
	iounmap(res);
	
	/* Free device numbers */

	// void unregister_chrdev_region(dev_t first, unsigned int count);
	unregister_chrdev_region(gamepad_num, DEV_NUM_COUNT);

	/* Remove device from the system */

	// void cdev_del(struct cdev *dev);
	cdev_del(gamepad_cdev);

	device_destroy(gamepad_cl, gamepad_num);
	class_destroy(gamepad_cl);

	return 0;
}

/*
 * gamepad_cleanup: function to cleanup this module from kernel space
 *
 * This is the second of two exported functions to handle cleanup this
 * code from a running kernel
 */

static void __exit gamepad_cleanup(void)
{	
	platform_driver_unregister(&gamepad_driver);
}

/*
 * The open method is provided for a driver to do any initialization in preparation for later operations
 */
 
static int gamepad_open(struct inode *inode, struct file *filp)
{	
	return 0;
}

/*
 *The role of the release method is the reverse of open.
 */

static int gamepad_release(struct inode *inode, struct file *flip)
{	
	return 0;
}

/*
 * The task of the read method is to copy data from the device to user space (using copy_to_user)
 */

static ssize_t gamepad_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	unsigned long reading;
		
	/* Read from GPIO_PC_DIN to get status of input pins */
	reading = ioread32(gpio_start + GPIO_PC_DIN);
	
	/*
	 * Copy reading from kernel space to user space
	 * copy_to_user reterns EFAULT if not successful
	 */
	
	//unsigned long copy_to_user(void __user *to, const void *from, unsigned long count);
	if (copy_to_user(buff, &reading, sizeof(reading))) {
		return 0;
	}
	
	// return the amount of memory still to be copied p. 23 chapter 3
	return 1;
	
}

static int gamepad_fasync(int fd, struct file *filp, int mode)
{
	// int fasync_helper(int fd, struct file *filp, int mode, struct fasync_struct **fa);
	return fasync_helper(fd, filp, mode, &gamepad_async_queue);
}

/*
 * Interrupt handler
 */

static irqreturn_t gp_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned long clear_value;
		
	/* Clear interrupt flags */
	clear_value = ioread32(gpio_start + GPIO_IF); //Find source of interrupt by reading the GPIO_IF register
	iowrite32(clear_value, gpio_start + GPIO_IFC);
	
	/* When data arrives, the following statement must be executed to signal asynchronous readers */
	if (gamepad_async_queue) {
		kill_fasync(&gamepad_async_queue, SIGIO, POLL_IN);
	}
	
	return IRQ_HANDLED;
}

/* Let the kernel know which functions are init and exit */
module_init(gamepad_init);
module_exit(gamepad_cleanup);

/* Licence */
MODULE_DESCRIPTION("Gamepad module.");
MODULE_LICENSE("GPL");

