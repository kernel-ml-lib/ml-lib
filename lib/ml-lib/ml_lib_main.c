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

#define UNKNOWN_SUBSYSTEM_NAME "unknown_subsystem"
#define UNKNOWN_ML_MODEL_NAME "unknown_model"

/*
 * default_ml_model_ops - default ML model operations
 */
struct ml_lib_model_operations default_ml_model_ops = {
	.create				= generic_create_ml_model,
	.init				= generic_init_ml_model,
	.re_init			= generic_re_init_ml_model,
	.start				= generic_start_ml_model,
	.stop				= generic_stop_ml_model,
	.destroy			= generic_destroy_ml_model,
	.get_system_state		= generic_get_system_state,
	.get_dataset			= generic_get_dataset,
	.preprocess_data		= generic_preprocess_data,
	.publish_data			= generic_publish_data,
	.preprocess_recommendation	= generic_preprocess_recommendation,
	.estimate_system_state		= generic_estimate_system_state,
	.apply_recommendation		= generic_apply_recommendation,
	.execute_operation		= generic_execute_operation,
	.estimate_efficiency		= generic_estimate_efficiency,
	.error_backpropagation		= generic_error_backpropagation,
	.correct_system_state		= generic_correct_system_state,
};

/******************************************************************************
 *                             ML library API                                 *
 ******************************************************************************/

void *allocate_ml_model(size_t size, gfp_t gfp)
{
	struct ml_lib_model *ml_model;

	ml_model = kzalloc(size, gfp);
	if (!ml_model)
		return ERR_PTR(-ENOMEM);

	atomic_set(&ml_model->mode, ML_LIB_UNKNOWN_MODE);
	ml_model->model_ops = &default_ml_model_ops;

	return (void *)ml_model;
}
EXPORT_SYMBOL(allocate_ml_model);

void free_ml_model(struct ml_lib_model *ml_model)
{
	if (ml_model)
		kfree(ml_model);
}
EXPORT_SYMBOL(free_ml_model);


void *allocate_subsystem_object(size_t size, gfp_t gfp)
{
	return NULL;
}
EXPORT_SYMBOL(allocate_subsystem_object);

void free_subsystem_object(struct ml_lib_subsystem *object)
{
}
EXPORT_SYMBOL(free_subsystem_object);

void *allocate_subsystem_state(size_t size, gfp_t gfp)
{
	return NULL;
}
EXPORT_SYMBOL(allocate_subsystem_state);

void free_subsystem_state(struct ml_lib_subsystem_state *state)
{
}
EXPORT_SYMBOL(free_subsystem_state);

void *allocate_dataset(size_t size, gfp_t gfp)
{
	return NULL;
}
EXPORT_SYMBOL(allocate_dataset);

void free_dataset(struct ml_lib_dataset *dataset)
{
}
EXPORT_SYMBOL(free_dataset);

void *allocate_request_config(size_t size, gfp_t gfp)
{
	return NULL;
}
EXPORT_SYMBOL(allocate_request_config);

void free_request_config(struct ml_lib_request_config *config)
{
}
EXPORT_SYMBOL(free_request_config);

int ml_model_create(struct ml_lib_model *ml_model,
		    const char *subsystem_name,
		    const char *model_name,
		    struct kobject *subsystem_kobj)
{
	struct kobject *parent = NULL;
	int err = 0;

	if (!ml_model)
		return -EINVAL;

	if (!subsystem_name)
		ml_model->subsystem_name = UNKNOWN_SUBSYSTEM_NAME;
	else
		ml_model->subsystem_name = subsystem_name;

	if (!model_name)
		ml_model->model_name = UNKNOWN_ML_MODEL_NAME;
	else
		ml_model->model_name = model_name;

	if (!subsystem_kobj)
		parent = kernel_kobj;
	else
		parent = subsystem_kobj;

	err = ml_model_create_sysfs_group(ml_model, parent);
	if (err) {
		pr_err("ml_lib: failed to create sysfs group: err %d\n", err);
		goto finish_model_create;
	}

	if (ml_model->model_ops->create) {
		err = ml_model->model_ops->create(ml_model);
		if (unlikely(err)) {
			pr_err("ml_lib: failed to create ML model: err %d\n",
				err);
			goto remove_sysfs_group;
		}
	}

	return 0;

remove_sysfs_group:
	ml_model_delete_sysfs_group(ml_model);

finish_model_create:
	return err;
}
EXPORT_SYMBOL(ml_model_create);

int ml_model_init(struct ml_lib_model *ml_model,
		  struct ml_lib_model_options *options)
{
	return 0;
}
EXPORT_SYMBOL(ml_model_init);

int ml_model_re_init(struct ml_lib_model *ml_model,
		     struct ml_lib_model_options *options)
{
	return 0;
}
EXPORT_SYMBOL(ml_model_re_init);

int ml_model_start(struct ml_lib_model *ml_model,
		   struct ml_lib_model_run_config *config)
{
	pr_err("ml_lib: TODO: implement start ML model\n");
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(ml_model_start);

int ml_model_stop(struct ml_lib_model *ml_model)
{
	pr_err("ml_lib: TODO: implement stop ML model\n");
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(ml_model_stop);

void ml_model_destroy(struct ml_lib_model *ml_model)
{
	if (!ml_model)
		return;

	ml_model_delete_sysfs_group(ml_model);

	if (ml_model->model_ops->destroy)
		ml_model->model_ops->destroy(ml_model);
}
EXPORT_SYMBOL(ml_model_destroy);

struct ml_lib_subsystem_state *get_system_state(struct ml_lib_model *ml_model)
{
	return NULL;
}
EXPORT_SYMBOL(get_system_state);

struct ml_lib_dataset *get_dataset(struct ml_lib_model *ml_model,
				   struct ml_lib_request_config *config,
				   struct ml_lib_user_space_request *request)
{
	return NULL;
}
EXPORT_SYMBOL(get_dataset);

int ml_model_preprocess_data(struct ml_lib_model *ml_model,
			     struct ml_lib_dataset *dataset)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(ml_model_preprocess_data);

int ml_model_publish_data(struct ml_lib_model *ml_model,
			  struct ml_lib_dataset *dataset,
			  struct ml_lib_user_space_notification *notify)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(ml_model_publish_data);

int ml_model_preprocess_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(ml_model_preprocess_recommendation);

int estimate_system_state(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(estimate_system_state);

int apply_ml_model_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(apply_ml_model_recommendation);

int execute_ml_model_operation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(execute_ml_model_operation);

int estimate_ml_model_efficiency(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(estimate_ml_model_efficiency);

int ml_model_error_backpropagation(struct ml_lib_model *ml_model,
			    struct ml_lib_backpropagation_feedback *feedback,
			    struct ml_lib_user_space_notification *notify)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(ml_model_error_backpropagation);

int correct_system_state(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(correct_system_state);

/******************************************************************************
 *              Generic implementation of ML model's methods                  *
 ******************************************************************************/

int generic_create_ml_model(struct ml_lib_model *ml_model)
{
	atomic_set(&ml_model->mode, ML_LIB_EMERGENCY_MODE);
	return 0;
}
EXPORT_SYMBOL(generic_create_ml_model);

int generic_init_ml_model(struct ml_lib_model *ml_model,
			  struct ml_lib_model_options *options)
{
	return 0;
}
EXPORT_SYMBOL(generic_init_ml_model);

int generic_re_init_ml_model(struct ml_lib_model *ml_model,
			     struct ml_lib_model_options *options)
{
	return 0;
}
EXPORT_SYMBOL(generic_re_init_ml_model);

int generic_start_ml_model(struct ml_lib_model *ml_model,
			   struct ml_lib_model_run_config *config)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_start_ml_model);

int generic_stop_ml_model(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_stop_ml_model);

void generic_destroy_ml_model(struct ml_lib_model *ml_model)
{
	atomic_set(&ml_model->mode, ML_LIB_UNKNOWN_MODE);
}
EXPORT_SYMBOL(generic_destroy_ml_model);

struct ml_lib_subsystem_state *
generic_get_system_state(struct ml_lib_model *ml_model)
{
	return NULL;
}
EXPORT_SYMBOL(generic_get_system_state);

struct ml_lib_dataset *
generic_get_dataset(struct ml_lib_model *ml_model,
		    struct ml_lib_request_config *config,
		    struct ml_lib_user_space_request *request)
{
	return NULL;
}
EXPORT_SYMBOL(generic_get_dataset);

int generic_preprocess_data(struct ml_lib_model *ml_model,
			    struct ml_lib_dataset *dataset)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_preprocess_data);

int generic_publish_data(struct ml_lib_model *ml_model,
			 struct ml_lib_dataset *dataset,
			 struct ml_lib_user_space_notification *notify)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_publish_data);

int generic_preprocess_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_preprocess_recommendation);

int generic_estimate_system_state(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_estimate_system_state);

int generic_apply_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_apply_recommendation);

int generic_execute_operation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_execute_operation);

int generic_estimate_efficiency(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_estimate_efficiency);

int generic_error_backpropagation(struct ml_lib_model *ml_model,
			    struct ml_lib_backpropagation_feedback *feedback,
			    struct ml_lib_user_space_notification *notify)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_error_backpropagation);

int generic_correct_system_state(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(generic_correct_system_state);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Viacheslav Dubeyko <slava@dubeyko.com>");
MODULE_DESCRIPTION("ML library");
MODULE_VERSION("1.0");
