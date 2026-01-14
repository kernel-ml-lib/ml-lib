// SPDX-License-Identifier: GPL-2.0
/*
 * Machine Learning (ML) library
 * Testing Character Device Driver
 *
 * Copyright (C) 2025-2026 Viacheslav Dubeyko <slava@dubeyko.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>
#include <linux/ml-lib/ml_lib.h>

#define DEVICE_NAME "mllibdev"
#define CLASS_NAME "ml_lib_test"
#define BUFFER_SIZE 1024

/* IOCTL commands */
#define ML_LIB_TEST_DEV_IOC_MAGIC   'M'
#define ML_LIB_TEST_DEV_IOCRESET    _IO(ML_LIB_TEST_DEV_IOC_MAGIC, 0)
#define ML_LIB_TEST_DEV_IOCGETSIZE  _IOR(ML_LIB_TEST_DEV_IOC_MAGIC, 1, int)
#define ML_LIB_TEST_DEV_IOCSETSIZE  _IOW(ML_LIB_TEST_DEV_IOC_MAGIC, 2, int)

/* Device data structure */
struct ml_lib_test_dev_data {
	struct cdev cdev;
	struct device *device;
	char *buffer;
	size_t buffer_size;
	size_t data_size;
	struct mutex lock;
	unsigned long access_count;
	unsigned long read_count;
	unsigned long write_count;

	struct ml_lib_model *ml_model1;
};

#define ML_MODEL_1_NAME "ml_model1"

static dev_t dev_number;
static struct class *ml_lib_test_dev_class;
static struct ml_lib_test_dev_data *dev_data;
static struct proc_dir_entry *proc_entry;

/* File operations */
static int ml_lib_test_dev_open(struct inode *inode, struct file *file)
{
	struct ml_lib_test_dev_data *data = container_of(inode->i_cdev,
						struct ml_lib_test_dev_data,
						cdev);

	file->private_data = data;

	mutex_lock(&data->lock);
	data->access_count++;
	mutex_unlock(&data->lock);

	pr_info("ml_lib_test_dev: Device opened (total opens: %lu)\n",
		data->access_count);

	return 0;
}

static int ml_lib_test_dev_release(struct inode *inode, struct file *file)
{
	pr_info("ml_lib_test_dev: Device closed\n");
	return 0;
}

static ssize_t ml_lib_test_dev_read(struct file *file, char __user *buf,
				    size_t count, loff_t *ppos)
{
	struct ml_lib_test_dev_data *data = file->private_data;
	size_t to_read;
	int ret;

	mutex_lock(&data->lock);

	if (*ppos >= data->data_size) {
		mutex_unlock(&data->lock);
		return 0;
	}

	to_read = min(count, data->data_size - (size_t)*ppos);

	ret = copy_to_user(buf, data->buffer + *ppos, to_read);
	if (ret) {
		mutex_unlock(&data->lock);
		return -EFAULT;
	}

	*ppos += to_read;
	data->read_count++;

	mutex_unlock(&data->lock);

	pr_info("ml_lib_test_dev: Read %zu bytes\n", to_read);

	return to_read;
}

static ssize_t ml_lib_test_dev_write(struct file *file, const char __user *buf,
				     size_t count, loff_t *ppos)
{
	struct ml_lib_test_dev_data *data = file->private_data;
	size_t to_write;
	int ret;

	mutex_lock(&data->lock);

	if (*ppos >= data->buffer_size) {
		mutex_unlock(&data->lock);
		return -ENOSPC;
	}

	to_write = min(count, data->buffer_size - (size_t)*ppos);

	ret = copy_from_user(data->buffer + *ppos, buf, to_write);
	if (ret) {
		mutex_unlock(&data->lock);
		return -EFAULT;
	}

	*ppos += to_write;
	if (*ppos > data->data_size)
		data->data_size = *ppos;

	data->write_count++;

	mutex_unlock(&data->lock);

	pr_info("ml_lib_test_dev: Wrote %zu bytes\n", to_write);

	return to_write;
}

static long ml_lib_test_dev_ioctl(struct file *file, unsigned int cmd,
				  unsigned long arg)
{
	struct ml_lib_test_dev_data *data = file->private_data;
	int size;

	switch (cmd) {
	case ML_LIB_TEST_DEV_IOCRESET:
		mutex_lock(&data->lock);
		memset(data->buffer, 0, data->buffer_size);
		data->data_size = 0;
		mutex_unlock(&data->lock);
		pr_info("ml_lib_test_dev: Buffer reset via IOCTL\n");
		break;

	case ML_LIB_TEST_DEV_IOCGETSIZE:
		mutex_lock(&data->lock);
		size = data->data_size;
		mutex_unlock(&data->lock);
		if (copy_to_user((int __user *)arg, &size, sizeof(size)))
			return -EFAULT;
		break;

	case ML_LIB_TEST_DEV_IOCSETSIZE:
		if (copy_from_user(&size, (int __user *)arg, sizeof(size)))
			return -EFAULT;
		if (size < 0 || size > data->buffer_size)
			return -EINVAL;
		mutex_lock(&data->lock);
		data->data_size = size;
		mutex_unlock(&data->lock);
		pr_info("ml_lib_test_dev: Data size set to %d via IOCTL\n", size);
		break;

	default:
		return -ENOTTY;
	}

	return 0;
}

static const struct file_operations ml_lib_test_dev_fops = {
	.owner = THIS_MODULE,
	.open = ml_lib_test_dev_open,
	.release = ml_lib_test_dev_release,
	.read = ml_lib_test_dev_read,
	.write = ml_lib_test_dev_write,
	.unlocked_ioctl = ml_lib_test_dev_ioctl,
	.llseek = default_llseek,
};

/* Sysfs attributes */
static ssize_t buffer_size_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct ml_lib_test_dev_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%zu\n", data->buffer_size);
}

static ssize_t data_size_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct ml_lib_test_dev_data *data = dev_get_drvdata(dev);
	size_t size;

	mutex_lock(&data->lock);
	size = data->data_size;
	mutex_unlock(&data->lock);

	return sprintf(buf, "%zu\n", size);
}

static ssize_t access_count_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct ml_lib_test_dev_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", data->access_count);
}

static ssize_t stats_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct ml_lib_test_dev_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "Opens: %lu\nReads: %lu\nWrites: %lu\n",
		       data->access_count, data->read_count,
		       data->write_count);
}

static DEVICE_ATTR_RO(buffer_size);
static DEVICE_ATTR_RO(data_size);
static DEVICE_ATTR_RO(access_count);
static DEVICE_ATTR_RO(stats);

static struct attribute *ml_lib_test_dev_attrs[] = {
	&dev_attr_buffer_size.attr,
	&dev_attr_data_size.attr,
	&dev_attr_access_count.attr,
	&dev_attr_stats.attr,
	NULL,
};

static const struct attribute_group ml_lib_test_dev_attr_group = {
	.attrs = ml_lib_test_dev_attrs,
};

/* Procfs operations */
static int ml_lib_test_dev_proc_show(struct seq_file *m, void *v)
{
	struct ml_lib_test_dev_data *data = dev_data;

	seq_printf(m, "ML Library Testing Device Driver Information\n");
	seq_printf(m, "=================================\n");
	seq_printf(m, "Device name:     %s\n", DEVICE_NAME);
	seq_printf(m, "Buffer size:     %zu bytes\n", data->buffer_size);
	seq_printf(m, "Data size:       %zu bytes\n", data->data_size);
	seq_printf(m, "Access count:    %lu\n", data->access_count);
	seq_printf(m, "Read count:      %lu\n", data->read_count);
	seq_printf(m, "Write count:     %lu\n", data->write_count);

	return 0;
}

static int ml_lib_test_dev_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ml_lib_test_dev_proc_show, NULL);
}

static const struct proc_ops ml_lib_test_dev_proc_ops = {
	.proc_open = ml_lib_test_dev_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

/* Module initialization */
static int __init ml_lib_test_dev_init(void)
{
	struct ml_lib_model_options options = {};
	int ret;

	pr_info("ml_lib_test_dev: Initializing driver\n");

	/* Allocate device data */
	dev_data = kzalloc(sizeof(struct ml_lib_test_dev_data), GFP_KERNEL);
	if (!dev_data)
		return -ENOMEM;

	/* Allocate buffer */
	dev_data->buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
	if (!dev_data->buffer) {
		ret = -ENOMEM;
		goto err_free_data;
	}

	dev_data->buffer_size = BUFFER_SIZE;
	dev_data->data_size = 0;
	mutex_init(&dev_data->lock);

	/* Allocate device number */
	ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		pr_err("ml_lib_test_dev: Failed to allocate device number\n");
		goto err_free_buffer;
	}

	pr_info("ml_lib_test_dev: Device number allocated: %d:%d\n",
		MAJOR(dev_number), MINOR(dev_number));

	/* Create device class */
	ml_lib_test_dev_class = class_create(CLASS_NAME);
	if (IS_ERR(ml_lib_test_dev_class)) {
		ret = PTR_ERR(ml_lib_test_dev_class);
		pr_err("ml_lib_test_dev: Failed to create class\n");
		goto err_unregister_chrdev;
	}

	/* Initialize and add cdev */
	cdev_init(&dev_data->cdev, &ml_lib_test_dev_fops);
	dev_data->cdev.owner = THIS_MODULE;

	ret = cdev_add(&dev_data->cdev, dev_number, 1);
	if (ret < 0) {
		pr_err("ml_lib_test_dev: Failed to add cdev\n");
		goto err_class_destroy;
	}

	/* Create device */
	dev_data->device = device_create(ml_lib_test_dev_class,
					 NULL, dev_number,
					 dev_data, DEVICE_NAME);
	if (IS_ERR(dev_data->device)) {
		ret = PTR_ERR(dev_data->device);
		pr_err("ml_lib_test_dev: Failed to create device\n");
		goto err_cdev_del;
	}

	/* Create sysfs attributes */
	ret = sysfs_create_group(&dev_data->device->kobj,
				 &ml_lib_test_dev_attr_group);
	if (ret < 0) {
		pr_err("ml_lib_test_dev: Failed to create sysfs group\n");
		goto err_device_destroy;
	}

	/* Create procfs entry */
	proc_entry = proc_create(DEVICE_NAME, 0444, NULL,
				 &ml_lib_test_dev_proc_ops);
	if (!proc_entry) {
		pr_err("ml_lib_test_dev: Failed to create proc entry\n");
		ret = -ENOMEM;
		goto err_sysfs_remove;
	}

	dev_data->ml_model1 = allocate_ml_model(sizeof(struct ml_lib_model),
						GFP_KERNEL);
	if (IS_ERR(dev_data->ml_model1)) {
		ret = PTR_ERR(dev_data->ml_model1);
		pr_err("ml_lib_test_dev: Failed to allocate ML model\n");
		goto err_procfs_remove;
	} else if (!dev_data->ml_model1) {
		ret = -ENOMEM;
		pr_err("ml_lib_test_dev: Failed to allocate ML model\n");
		goto err_procfs_remove;
	}

	ret = ml_model_create(dev_data->ml_model1, CLASS_NAME,
			      ML_MODEL_1_NAME, &dev_data->device->kobj);
	if (ret < 0) {
		pr_err("ml_lib_test_dev: Failed to create ML model\n");
		goto err_ml_model_free;
	}

	ret = ml_model_init(dev_data->ml_model1, &options);
	if (ret < 0) {
		pr_err("ml_lib_test_dev: Failed to init ML model\n");
		goto err_ml_model_destroy;
	}

	pr_info("ml_lib_test_dev: Driver initialized successfully\n");
	pr_info("ml_lib_test_dev: Device created at /dev/%s\n",
		DEVICE_NAME);
	pr_info("ml_lib_test_dev: Proc entry created at /proc/%s\n",
		DEVICE_NAME);

	return 0;

err_ml_model_destroy:
	ml_model_destroy(dev_data->ml_model1);
err_ml_model_free:
	free_ml_model(dev_data->ml_model1);
err_procfs_remove:
	proc_remove(proc_entry);
err_sysfs_remove:
	sysfs_remove_group(&dev_data->device->kobj,
			   &ml_lib_test_dev_attr_group);
err_device_destroy:
	device_destroy(ml_lib_test_dev_class, dev_number);
err_cdev_del:
	cdev_del(&dev_data->cdev);
err_class_destroy:
	class_destroy(ml_lib_test_dev_class);
err_unregister_chrdev:
	unregister_chrdev_region(dev_number, 1);
err_free_buffer:
	kfree(dev_data->buffer);
err_free_data:
	kfree(dev_data);
	return ret;
}

/* Module cleanup */
static void __exit ml_lib_test_dev_exit(void)
{
	pr_info("ml_lib_test_dev: Cleaning up driver\n");

	/* Destroy ML model */
	ml_model_destroy(dev_data->ml_model1);
	free_ml_model(dev_data->ml_model1);

	/* Remove procfs entry */
	proc_remove(proc_entry);

	/* Remove sysfs attributes */
	sysfs_remove_group(&dev_data->device->kobj,
			   &ml_lib_test_dev_attr_group);

	/* Destroy device */
	device_destroy(ml_lib_test_dev_class, dev_number);

	/* Delete cdev */
	cdev_del(&dev_data->cdev);

	/* Destroy class */
	class_destroy(ml_lib_test_dev_class);

	/* Unregister device number */
	unregister_chrdev_region(dev_number, 1);

	/* Free buffers */
	kfree(dev_data->buffer);
	kfree(dev_data);

	pr_info("ml_lib_test_dev: Driver removed successfully\n");
}

module_init(ml_lib_test_dev_init);
module_exit(ml_lib_test_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Viacheslav Dubeyko <slava@dubeyko.com>");
MODULE_DESCRIPTION("ML libraray testing character device driver");
MODULE_VERSION("1.0");
