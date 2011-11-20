#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/smp_lock.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>

#include "mod.h"
MODULE_LICENSE("Dual BSD/GPL");

#ifdef __arm__
  #define usb_buffer_free(a,b,c,d)  usb_free_coherent((a),(b),(c),(d))
  #define usb_buffer_alloc(a,b,c,d) usb_alloc_coherent((a),(b),(c),(d))
#endif

#ifdef DEBUG
  #define DPRINTK(...)  printk(__VA_ARGS__)
  #define FUNC_HI()     printk(KERN_ALERT "%s:%d: Hello",__FUNCTION__,__LINE__)
  #define FUNC_BYE()    printk(KERN_ALERT "%s:%d: Goodbye",__FUNCTION__,__LINE__)
  #define FUNC_ERR()    printk(KERN_ALERT "%s:%d: ******* ERROR! *******",__FUNCTION__,__LINE__)
#else
/* THIS IS DANGEROUS!!! IT WORKS FOR NOW, BUT WHO KNOWS WHEN IT MAY BREAK!
   if you replace this with a more sensible line like FUNC_HI() the module
   will probrably CRASH!!!                                                 */
  #define DPRINTK(...)  snprintf(NULL,0,__VA_ARGS__)
  #define FUNC_HI()     
  #define FUNC_BYE()    
  #define FUNC_ERR()    
#endif

/* dont even ask what this is about... */
#define DDPRINTK(...)   { int i,o,p; printk(__VA_ARGS__); i = 0xFFFFFF; for (o = i; o; o--) { for (p = i; p; p--) { p++; p--; } } }

/* the following is stolen from: linux/usb/cdc.h */
#define USB_CDC_UNION_TYPE              0x06
/* "Union Functional Descriptor" from CDC spec 5.2.3.8 */
struct usb_cdc_union_desc {
        __u8    bLength;
        __u8    bDescriptorType;
        __u8    bDescriptorSubType;

        __u8    bMasterInterface0;
        __u8    bSlaveInterface0;
        /* ... and there could be other slave interfaces */
} __attribute__ ((packed));


/* ############################################################## */


static void diodev_delete(struct kref *kref) {
	struct usb_dio_dev *dev = container_of(kref, usb_dio_dev, kref);
  FUNC_HI();
	usb_put_dev(dev->dev);
	if (dev->bulk_in_buffer) kfree(dev->bulk_in_buffer);
	kfree(dev);
  FUNC_BYE();
}


/* ############################################################## */


static int diodev_open(struct inode *inode, struct file *file) {
  struct usb_dio_dev *dev;
  struct usb_interface *interface;
  int subminor;
  int p;
  FUNC_HI();
  
  subminor = iminor(inode);
  
  interface = usb_find_interface(&usb_dio_driver, subminor);
  if (!interface) {
    printk(KERN_ALERT "%s: Can't find device for minor %d\n", usb_dio_driver.name, subminor);
    FUNC_ERR();
    return -ENODEV;
  }
  
  dev = usb_get_intfdata(interface);
  if (!dev) {
    printk(KERN_ALERT "%s: Can't find interface data for minor %d\n", usb_dio_driver.name, subminor);
    FUNC_ERR();
    return -ENODEV;
  }
  
  kref_get(&dev->kref);
  
  file->private_data = dev;
  
  if (down_interruptible(&dev->buffer_sem)) {
    FUNC_ERR();
    return -EINTR;
  }
  dev->buffer_head = 0;
  dev->buffer_tail = 0;
  p = down_trylock(&dev->buffer_empty_sem);
  up(&dev->buffer_sem);
  
  FUNC_BYE();
  return 0;
}


/* ############################################################## */


static void diodev_rx_cb(struct urb *urb) {
  struct usb_dio_dev *dev;
  FUNC_HI();
  DPRINTK(KERN_ALERT "=== diodev_rx_cb() hello\n");
  DPRINTK(KERN_ALERT "=== diodev_rx_cb() urb @ %p\n",urb);
  dev = urb->context;
  if (!dev->running) {
    FUNC_ERR();
    return;
  }
  dev->bulk_in_urb = NULL;
  dev->bulk_in_work.arg = (void *)urb;
  queue_work(dev->bulk_in_workqueue, &(dev->bulk_in_work.work));
  DPRINTK(KERN_ALERT "=== diodev_rx_cb() %d\n",urb->actual_length);
  DPRINTK(KERN_ALERT "=== diodev_rx_cb() returning\n");
  FUNC_BYE();
}
void diodev_rx_work(struct work_struct *work) {
  struct usb_dio_workitem *workitem;
  struct usb_dio_dev *dev;
  FUNC_HI();
  workitem = (struct usb_dio_workitem*)work;
  dev = workitem->dev;
  
  DPRINTK(KERN_ALERT "=== diodev_rx_work() hello\n");
  
  if (!workitem->arg) {
    DPRINTK(KERN_ALERT "=== diodev_rx_work() NULL arg!?\n");
  } else {
    struct urb *urb;
    char *buf;
    int i;
    int p;
    char t[128];
    
    DPRINTK(KERN_ALERT "=== diodev_rx_work() non-null arg!\n");
    urb = workitem->arg;
    
    DPRINTK(KERN_ALERT "=== diodev_rx_work() %d\n",urb->actual_length);
    buf = (char *)urb->transfer_buffer;
    
    for (i = 0; i < 127 && i < urb->actual_length; i++) {
      t[i] = buf[i];
      if (t[i] == '\r') t[i] = '-';
    }
    t[i] = '\0';
    DPRINTK(KERN_ALERT "### Recieved [%s] ###\n", t);

    DPRINTK(KERN_ALERT "%%%%%% %d - %d\n",dev->buffer_head,dev->buffer_tail);    
    /* if we have a valid return value */
    DDPRINTK("R");
    if (urb->actual_length == 6 && buf[0] == '!' && buf[1] == '0' && buf[2] == '0' && buf[5] == '\r') {
      char t[3];
      unsigned char c;
      t[0] = buf[3];
      t[1] = buf[4];
      t[2] = '\0';
      sscanf(t,"%02X",(unsigned int *)&c);
      DPRINTK(KERN_ALERT "### Recieved 0x%02X ###\n", c);
      if (down_interruptible(&dev->buffer_sem)) {
        workitem->arg = NULL;
        diodev_rx_setup(dev);
        FUNC_ERR();
        return;
      }
      DPRINTK(KERN_ALERT "=== diodev_rx_work() success down(buffer_sem)...\n");
      if (!((dev->buffer_tail == USB_DIO_DEV_BUFFERSIZE && dev->buffer_head == 0) ||
          (dev->buffer_tail == dev->buffer_head - 1))) {
        
        dev->buffer[dev->buffer_tail] = c;
        dev->buffer_tail++;
        if (dev->buffer_tail >= USB_DIO_DEV_BUFFERSIZE) dev->buffer_tail = 0;
        DPRINTK(KERN_ALERT "### Added to buffer...\n");
        /* if the lock fails... then it is already locked, so unlock it! */
        /* if the lock succeeds... then it is not already locked, but is now... so unlock it! */
        p = down_trylock(&dev->buffer_empty_sem);
        DPRINTK(KERN_ALERT "### Kicking sem...\n");
        up(&dev->buffer_empty_sem);
        DPRINTK(KERN_ALERT "%%%%%% %d - %d\n",dev->buffer_head,dev->buffer_tail);
      } else {
        DPRINTK(KERN_ALERT "### Buffer full...\n");
      }
      DDPRINTK("-0x%02X", c);
      up(&dev->buffer_sem);
    } else {
      DDPRINTK("-X");
    }
    DDPRINTK("\n");
    DPRINTK(KERN_ALERT "%%%%%% %d - %d\n",dev->buffer_head,dev->buffer_tail);
    
    usb_buffer_free(dev->dev, dev->bulk_in_size, urb->transfer_buffer, urb->transfer_dma);
    usb_free_urb(urb);
    
  }
  workitem->arg = NULL;
  
  diodev_rx_setup(dev);
  DPRINTK(KERN_ALERT "=== diodev_rx_work() returning\n");
  FUNC_BYE();
}
void diodev_rx_setup(struct usb_dio_dev *dev) {
  struct urb *urb;
	char *urbBuf;
  int retval;
  FUNC_HI();
  DPRINTK(KERN_ALERT "=== diodev_rx_setup() hello\n");
  
  urb = usb_alloc_urb(0, GFP_KERNEL);
  if (!urb) {
    printk(KERN_ALERT "=== diodev_rx_setup() -ENOMEM\n");
    FUNC_ERR();
    return;
  }
  
  DPRINTK(KERN_ALERT "=== Building Rx Buffer...\n");
  urbBuf = usb_buffer_alloc(dev->dev, dev->bulk_in_size, GFP_KERNEL, &urb->transfer_dma);
  if (!urbBuf) {
    usb_free_urb(urb);
    printk(KERN_ALERT "=== diodev_rx_setup() -ENOMEM\n");
    FUNC_ERR();
    return;
  }
  
  usb_fill_bulk_urb(urb, dev->dev, dev->bulk_in_endpointPipe, urbBuf, dev->bulk_in_size, (usb_complete_t)diodev_rx_cb, dev);
  urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
  
  retval = usb_submit_urb(urb, GFP_KERNEL);
  if (retval) {
    printk(KERN_ALERT "=== diodev_rx_setup() URB submit error %d\n", retval);
    usb_buffer_free(dev->dev, dev->bulk_in_size, urbBuf, urb->transfer_dma);
    usb_free_urb(urb);
    FUNC_ERR();
    return;
  }
  dev->bulk_in_urb = urb;
  DPRINTK(KERN_ALERT "=== diodev_rx_setup() returning\n");
  FUNC_BYE();
}

/* pulls data from the circular buffer - dev->buffer */
static ssize_t diodev_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos) {
  struct usb_dio_dev *dev;
  unsigned char buf[USB_DIO_DEV_BUFFERSIZE];
  int buflen = 0;
  FUNC_HI();
  dev = (struct usb_dio_dev *)file->private_data;
  DPRINTK(KERN_ALERT "=== diodev_read() hello\n");
  
  if (down_interruptible(&dev->buffer_sem)) {
    FUNC_ERR();
    return -EINTR;
  }
  while (dev->buffer_head == dev->buffer_tail) {  
    DPRINTK(KERN_ALERT "%%%%%% %d - %d\n",dev->buffer_head,dev->buffer_tail);
    up(&dev->buffer_sem);

    DPRINTK(KERN_ALERT "=== diodev_read() buffer empty... waiting on sem\n");
    
    if (down_interruptible(&dev->buffer_empty_sem) == -EINTR) {
      DPRINTK(KERN_ALERT "=== diodev_read() interrupted!\n");
      FUNC_ERR();
      return -EINTR;
    }
    DPRINTK(KERN_ALERT "=== diodev_read() ...\n");
    
    if (down_interruptible(&dev->buffer_sem)) {
      FUNC_ERR();
      return -EINTR;
    }
  }
  DPRINTK(KERN_ALERT "%%%%%% %d - %d\n",dev->buffer_head,dev->buffer_tail);
  DPRINTK(KERN_ALERT "=== diodev_read() continuing!\n");

  if (dev->buffer_head < dev->buffer_tail) {
    buflen = dev->buffer_tail - dev->buffer_head;
    if (count < buflen) buflen = count;
    count -= buflen;
    memcpy(&(buf[0]), &(dev->buffer[dev->buffer_head]), buflen);
    dev->buffer_head += buflen;
    if (dev->buffer_head >= USB_DIO_DEV_BUFFERSIZE) dev->buffer_head = 0;
    
  } else {
    buflen = USB_DIO_DEV_BUFFERSIZE - dev->buffer_head;
    if (count < buflen) buflen = count;
    count -= buflen;
    memcpy(&(buf[0]), &(dev->buffer[dev->buffer_head]), buflen);
    dev->buffer_head += buflen;
    if (dev->buffer_head >= USB_DIO_DEV_BUFFERSIZE) dev->buffer_head = 0;
    
    if (count && dev->buffer_head != 0) {
      FUNC_ERR();
      return -EIO;
    }
    
    if (count) {
      int buflen2;
      buflen2 = dev->buffer_tail;
      if (count < buflen2) buflen2 = count;
      memcpy(&(buf[buflen]), &(dev->buffer[0]), buflen2);
      dev->buffer_head += buflen2;
      if (dev->buffer_head >= USB_DIO_DEV_BUFFERSIZE) dev->buffer_head = 0;
      
      buflen += buflen2;
    }
  }
  
  if (copy_to_user(buffer, buf, buflen) != 0) {
    dev->buffer_head = 0;
    dev->buffer_tail = 0;
    up(&dev->buffer_sem);
    FUNC_ERR();
    return -EIO;
  }
  up(&dev->buffer_sem);
  
  DPRINTK(KERN_ALERT "=== diodev_read() returning %d bytes\n",buflen);
  FUNC_BYE();
  return buflen;
}

static void diodev_write_cb(struct urb *urb) {
  struct usb_dio_dev *dev;
  FUNC_HI();
  dev = (struct usb_dio_dev *)urb->context;
  if (!dev->running) {
    FUNC_ERR();
    return;
  }

  if (down_interruptible(&dev->bulk_out_cb_urb_chain_sem)) {
    FUNC_ERR();
    return;
  }
  
  dev->bulk_out_work.arg = (void *)urb;
  queue_work(dev->bulk_out_workqueue, &(dev->bulk_out_work.work));
  FUNC_BYE();
}
void diodev_write_work(struct work_struct *work) {
  struct usb_dio_workitem *workitem;
  struct usb_dio_dev *dev;
  struct urb *urb;
  struct usb_dio_urb_chain *t_urb_chain;
  FUNC_HI();
  
  workitem = (struct usb_dio_workitem*)work;
  dev = workitem->dev;
  urb = (struct urb*)workitem->arg;
  
	/* sync/async unlink faults aren't errors */
	if (urb->status && 
	    !(urb->status == -ENOENT || 
	      urb->status == -ECONNRESET ||
	      urb->status == -ESHUTDOWN)) {
		DPRINTK(KERN_ALERT "%s - nonzero write bulk status received: %d", __FUNCTION__, urb->status);
	}

  if (down_interruptible(&dev->bulk_out_urb_chain_sem)) {
    FUNC_ERR();
    return;
  }
  t_urb_chain = dev->bulk_out_urb_chain;
  dev->bulk_out_urb_chain = t_urb_chain->next;
  if (t_urb_chain->urb != urb) {
    printk(KERN_ALERT "************ URB != bulk_out_urb_chain... ************\n");
  }
  up(&dev->bulk_out_urb_chain_sem);
  
	/* free up our allocated buffer & urb */
	usb_buffer_free(urb->dev, urb->transfer_buffer_length, urb->transfer_buffer, urb->transfer_dma);
  usb_free_urb(urb);
  
  if (dev->bulk_out_urb_chain) {
    struct urb *urb;
    int retval;
    urb = dev->bulk_out_urb_chain->urb;
    retval = usb_submit_urb(urb, GFP_KERNEL);
    if (retval) {
      DPRINTK(KERN_ALERT "=== URB submit error %d\n", retval);
      usb_buffer_free(dev->dev, urb->transfer_buffer_length, urb->transfer_buffer, urb->transfer_dma);
      usb_free_urb(urb);
      FUNC_ERR();
      return;
    }
  }
  kfree(t_urb_chain);
  up(&dev->bulk_out_cb_urb_chain_sem);
  FUNC_BYE();
}

/* 7 bytes max write!
   first byte
      bit 7 - flush buffer (buffer will only contain result of this command)
      bit 6 - query? (1 = query/read, 0 = set/write) if query is set, this is a single byte command
      bit 5 - PORTA dir present   "@00D0%02X\r"
      bit 4 - PORTB dir present   "@00D1%02X\r"
      bit 3 - PORTC dir present   "@00D2%02X\r"
      bit 2 - PORTA data present  "@00P0%02X\r"
      bit 1 - PORTB data present  "@00P1%02X\r"
      bit 0 - PORTC data present  "@00P2%02X\r"
   
*/
static ssize_t diodev_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos) {
#define CMDSEP "\r"
//#define CMDSEP "-"
  struct usb_dio_dev *dev;
  struct usb_dio_urb_chain *bulk_out_urb_chain;
  struct usb_dio_urb_chain *t_urb_chain;
  
  struct urb *urb;
	char *urbBuf;
  
  unsigned char buf[7];
  int ptr;
  
  unsigned char usbBuf[64]; /* max of 48 bytes possible */
  int usbBytes = 0;
  
  int isQuery = 0;
  int retval;
  FUNC_HI();
  
  dev = (struct usb_dio_dev *)file->private_data;
  
  if (count > 7) {
    printk(KERN_ALERT "=== Too many bytes! (%d)\n", (int)count);
    FUNC_ERR();
    return -EIO;
  }
  if (copy_from_user(&(buf[0]),user_buffer,count)) {
    printk(KERN_ALERT "=== Copy error\n");
    FUNC_ERR();
    return -EIO;
  }
  
  if (buf[0] & 0x80) {
    if (down_interruptible(&dev->buffer_sem)) {
      FUNC_ERR();
      return -EINTR;
    }
    dev->buffer_head = 0;
    dev->buffer_tail = 0;
    up(&dev->buffer_sem);
    DPRINTK(KERN_ALERT "=== Flushed\n");
  }
  if (buf[0] & 0x40) {
    isQuery = 1;
    DPRINTK(KERN_ALERT "=== isQuery\n");
  }
  
  /* if is a query... we only need 1 byte! */
  if (buf[0] & 0x40) {
    DPRINTK(KERN_ALERT "=== Looking for %d bytes...\n",1);
    DPRINTK(KERN_ALERT "=== Given %d bytes...\n", (int)count);
    if (count != 1) {
      FUNC_ERR();
      return -EIO;
    }
  } else {
    int c = 1;
    unsigned char t = 0x20;
    while (t) {
      if (buf[0] & t) c++;
      t >>= 1;
      t &= 0x7F;
    }
    DPRINTK(KERN_ALERT "=== Looking for %d bytes...\n",c);
    DPRINTK(KERN_ALERT "=== Given %d bytes...\n", (int)count);
    if (count != c) {
      FUNC_ERR();
      return -EIO;
    }
  }
  
  if (isQuery) {
    DDPRINTK("Q-");
  } else {
    DDPRINTK("W-");
  }
  ptr = 1;
  if (buf[0] & 0x20) {
    DDPRINTK("d0");
    DPRINTK(KERN_ALERT "=== D0\n");
    if (isQuery) {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00D0?"CMDSEP);
    } else {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00D0%02X"CMDSEP,buf[ptr]);
      ptr++;
    }
  }
  if (buf[0] & 0x10) {
    DDPRINTK("d1");
    DPRINTK(KERN_ALERT "=== D1\n");
    if (isQuery) {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00D1?"CMDSEP);
    } else {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00D1%02X"CMDSEP,buf[ptr]);
      ptr++;
    }
  }
  if (buf[0] & 0x08) {
    DDPRINTK("d2");
    DPRINTK(KERN_ALERT "=== D2\n");
    if (isQuery) {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00D2?"CMDSEP);
    } else {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00D2%02X"CMDSEP,buf[ptr]);
      ptr++;
    }
  }
  if (buf[0] & 0x04) {
    DDPRINTK("p0");
    DPRINTK(KERN_ALERT "=== P0\n");
    if (isQuery) {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00P0?"CMDSEP);
    } else {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00P0%02X"CMDSEP,buf[ptr]);
      ptr++;
    }
  }
  if (buf[0] & 0x02) {
    DDPRINTK("p1");
    DPRINTK(KERN_ALERT "=== P1\n");
    if (isQuery) {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00P1?"CMDSEP);
    } else {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00P1%02X"CMDSEP,buf[ptr]);
      ptr++;
    }
  }
  if (buf[0] & 0x01) {
    DDPRINTK("p2");
    DPRINTK(KERN_ALERT "=== P2\n");
    if (isQuery) {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00P2?"CMDSEP);
    } else {
      usbBytes += snprintf(&(usbBuf[usbBytes]),64-usbBytes,"@00P2%02X"CMDSEP,buf[ptr]);
      ptr++;
    }
  }
  
  DPRINTK(KERN_ALERT "=== STRING:- %s\n",usbBuf);
  
  DPRINTK(KERN_ALERT "=== Building URB...\n");
  
  urb = usb_alloc_urb(0, GFP_KERNEL);
  DPRINTK(KERN_ALERT "** urb @ %p\n",urb);
  if (!urb) {
    FUNC_ERR();
    return -ENOMEM;
  }
  
  DPRINTK(KERN_ALERT "=== Building Buffer (%d)...\n",usbBytes);
  urbBuf = usb_buffer_alloc(dev->dev, usbBytes, GFP_KERNEL, &urb->transfer_dma);
  DPRINTK(KERN_ALERT "** urbBuf @ %p\n",urbBuf);
  if (!urbBuf) {
    usb_free_urb(urb);
    FUNC_ERR();
    return -ENOMEM;
  }
  
  DPRINTK(KERN_ALERT "=== Populating Buffer...\n");
  memcpy(urbBuf, usbBuf, usbBytes);
  
  usb_fill_bulk_urb(urb, dev->dev, dev->bulk_out_endpointPipe, urbBuf, usbBytes, (usb_complete_t)diodev_write_cb, dev);
  urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
  
  t_urb_chain = kmalloc(sizeof(struct usb_dio_urb_chain), GFP_KERNEL);    
  if (dev == NULL) {
    err("Out of Memory");
    FUNC_ERR();
    return -ENOMEM;
  }
  t_urb_chain->next = NULL;
  t_urb_chain->urb = urb;
  if (down_interruptible(&dev->bulk_out_urb_chain_sem)) {
    FUNC_ERR();
    return -EINTR;
  }
  if (dev->bulk_out_urb_chain) {
    DDPRINTK("-C");
    DPRINTK(KERN_ALERT "=== Chaining URB...\n");
    bulk_out_urb_chain = dev->bulk_out_urb_chain;
    while (bulk_out_urb_chain->next) {
      bulk_out_urb_chain = bulk_out_urb_chain->next;
    }
    bulk_out_urb_chain->next = t_urb_chain;
  } else {
    DDPRINTK("-S");
    DPRINTK(KERN_ALERT "=== Submitting URB...\n");
    retval = usb_submit_urb(urb, GFP_KERNEL);
    if (retval) {
      up(&dev->bulk_out_urb_chain_sem);
      DPRINTK(KERN_ALERT "=== URB submit error %d\n", retval);
      usb_buffer_free(dev->dev, usbBytes, urbBuf, urb->transfer_dma);
      usb_free_urb(urb);
      FUNC_ERR();
      return retval;
    }
    dev->bulk_out_urb_chain = t_urb_chain;
  }
  DDPRINTK("\n");
  up(&dev->bulk_out_urb_chain_sem);
  
  DPRINTK(KERN_ALERT "=== diodev_write() Returning!\n");
  FUNC_BYE();
  return count;
}

static int diodev_release(struct inode *inode, struct file *file) {
	struct usb_dio_dev *dev;
  FUNC_HI();

	dev = (struct usb_dio_dev *)file->private_data;
	if (dev == NULL) {
    FUNC_ERR();
		return -ENODEV;
  }

	/* decrement the count on our device */
	kref_put(&dev->kref, diodev_delete);
  FUNC_BYE();
	return 0;
}

/* ############################################################## */


static int usb_dio_init(void) {
  int result;
  FUNC_HI();
  /* register this driver with the USB subsystem */
  result = usb_register(&usb_dio_driver);
  if (result) {
    DPRINTK(KERN_ALERT "%s: usb_register() failed... Error number %d", usb_dio_driver.name, result);
    DPRINTK(KERN_ALERT "%s: NOT Loaded!\n", usb_dio_driver.name);
  } else {
    DPRINTK(KERN_ALERT "%s: Loaded!\n", usb_dio_driver.name);
  }
  
  FUNC_BYE();
  return result;
}

static void usb_dio_exit(void) {
  FUNC_HI();
  /* deregister the driver with the USB subsystem */
  usb_deregister(&usb_dio_driver);
  
  DPRINTK(KERN_ALERT "%s: Unloaded!\n", usb_dio_driver.name);
  FUNC_BYE();
}

module_init(usb_dio_init);
module_exit(usb_dio_exit);


/* ############################################################## */


static int usb_dio_probe(struct usb_interface *intf, const struct usb_device_id *id) {
  struct usb_cdc_union_desc *union_header = NULL;
	unsigned char *buffer = intf->altsetting->extra;
	int buflen = intf->altsetting->extralen;
	struct usb_interface *control_interface;
	struct usb_interface *data_interface;
	struct usb_device *usb_dev = interface_to_usbdev(intf);
  int retval = 0;
  
  usb_dio_dev *dev;
  
  /* prevent usb_dio_probe() from racing usb_dio_disconnect() */
  lock_kernel();
  
  FUNC_HI();
  dev = kmalloc(sizeof(usb_dio_dev), GFP_KERNEL);
  if (dev == NULL) {
    err("Out of Memory");
    unlock_kernel();
    FUNC_ERR();
    return -ENOMEM;
  }
	memset(dev, 0x00, sizeof (usb_dio_dev));
  dev->dev = usb_get_dev(interface_to_usbdev(intf));
	kref_init(&(dev->kref));
  usb_set_intfdata(intf, dev);
  
  printk(KERN_ALERT "%s: === Starting device probe ===\n", usb_dio_driver.name);
  
  if (!buflen) {
    printk(KERN_ALERT "%s: === Invalid / unwanted device ===\n", usb_dio_driver.name);
    unlock_kernel();
    FUNC_ERR();
    return -EINVAL;
  }

  DPRINTK(KERN_ALERT "%s: == Chunk size    = %2d ==\n", usb_dio_driver.name, buffer[0]);
  DPRINTK(KERN_ALERT "%s: == Buffer length = %2d ==\n", usb_dio_driver.name, buflen);
	while (buflen > 0) {
		switch (buffer[2]) {
      case USB_CDC_UNION_TYPE: /* we've found it */
        DPRINTK(KERN_ALERT "%s: ==== USB_CDC_UNION_TYPE ==============\n", usb_dio_driver.name);
        if (union_header) {
          DPRINTK(KERN_ALERT "%s: ===== More than one union header! =====\n", usb_dio_driver.name);
          break;
        }
        union_header = (struct usb_cdc_union_desc *)buffer;
        break;
      default:
        DPRINTK(KERN_ALERT "%s: ==== Unwanted default... =============\n", usb_dio_driver.name);
        break;
		}
    DPRINTK(KERN_ALERT "%s: === continuation with %2d remaining... ===\n", usb_dio_driver.name, buflen - buffer[0]);
		buflen -= buffer[0];
		buffer += buffer[0];
	}
  DPRINTK(KERN_ALERT "%s: == complete with %2d remaining ==\n", usb_dio_driver.name, buflen);

  control_interface = usb_ifnum_to_if(usb_dev, union_header->bMasterInterface0);
  data_interface = usb_ifnum_to_if(usb_dev, union_header->bSlaveInterface0);
  if (!control_interface || !data_interface) {
    printk(KERN_ALERT "%s: === missing interface(s)! ===\n", usb_dio_driver.name);
    unlock_kernel();
    FUNC_ERR();
    return -ENODEV;
  }

  sema_init(&dev->buffer_sem,1);
  sema_init(&dev->buffer_empty_sem,1);

  dev->bulk_in = &(data_interface->cur_altsetting->endpoint[0].desc);
  dev->bulk_in_urb = NULL;
  dev->bulk_in_size = dev->bulk_in->wMaxPacketSize;
  dev->bulk_in_endpointAddr = dev->bulk_in->bEndpointAddress & 0xF;
  dev->bulk_in_endpointPipe = usb_rcvbulkpipe(dev->dev,dev->bulk_in_endpointAddr);
  dev->bulk_in_buffer = kmalloc(dev->bulk_in->wMaxPacketSize, GFP_KERNEL);
  dev->bulk_in_workqueue = create_singlethread_workqueue("Rx");
  INIT_WORK(&(dev->bulk_in_work.work),(void(*)(struct work_struct *))diodev_rx_work);
  dev->bulk_in_work.arg = NULL;
  dev->bulk_in_work.dev = dev;
  
  dev->bulk_out = &(data_interface->cur_altsetting->endpoint[1].desc);
  dev->bulk_out_endpointAddr = dev->bulk_out->bEndpointAddress & 0xF;
  dev->bulk_out_endpointPipe = usb_sndbulkpipe(dev->dev,dev->bulk_out_endpointAddr);
  dev->bulk_out_cb_urb_chain = NULL;
  sema_init(&dev->bulk_out_cb_urb_chain_sem,1);
  dev->bulk_out_urb_chain = NULL;
  sema_init(&dev->bulk_out_urb_chain_sem,1);
  dev->bulk_out_workqueue = create_singlethread_workqueue("Tx");
  INIT_WORK(&(dev->bulk_out_work.work),(void(*)(struct work_struct *))diodev_write_work);
  dev->bulk_out_work.arg = NULL;
  dev->bulk_out_work.dev = dev;
  
  dev->bulk_ctrl = &(control_interface->cur_altsetting->endpoint[0].desc);
  dev->bulk_ctrl_endpointAddr = dev->bulk_ctrl->bEndpointAddress & 0xF;
  
  retval = usb_register_dev(intf, &usb_dio_class);
  if (retval) {
    printk(KERN_ALERT "%s: Not able to get a minor for this device...\n", usb_dio_driver.name);
    usb_set_intfdata(intf, NULL);
  } else {
    printk(KERN_ALERT "%s: Minor device %d\n", usb_dio_driver.name, intf->minor);
  }
  
  dev->running = 1;
  diodev_rx_setup(dev);
  
  unlock_kernel();
  
  FUNC_BYE();
  return retval;
}

static void usb_dio_disconnect(struct usb_interface *interface) {
  usb_dio_dev *dev;
  int minor = interface->minor;
  FUNC_HI();
  
  /* give back our minor */
  usb_deregister_dev(interface, &usb_dio_class);
  
  dev = usb_get_intfdata(interface);
  if (!dev) {
    FUNC_ERR();
    return;
  }
  usb_set_intfdata(interface, NULL);
  
  dev->running = 0;
  if (dev->bulk_in_urb) {
    DPRINTK(KERN_ALERT "*** killing urb @ %p\n",dev->bulk_in_urb);
    usb_kill_urb(dev->bulk_in_urb);
  }
  
  /* decrement our usage count - this does kfree(dev) */
  kref_put(&(dev->kref), diodev_delete);
  
  printk(KERN_ALERT "%s: Disconnected #%d\n", usb_dio_driver.name, minor);
  FUNC_BYE();
}
