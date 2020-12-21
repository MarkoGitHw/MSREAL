#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#define MAX_BUFF_SIZE 300
#define MAX_STRING_SIZE 101
MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

DECLARE_WAIT_QUEUE_HEAD(readQ);
DECLARE_WAIT_QUEUE_HEAD(writeQ);
struct semaphore sem;


int stred_string[MAX_STRING_SIZE];
int pos = 0;
int endRead = 0;

int stred_open(struct inode *pinode, struct file *pfile);
int stred_close(struct inode *pinode, struct file *pfile);
ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = stred_open,
	.read = stred_read,
	.write = stred_write,
	.release = stred_close,
};


int stred_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened stred\n");
		return 0;
}

int stred_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed stred\n");
		return 0;
}

ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[MAX_BUFF_SIZE];
	long int len = 0;
	int duzina_ss=strlen(string_stred);
	if (endRead){

	  printk(KERN_INFO "Uspesno Procitan String\n");
	        pos=0;
	        endRead = 0;
		return 0;
	}
	if (pos==0){
	if(down_interruptible(&sem))
		return -ERESTARTSYS;
	while(duzina_ss==0)
	{
		up(&sem);
		if(wait_event_interruptible(readQ,(pos>0)))
			return -ERESTARTSYS;
		if(down_interruptible(&sem))
			return -ERESTARTSYS;
	}

	}
	
		len = scnprintf(buff,MAX_BUFF_SIZE, "%c ", stred[pos++]);
		ret = copy_to_user(buffer, buff, len);
		if(ret)
			return -EFAULT;
		if(pos==duzina_ss){
		  up(&sem);
		  endRead=1;
		  wake_up_interuptible(&writeQ);
		}
	

	return len;
}

ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	int value;
	int ret;
	char niz_in[BUFF_SIZE+9];

	ret = copy_from_user(buff, buffer, length);
	

	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';

	if(down_interruptible(&sem))
		return -ERESTARTSYS;
	while(pos == 10)
	{
		up(&sem);
		if(wait_event_interruptible(writeQ,(pos<10)))
			return -ERESTARTSYS;
		if(down_interruptible(&sem))
			return -ERESTARTSYS;
	}

	if(pos<10)
	{
		ret = sscanf(buff,"%d",&value);
		if(ret==1)//one parameter parsed in sscanf
		{
			printk(KERN_INFO "Succesfully wrote value %d", value); 
			stred[pos] = value; 
			pos=pos+1;
		}
		else
		{
			printk(KERN_WARNING "Wrong command format\n");
		}
	}
	else
	{
		printk(KERN_WARNING "Lifo is full\n"); 
	}
	switch 
	up(&sem);
	wake_up_interruptible(&readQ);
	strcpy(niz_in, buff);

	if(!strncmp(niz_in, "=string", 7)){
		strcpy(string_stred, niz_in+7);
			}
	else if(!strncmp(niz_in, "clear", 5)){
		for(i=0; i<MAX_STRING_SIZE; i++){
			string_stred+i=0;
		}
	}
	else if(!strncmp(niz_in,"shrink", 6)){
		if(string_stred==' '){
			for(i=1; i<strlen(string_stred); i++){
				if(string_stred[i]==' '){
						k++;
				}
				else break;
			}	
		}
		strcpy(string_stred, string_stred+k);
		for(i=strlen(string_stred); i!=0 ;i--){
			if(string_stred[i]==' '){
				q++;
			}
			else break;
		}	
		for(i=strlen(string_stred); i==q; i--){
			string_stred[i]=0;
		}
	}
	else if(!strncmp(niz_in, "append", 6)){
		strcat(string_stred, niz_in+6;)
	}
	else if(!strncmp(niz_in, "truncate=", 9)){
				
		}
	}
	else if(!strncmp(niz_in, "remove=", 7)){
		
	}
	}	
	return length;
}

static int __init stred_init(void)
{
   int ret = 0;
	int i=0;
	
	sema_init(&sem,1);

	//Initialize array
	for (i=0; i<MAX_STRING_SIZE; i++)
		stred_string[i] = 0;

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "stred");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "stred_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "stred");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "Hello world\n");

   return 0;

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit stred_exit(void)
{
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(stred_init);
module_exit(stred_exit);
