
#define DIOVendor   0x09CA
#define DIOProduct  0x5544
static struct usb_device_id usb_dio_table [] = {
  { USB_DEVICE(DIOVendor, DIOProduct) },
  { } /* Terminating Entry */
};
MODULE_DEVICE_TABLE(usb, usb_dio_table);
static int usb_dio_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void usb_dio_disconnect(struct usb_interface *interface);

static struct usb_driver usb_dio_driver = {
  //.owner = THIS_MODULE,
  .name = "BMCM USB-PIO Driver",
  .id_table = usb_dio_table,
  .probe = usb_dio_probe,
  .disconnect = usb_dio_disconnect
};

typedef struct usb_dio_dev usb_dio_dev;
struct usb_dio_workitem {
  struct work_struct work;
  void *arg;
  struct usb_dio_dev *dev;
};

struct usb_dio_urb_chain {
  struct usb_dio_urb_chain *next;
  struct urb *urb;
};

struct usb_dio_dev {
  struct usb_device              *dev;
  struct usb_interface           *interface;
  int                             running;
  
#define USB_DIO_DEV_BUFFERSIZE 128
  unsigned char                   buffer[USB_DIO_DEV_BUFFERSIZE];
  int                             buffer_head;
  int                             buffer_tail;
  struct semaphore                buffer_sem;
  struct semaphore                buffer_empty_sem;
  
  struct usb_endpoint_descriptor *bulk_in;
  struct urb                     *bulk_in_urb;
  unsigned char                  *bulk_in_buffer;
  size_t                          bulk_in_size;
  __u8                            bulk_in_endpointAddr;
  unsigned int                    bulk_in_endpointPipe;
  struct workqueue_struct        *bulk_in_workqueue;
  struct usb_dio_workitem         bulk_in_work;
  
  struct usb_endpoint_descriptor *bulk_out;
  __u8                            bulk_out_endpointAddr;
  unsigned int                    bulk_out_endpointPipe;
  struct workqueue_struct        *bulk_out_workqueue;
  struct usb_dio_workitem         bulk_out_work;
  struct usb_dio_urb_chain       *bulk_out_cb_urb_chain;
  struct semaphore                bulk_out_cb_urb_chain_sem;
  struct usb_dio_urb_chain       *bulk_out_urb_chain;
  struct semaphore                bulk_out_urb_chain_sem;
  
  struct usb_endpoint_descriptor *bulk_ctrl;
  __u8                            bulk_ctrl_endpointAddr;
  struct kref                     kref;
};

void diodev_rx_setup(struct usb_dio_dev *dev);

static int diodev_release(struct inode *inode, struct file *file);
static int diodev_open(struct inode *inode, struct file *file);
static ssize_t diodev_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos);
static ssize_t diodev_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos);
static struct file_operations usb_dio_fops = {
	.owner =	  THIS_MODULE,
	.open =		  diodev_open,
	.read =		  diodev_read,
	.write =	  diodev_write,
	.release =	diodev_release,
};
static struct usb_class_driver usb_dio_class = {
	.name = "usb/bmcm_dio%d",
	.fops = &usb_dio_fops,
	//.mode = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH,
	.minor_base = 192,
};
