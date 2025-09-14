/*
 * PCIe Simulator - Main Driver Module
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
 * This module handles driver initialization, device management, and cleanup.
 */

#include "common.h"

/* Global driver state definition */
struct {
    struct class *class;
    dev_t devt_base;
    int major;
    struct pcie_sim_device *devices[DEVICE_COUNT];
} driver_state;

/* Platform devices array */
static struct platform_device *pcie_sim_devices[DEVICE_COUNT];

/*
 * Platform device probe - called when device is found
 */
static int pcie_sim_probe(struct platform_device *pdev)
{
    struct pcie_sim_device *dev;
    int device_id = pdev->id;
    int ret;

    if (device_id >= DEVICE_COUNT)
        return -EINVAL;

    pr_info("Probing device %d\n", device_id);

    /* Allocate device structure */
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    /* Initialize device */
    dev->pdev = pdev;
    dev->device_id = device_id;
    dev->enabled = true;
    mutex_init(&dev->mutex);
    memset(&dev->stats, 0, sizeof(dev->stats));

    platform_set_drvdata(pdev, dev);
    driver_state.devices[device_id] = dev;

    /* Initialize character device interface */
    ret = pcie_sim_char_init(dev);
    if (ret) {
        pr_err("Failed to initialize character device: %d\n", ret);
        goto err_char;
    }

    /* Initialize proc interface */
    ret = pcie_sim_proc_init(dev);
    if (ret) {
        pr_err("Failed to initialize proc interface: %d\n", ret);
        goto err_proc;
    }

    pr_info("Device %d initialized successfully\n", device_id);
    return 0;

err_proc:
    pcie_sim_char_cleanup(dev);
err_char:
    driver_state.devices[device_id] = NULL;
    kfree(dev);
    return ret;
}

/*
 * Platform device remove - called when device is removed
 */
static int pcie_sim_remove(struct platform_device *pdev)
{
    struct pcie_sim_device *dev = platform_get_drvdata(pdev);
    int device_id = pdev->id;

    pr_info("Removing device %d\n", device_id);

    if (dev) {
        pcie_sim_proc_cleanup(dev);
        pcie_sim_char_cleanup(dev);
        driver_state.devices[device_id] = NULL;
        kfree(dev);
    }

    return 0;
}

/* Platform driver structure */
static struct platform_driver pcie_sim_platform_driver = {
    .probe = pcie_sim_probe,
    .remove = pcie_sim_remove,
    .driver = {
        .name = DRIVER_NAME,
    },
};

/*
 * Create platform devices
 */
static int create_platform_devices(void)
{
    int i, ret;

    for (i = 0; i < DEVICE_COUNT; i++) {
        pcie_sim_devices[i] = platform_device_alloc(DRIVER_NAME, i);
        if (!pcie_sim_devices[i]) {
            ret = -ENOMEM;
            goto cleanup;
        }

        ret = platform_device_add(pcie_sim_devices[i]);
        if (ret) {
            platform_device_put(pcie_sim_devices[i]);
            pcie_sim_devices[i] = NULL;
            goto cleanup;
        }
    }

    return 0;

cleanup:
    while (--i >= 0) {
        if (pcie_sim_devices[i]) {
            platform_device_unregister(pcie_sim_devices[i]);
            pcie_sim_devices[i] = NULL;
        }
    }
    return ret;
}

/*
 * Destroy platform devices
 */
static void destroy_platform_devices(void)
{
    int i;

    for (i = 0; i < DEVICE_COUNT; i++) {
        if (pcie_sim_devices[i]) {
            platform_device_unregister(pcie_sim_devices[i]);
            pcie_sim_devices[i] = NULL;
        }
    }
}

/*
 * Module initialization
 */
static int __init pcie_sim_init(void)
{
    int ret;

    pr_info("PCIe Simulator Driver v%s loading\n", DRIVER_VERSION);

    /* Allocate character device numbers */
    ret = alloc_chrdev_region(&driver_state.devt_base, 0, DEVICE_COUNT, DRIVER_NAME);
    if (ret) {
        pr_err("Failed to allocate character device region: %d\n", ret);
        return ret;
    }
    driver_state.major = MAJOR(driver_state.devt_base);

    /* Create device class */
    driver_state.class = class_create(THIS_MODULE, DRIVER_NAME);
    if (IS_ERR(driver_state.class)) {
        ret = PTR_ERR(driver_state.class);
        pr_err("Failed to create device class: %d\n", ret);
        goto err_class;
    }

    /* Register platform driver */
    ret = platform_driver_register(&pcie_sim_platform_driver);
    if (ret) {
        pr_err("Failed to register platform driver: %d\n", ret);
        goto err_driver;
    }

    /* Create platform devices */
    ret = create_platform_devices();
    if (ret) {
        pr_err("Failed to create platform devices: %d\n", ret);
        goto err_devices;
    }

    pr_info("PCIe Simulator Driver loaded successfully\n");
    return 0;

err_devices:
    platform_driver_unregister(&pcie_sim_platform_driver);
err_driver:
    class_destroy(driver_state.class);
err_class:
    unregister_chrdev_region(driver_state.devt_base, DEVICE_COUNT);
    return ret;
}

/*
 * Module cleanup
 */
static void __exit pcie_sim_exit(void)
{
    pr_info("PCIe Simulator Driver unloading\n");

    destroy_platform_devices();
    platform_driver_unregister(&pcie_sim_platform_driver);
    class_destroy(driver_state.class);
    unregister_chrdev_region(driver_state.devt_base, DEVICE_COUNT);

    pr_info("PCIe Simulator Driver unloaded\n");
}

module_init(pcie_sim_init);
module_exit(pcie_sim_exit);

MODULE_AUTHOR("Karan Mamaniya <kmamaniya@gmail.com>");
MODULE_DESCRIPTION("PCIe Device Simulator for Educational Purposes");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("Dual MIT/GPL");