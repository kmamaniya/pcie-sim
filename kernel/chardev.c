/*
 * PCIe Simulator - Character Device Interface
 *
 * Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This module handles the character device interface (/dev/pcie_sim0).
 * It provides file operations and IOCTL commands for userspace applications.
 */

#include "common.h"

/*
 * Character device open operation
 */
static int pcie_sim_open(struct inode *inode, struct file *filp)
{
    struct pcie_sim_device *dev;

    dev = container_of(inode->i_cdev, struct pcie_sim_device, cdev);
    filp->private_data = dev;

    if (!dev->enabled)
        return -ENODEV;

    pr_debug("Device %d opened\n", dev->device_id);
    return 0;
}

/*
 * Character device release operation
 */
static int pcie_sim_release(struct inode *inode, struct file *filp)
{
    struct pcie_sim_device *dev = filp->private_data;

    if (dev)
        pr_debug("Device %d closed\n", dev->device_id);

    return 0;
}

/*
 * Character device IOCTL operation
 */
long pcie_sim_char_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct pcie_sim_device *dev = filp->private_data;
    int ret = 0;

    if (!dev)
        return -EINVAL;

    /* Verify IOCTL magic number */
    if (_IOC_TYPE(cmd) != PCIE_SIM_IOC_MAGIC) {
        pr_err("Invalid IOCTL magic: 0x%x\n", _IOC_TYPE(cmd));
        return -EINVAL;
    }

    /* Serialize IOCTL operations */
    if (mutex_lock_interruptible(&dev->mutex))
        return -ERESTARTSYS;

    switch (cmd) {
    case PCIE_SIM_IOC_TRANSFER:
    {
        struct pcie_sim_transfer_req req;

        if (copy_from_user(&req, (void __user *)arg, sizeof(req))) {
            ret = -EFAULT;
            break;
        }

        ret = pcie_sim_dma_transfer(dev, &req);
        if (ret == 0) {
            if (copy_to_user((void __user *)arg, &req, sizeof(req)))
                ret = -EFAULT;
        }
        break;
    }

    case PCIE_SIM_IOC_GET_STATS:
        if (copy_to_user((void __user *)arg, &dev->stats, sizeof(dev->stats)))
            ret = -EFAULT;
        break;

    case PCIE_SIM_IOC_RESET_STATS:
        memset(&dev->stats, 0, sizeof(dev->stats));
        pr_debug("Device %d statistics reset\n", dev->device_id);
        break;

    default:
        pr_err("Unknown IOCTL command: 0x%x\n", cmd);
        ret = -ENOTTY;
        break;
    }

    mutex_unlock(&dev->mutex);
    return ret;
}

/* File operations structure */
static const struct file_operations pcie_sim_fops = {
    .owner = THIS_MODULE,
    .open = pcie_sim_open,
    .release = pcie_sim_release,
    .unlocked_ioctl = pcie_sim_char_ioctl,
    .compat_ioctl = pcie_sim_char_ioctl,
};

/*
 * Initialize character device interface for a device
 */
int pcie_sim_char_init(struct pcie_sim_device *dev)
{
    char dev_name[32];
    int ret;

    pr_debug("Initializing character device for device %d\n", dev->device_id);

    /* Setup device number */
    dev->devt = MKDEV(driver_state.major, dev->device_id);

    /* Initialize and add character device */
    cdev_init(&dev->cdev, &pcie_sim_fops);
    dev->cdev.owner = THIS_MODULE;

    ret = cdev_add(&dev->cdev, dev->devt, 1);
    if (ret) {
        pr_err("Failed to add character device: %d\n", ret);
        return ret;
    }

    /* Create device file in /dev */
    snprintf(dev_name, sizeof(dev_name), "pcie_sim%d", dev->device_id);
    dev->dev = device_create(driver_state.class, &dev->pdev->dev,
                           dev->devt, dev, dev_name);
    if (IS_ERR(dev->dev)) {
        ret = PTR_ERR(dev->dev);
        pr_err("Failed to create device file: %d\n", ret);
        cdev_del(&dev->cdev);
        return ret;
    }

    pr_info("Character device /dev/%s created\n", dev_name);
    return 0;
}

/*
 * Cleanup character device interface for a device
 */
void pcie_sim_char_cleanup(struct pcie_sim_device *dev)
{
    if (!dev)
        return;

    pr_debug("Cleaning up character device for device %d\n", dev->device_id);

    /* Remove device file */
    if (dev->dev) {
        device_destroy(driver_state.class, dev->devt);
        dev->dev = NULL;
    }

    /* Remove character device */
    cdev_del(&dev->cdev);

    pr_debug("Character device cleanup complete for device %d\n", dev->device_id);
}