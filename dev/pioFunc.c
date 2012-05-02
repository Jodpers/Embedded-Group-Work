/* Sources: Essential Linux Device Drivers 
            http://www.cems.uwe.ac.uk/~cduffy/dwd/usbpio.pdf
	    http://www.cems.uwe.ac.uk/~cduffy/lkmpg.pdf
*/

/* Kernel return error codes: ENOMEM = Used when no mem available
                              EFAULT = Used when accessing memory outside you address space
*/

/* Pipes -> interger encoding of: endpoint address, direction (in /out), type of data transfer(control, bulk, irq, bulk, isochronous) */

static void usbPioDisconnect(struct usb_interface *interface)
  {
    printk("Starting usbPioDisconnect\n");
    struct usbPiodeviceT *usbPiodevice;
    
    /*Retrieve data saved with usb_set_intfdata - Free memory*/
    usbPiodevice = usb_get_intfdata(interface); //TODO Free memory
    
    /*Zero out interface data*/
    usb_set_intfdata(interface, NULL);
    
    /* release /dev/usbPio */
    usb_deregister_dev(interface, &usbPioClass);
    
    usbPiodevice->interface = NULL;
  }

static int usbPioOpen(struct inode *inode, struct file *file)
  {
    printk("Starting UsbPioOpenzn");

    usbPioDeviceT *usbPioDevice;
    struct usb_interface *interface;

    /* Get  the interface the device is associated with */
    interface = usb_find_interface(&usbPioDriver, iminor(inode));
    if (!interface)
      {
	printk (KERN_ALERT "Error: Can't find device for minor %d",__FUNCTION__, iminor(inode));
	return -ENODEV;
      }
    /* This data was saved in usbPioProbe*/
    usbPioDevice = usb_get_intfdata(interface);
    
    /* Saves the device specific object for use with read and write*/
    file->private_data = usbPioDevice;
  }

static ssize_t usbPioRead(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
  {
    printk("starting usbPioRead\n");
    
    int retval;
    usbPioDeviceT *usbPioDevice;
    struct urb *urb;
    char *dma_buffer;
    
    /* Get the address of the device, data saved in open */
    usbPioDevice = (usbPioDeviceT *)file->private_data;

    /* Allocate some mem for the URB*/
    usb = urb_alloc_urb(0, GFP_KERNEL);
    if (!usb)
      {
	printk(KERN_ALERT "Memory allocation for the URB failed\n");
	return -ENOMEM;
      }
    
    dma_buffer = usb_alloc_coherent(usbPioDevice->usbDev, usbPioDevice->inBulkBufferSize, GFP_KERNEL, &urb->transfer_dma);
    if (!dma_buffer)
      {
	printk(KERN_ALERT "Error: failed to allocate mem for usb dma buffer\n");
	return -ENOMEM;
      }

    /*Init the urb*/
    usb_fill_bulk_urb(urb,
		      usbPioDevice->usbDev,
		      //usbPioDevice->inBulkEndpointPipe,
		      dma_buffer,
		      count,
		      usbPioReadCallBack,
		      usbPioDevice);

    retval = usb_submit_urb(tele_device->ctrl_urb, GFP_ATOMIC)
    if (retval)
      {
	printk(KERN_ALERT "Error: failed to submit the URB, error number: %d", retval);
	return-EFAULT;
      }
    /*TO DO*/
    return 
  }

static ssize_t usbPioWrite(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos)
  {
    printk("Starting usbPioWrite");
    
    size_t buffer_size = count;
    char *dma_buffer; /*for usb_buffer_alloc*/
    struct urb *urb;
    struct usbPioDeviceT *usbPioDevice;
    int retval;

    /* Get the address of the device, data saved in open */
    usbPioDevice = (usbPioDeviceT *)file->private_data;

    /* Allocate some mem for the URB*/
    usb = urb_alloc_urb(0, GFP_KERNEL);
    if (!usb)
      {
	printk(KERN_ALERT "Memory allocation for the URB failed\n");
	return -ENOMEM;
      }
    
    dma_buffer = usb_alloc_coherent(usbPioDevice->usbDev, usbPioDevice->inBulkBufferSize, GFP_KERNEL, &urb->transfer_dma);
    if (!dma_buffer)
      {
	printk(KERN_ALERT "Error: failed to allocate mem for usb dma buffer\n");
	return -ENOMEM;
      }
    
    /* Copies the data from userland to kernel land, well really the DMA buffer*/
    if(copy_from_user())
      {
	printk(KERN_ALERT"Error: Failed to copy data from user land to the DMA buffer, error number: %d", retval);
      }

    /* TODO */

  }

