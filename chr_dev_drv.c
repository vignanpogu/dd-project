
/*	Linux device driver implementation and communication with IOCTL/Procfs/Sys filesystem.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>    // For Character Driver Registration
#include <linux/slab.h>   // For allocating the kernel memory
#include <linux/uaccess.h>  // For copy_to_user / copy_from_user
#include <linux/ioctl.h>   // For writing ioctl commands
#include <linux/proc_fs.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

// Macro for IOCTL
#define WR_IOCTL _IOW('a','a', int32_t*)
#define RD_IOCTL _IOR('a','b', int32_t*)

#define mem_size 1024       // Memory size

volatile int chr_value = 0; 
int32_t value = 0;
char chr_array[30] = "Linux Device Driver Programming using Procfs";
dev_t dev = 0;

static struct cdev chr_cdev;
static struct class *dev_class;
uint8_t *kernel_buffer;
struct kobject *kobj;


/* Functions Declaration */
static int __init chr_drv_init(void);
static void __exit chr_drv_exit(void);
static int chr_open(struct inode *inode, struct file *file);
static int chr_release (struct inode *inode, struct file *file);
static ssize_t chr_read(struct file *file, char *buf,size_t len,loff_t *off); 
static ssize_t chr_write(struct file *file,const char *buf,size_t len,loff_t *off); 
static long chr_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


/* Proc filesystem Function Declaration */

static int open_proc(struct inode *inode, struct file *file);
static int release_proc (struct inode *inode, struct file *file);
static ssize_t read_proc(struct file *file, char *buf,size_t len,loff_t *off); 
static ssize_t write_proc(struct file *file,const char *buf,size_t len,loff_t *off); 

/* Sysfs filesystem function Declaration */
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static  ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t size);


// Sysfs Attributes
struct kobj_attribute chr_attr = __ATTR(chr_value, 0660, sysfs_show, sysfs_store);

/* File Operation Structure */

static struct file_operations fops = {

	.owner		= THIS_MODULE,
	.read		= chr_read,
	.write		= chr_write,
	.open 		= chr_open,
	.unlocked_ioctl = chr_ioctl,
	.release	= chr_release,
};


/* Procfs file operation structure */

static struct file_operations proc_fops = {

	.open		=	open_proc,
	.read		=	read_proc,
	.write		=	write_proc,
	.release	=	release_proc,
};


// Function Sysfs FileSystem

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk(KERN_INFO" Reading From Sysfs filesystem...\n");
	return sprintf(buf, "%d", chr_value);
}


static  ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t size)
{
	printk(KERN_INFO" Writing into Sysfs filesystem...\n");
	sscanf(buf, "%d", &chr_value);
	return size;
}

// Function for Procfs


static int open_proc(struct inode *inode, struct file *file)
{

	printk(KERN_INFO"Procfs is opened...\n");
	return 0;
}


static int release_proc (struct inode *inode, struct file *file)
{
	printk(KERN_INFO"Procfs is released...\n");
	return 0;
}


static ssize_t read_proc(struct file *file, char *buf,size_t len,loff_t *off)
{
	if (len)
		len = 0;
	else {
		len = 1;	
		return 0;
	}	
	printk(KERN_INFO" Reading From Procfs Functions..\n");
	copy_to_user(buf,chr_array, 30);
	return len;
}

static ssize_t write_proc(struct file *file,const char *buf,size_t len,loff_t *off)
{
	printk(KERN_INFO"Writing into Procfs Functions..\n");
	copy_from_user(chr_array, buf, len);
	return len;
}

// Function to open the Device file

static int chr_open(struct inode *inode, struct file *file)
{
	if((kernel_buffer  = kmalloc(mem_size, GFP_KERNEL)) == 0) {
		printk(KERN_INFO" Unable to allocate the kernel memory\n");
		return -1;
	}
	printk(KERN_INFO " Device File Opened...\n");
	return 0;
}

// Function to release the device file 

static int chr_release (struct inode *inode, struct file *file)
{
	kfree(kernel_buffer);
	printk(KERN_INFO " Device File closed..\n");
	return 0;
}

// Function to read the data from the device file


static ssize_t chr_read(struct file *file, char *buf,size_t len,loff_t *off)
{
	copy_to_user(buf, kernel_buffer, mem_size);
	printk(KERN_INFO "Data Read Successfully ..\n");
	return mem_size;
}

// Function to write the data to the device file

static ssize_t chr_write(struct file *file,const char *buf,size_t len,loff_t *off)
{
	copy_from_user(kernel_buffer, buf, len);
	printk("Data is written successfully..\n");
	return len;
}

// Function to execute IOCTL calls

static long chr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd) {

		case WR_IOCTL:
			copy_from_user(&value, (int32_t*)arg, sizeof(value));
			printk(KERN_INFO"value =%d\n", value);
			break;
		case RD_IOCTL:
			copy_to_user((int32_t*) arg, &value, sizeof (value));
			break;
	}
	return 0;
}


/* Initializing the charcter device driver module */

static int __init chr_drv_init(void)
{

	//Allocating the Major Number (Dynamically)

	if((alloc_chrdev_region(&dev, 0, 1, "chr_Dev")) < 0) {
		printk(KERN_INFO "Unable to allocate the major number..\n");
		return -1;
	}

	printk(KERN_INFO "Major =  %d Minor =%d\n",MAJOR(dev), MINOR(dev));

	// Creating the cdev Structure
	cdev_init(&chr_cdev, &fops);

	// Adding the character device 
	cdev_add(&chr_cdev, dev, 1);

	// Creating the struct class 
	if((dev_class = class_create(THIS_MODULE,"chr_class")) == NULL) {
		printk(KERN_INFO" Unable to create the struct class\n");
		goto remove_class;
	}
	//Creating device within the class
	if((device_create(dev_class,NULL,dev, NULL,"chr_device")) == NULL) {
		printk(KERN_INFO" unable to create the device\n");
		goto remove_device;
	}

	// create the Proc Entry
	proc_create("chr_proc", 0666, NULL, &proc_fops);

	//Create the Directory in /sys/kernel
		kobj = kobject_create_and_add("sysfs_drv",kernel_kobj);

	//create the Sysfs file 
		if(sysfs_create_file(kobj, &chr_attr.attr)) {
			printk(KERN_INFO" Cannot create the sysfs file..\n");
			goto remove_sysfs;
		}

	printk(KERN_INFO " Character Device Driver Insert Successfully...\n");
	return 0;

remove_sysfs:
	kobject_put(kobj);
	sysfs_remove_file(kernel_kobj,&chr_attr.attr);
remove_class:
	unregister_chrdev_region(dev, 1);
remove_device:
	class_destroy(dev_class);
	return -1;
}

static void __exit chr_drv_exit(void)
{
	kobject_put(kobj);
	sysfs_remove_file(kernel_kobj,&chr_attr.attr);	
	remove_proc_entry("chr_proc", NULL);
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&chr_cdev);
	unregister_chrdev_region(dev, 1);

	printk(KERN_INFO" Character Device Driver Removed suucessfully.\n");
}

module_init(chr_drv_init);
module_exit(chr_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vignan");
MODULE_DESCRIPTION("Character Device Driver Programming");

