/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Machine Learning (ML) library
 *
 * Userspace API for ml_lib_dev testing driver
 *
 * Copyright (C) 2025-2026 Viacheslav Dubeyko <slava@dubeyko.com>
 */

#ifndef _ML_LIB_TEST_DEV_IOCTL_H
#define _ML_LIB_TEST_DEV_IOCTL_H

#include <linux/ioctl.h>

/* IOCTL commands */
#define ML_LIB_TEST_DEV_IOC_MAGIC   'M'
#define ML_LIB_TEST_DEV_IOCRESET    _IO(ML_LIB_TEST_DEV_IOC_MAGIC, 0)
#define ML_LIB_TEST_DEV_IOCGETSIZE  _IOR(ML_LIB_TEST_DEV_IOC_MAGIC, 1, int)
#define ML_LIB_TEST_DEV_IOCSETSIZE  _IOW(ML_LIB_TEST_DEV_IOC_MAGIC, 2, int)

#endif /* _ML_LIB_TEST_DEV_IOCTL_H */
