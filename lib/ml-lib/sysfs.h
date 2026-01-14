// SPDX-License-Identifier: GPL-2.0-only
/*
 * Machine Learning (ML) library
 *
 * Copyright (C) 2025-2026 Viacheslav Dubeyko <slava@dubeyko.com>
 */

#ifndef _LINUX_ML_LIB_SYSFS_H
#define _LINUX_ML_LIB_SYSFS_H

#include <linux/sysfs.h>

int ml_model_create_sysfs_group(struct ml_lib_model *ml_model,
				struct kobject *subsystem_kobj);
void ml_model_delete_sysfs_group(struct ml_lib_model *ml_model);

#endif /* _LINUX_ML_LIB_SYSFS_H */
