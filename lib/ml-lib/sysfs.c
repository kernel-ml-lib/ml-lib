// SPDX-License-Identifier: GPL-2.0-only
/*
 * Machine Learning (ML) library
 *
 * Copyright (C) 2025-2026 Viacheslav Dubeyko <slava@dubeyko.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/ml-lib/ml_lib.h>

#include "sysfs.h"

struct ml_lib_feature_attr {
	struct attribute attr;
	ssize_t (*show)(struct ml_lib_feature_attr *,
			struct ml_lib_model *,
			char *);
	ssize_t (*store)(struct ml_lib_feature_attr *,
				struct ml_lib_model *,
				const char *, size_t);
};

#define ML_LIB_ATTR(type, name, mode, show, store) \
	static struct ml_lib_##type##_attr ml_lib_##type##_attr_##name = \
		__ATTR(name, mode, show, store)

#define ML_LIB_FEATURE_INFO_ATTR(name) \
	ML_LIB_ATTR(feature, name, 0444, NULL, NULL)
#define ML_LIB_FEATURE_RO_ATTR(name) \
	ML_LIB_ATTR(feature, name, 0444, ml_lib_feature_##name##_show, NULL)
#define ML_LIB_FEATURE_W_ATTR(name) \
	ML_LIB_ATTR(feature, name, 0220, NULL, ml_lib_feature_##name##_store)
#define ML_LIB_FEATURE_RW_ATTR(name) \
	ML_LIB_ATTR(feature, name, 0644, \
		    ml_lib_feature_##name##_show, ml_lib_feature_##name##_store)

enum {
	ML_LIB_START_COMMAND,
	ML_LIB_STOP_COMMAND,
	ML_LIB_COMMAND_NUMBER
};

static const char *control_command_str[ML_LIB_COMMAND_NUMBER] = {
	"start",
	"stop",
};

static ssize_t ml_lib_feature_control_store(struct ml_lib_feature_attr *attr,
					    struct ml_lib_model *ml_model,
					    const char *buf, size_t len)
{
	struct ml_lib_model_run_config config = {0};
	int i;
	int err;

	for (i = 0; i < ML_LIB_COMMAND_NUMBER; i++) {
		size_t iter_len = min(len, strlen(control_command_str[i]));

		if (strncmp(control_command_str[i], buf, iter_len) == 0)
			break;
	}

	if (i >= ML_LIB_COMMAND_NUMBER)
		return -EOPNOTSUPP;

	switch (i) {
	case ML_LIB_START_COMMAND:
		err = ml_model_start(ml_model, &config);
		break;

	case ML_LIB_STOP_COMMAND:
		err = ml_model_stop(ml_model);
		break;
	}

	if (unlikely(err))
		return err;

	return len;
}

ML_LIB_FEATURE_W_ATTR(control);

static struct attribute *ml_model_attrs[] = {
	&ml_lib_feature_attr_control.attr,
	NULL,
};

static const struct attribute_group ml_model_group = {
	.attrs = ml_model_attrs,
};

static const struct attribute_group *ml_model_groups[] = {
	&ml_model_group,
	NULL,
};

static
ssize_t ml_model_attr_show(struct kobject *kobj,
			   struct attribute *attr,
			   char *buf)
{
	struct ml_lib_model *ml_model = container_of(kobj,
						     struct ml_lib_model,
						     kobj);
	struct ml_lib_feature_attr *ml_model_attr =
			container_of(attr, struct ml_lib_feature_attr, attr);

	if (!ml_model_attr->show)
		return -EIO;

	return ml_model_attr->show(ml_model_attr, ml_model, buf);
}

static
ssize_t ml_model_attr_store(struct kobject *kobj,
			    struct attribute *attr,
			    const char *buf, size_t len)
{
	struct ml_lib_model *ml_model = container_of(kobj,
						     struct ml_lib_model,
						     kobj);
	struct ml_lib_feature_attr *ml_model_attr =
			container_of(attr, struct ml_lib_feature_attr, attr);

	if (!ml_model_attr->store)
		return -EIO;

	return ml_model_attr->store(ml_model_attr, ml_model, buf, len);
}

static const struct sysfs_ops ml_model_attr_ops = {
	.show	= ml_model_attr_show,
	.store	= ml_model_attr_store,
};

static inline
void ml_model_kobj_release(struct kobject *kobj)
{
	struct ml_lib_model *ml_model = container_of(kobj,
						     struct ml_lib_model,
						     kobj);
	complete(&ml_model->kobj_unregister);
}

static struct kobj_type ml_model_ktype = {
	.default_groups = ml_model_groups,
	.sysfs_ops	= &ml_model_attr_ops,
	.release	= ml_model_kobj_release,
};

int ml_model_create_sysfs_group(struct ml_lib_model *ml_model,
				struct kobject *subsystem_kobj)
{
	int err;

	init_completion(&ml_model->kobj_unregister);

	err = kobject_init_and_add(&ml_model->kobj, &ml_model_ktype,
				   subsystem_kobj,
				   "%s", ml_model->model_name);
	if (err)
		pr_err("ml_lib: failed to create sysfs group: err %d\n", err);

	return err;
}

void ml_model_delete_sysfs_group(struct ml_lib_model *ml_model)
{
	kobject_del(&ml_model->kobj);
	kobject_put(&ml_model->kobj);
	wait_for_completion(&ml_model->kobj_unregister);
}
