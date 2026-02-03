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

	if (size < sizeof(struct ml_lib_model))
		return ERR_PTR(-EINVAL);

	ml_model = kzalloc(size, gfp);
	if (unlikely(!ml_model))
		return ERR_PTR(-ENOMEM);

	atomic_set(&ml_model->mode, ML_LIB_UNKNOWN_MODE);
	atomic_set(&ml_model->state, ML_LIB_UNKNOWN_MODEL_STATE);
	ml_model->model_ops = &default_ml_model_ops;

	return (void *)ml_model;
}
EXPORT_SYMBOL(allocate_ml_model);

void free_ml_model(struct ml_lib_model *ml_model)
{
	if (!ml_model)
		return;

	free_subsystem_object(ml_model->parent);
	kfree(ml_model);
}
EXPORT_SYMBOL(free_ml_model);

void *allocate_subsystem_object(size_t size, gfp_t gfp)
{
	struct ml_lib_subsystem *subsystem;

	if (size < sizeof(struct ml_lib_subsystem))
		return ERR_PTR(-EINVAL);

	subsystem = kzalloc(size, gfp);
	if (unlikely(!subsystem))
		return ERR_PTR(-ENOMEM);

	subsystem->size = size;
	atomic_set(&subsystem->type, ML_LIB_UNKNOWN_SUBSYSTEM_TYPE);

	return (void *)subsystem;
}
EXPORT_SYMBOL(allocate_subsystem_object);

void free_subsystem_object(struct ml_lib_subsystem *object)
{
	if (!object)
		return;

	kfree(object);
}
EXPORT_SYMBOL(free_subsystem_object);

void *allocate_ml_model_options(size_t size, gfp_t gfp)
{
	struct ml_lib_model_options *options;

	if (size < sizeof(struct ml_lib_model_options))
		return ERR_PTR(-EINVAL);

	options = kzalloc(size, gfp);
	if (unlikely(!options))
		return ERR_PTR(-ENOMEM);

	options->sleep_timeout = U32_MAX;

	return (void *)options;
}
EXPORT_SYMBOL(allocate_ml_model_options);

void free_ml_model_options(struct ml_lib_model_options *options)
{
	if (!options)
		return;

	kfree(options);
}
EXPORT_SYMBOL(free_ml_model_options);

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
	struct ml_lib_dataset *dataset;

	if (size < sizeof(struct ml_lib_dataset))
		return ERR_PTR(-EINVAL);

	dataset = kzalloc(size, gfp);
	if (unlikely(!dataset))
		return ERR_PTR(-ENOMEM);

	atomic_set(&dataset->type, ML_LIB_UNKNOWN_DATASET_TYPE);
	atomic_set(&dataset->state, ML_LIB_UNKNOWN_DATASET_STATE);

	return (void *)dataset;
}
EXPORT_SYMBOL(allocate_dataset);

void free_dataset(struct ml_lib_dataset *dataset)
{
	if (!dataset)
		return;

	kfree(dataset);
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
	size_t size;
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

	spin_lock_init(&ml_model->parent_state_lock);
	spin_lock_init(&ml_model->options_lock);
	spin_lock_init(&ml_model->dataset_lock);

	err = ml_model_create_sysfs_group(ml_model, parent);
	if (err) {
		pr_err("ml_lib: failed to create sysfs group: err %d\n", err);
		goto finish_model_create;
	}

	if (!ml_model->model_ops || !ml_model->model_ops->create) {
		size = sizeof(struct ml_lib_subsystem);

		ml_model->parent = allocate_subsystem_object(size, GFP_KERNEL);
		if (unlikely(!ml_model->parent)) {
			err = -ENOMEM;
			goto remove_sysfs_group;
		}

		atomic_set(&ml_model->parent->type, ML_LIB_GENERIC_SUBSYSTEM);
	} else {
		err = ml_model->model_ops->create(ml_model);
		if (unlikely(err)) {
			pr_err("ml_lib: failed to create ML model: err %d\n",
				err);
			goto remove_sysfs_group;
		}
	}

	atomic_set(&ml_model->state, ML_LIB_MODEL_CREATED);

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
	struct ml_lib_model_options *old_options;
	int err = 0;

	if (!ml_model)
		return -EINVAL;

	if (!ml_model->model_ops || !ml_model->model_ops->init)
		options->sleep_timeout = ML_LIB_SLEEP_TIMEOUT_DEFAULT;
	else {
		err = ml_model->model_ops->init(ml_model, options);
		if (unlikely(err)) {
			pr_err("ml_lib: failed to init ML model: err %d\n",
				err);
			goto finish_model_init;
		}
	}

	spin_lock(&ml_model->options_lock);
	old_options = rcu_dereference_protected(ml_model->options,
				lockdep_is_held(&ml_model->options_lock));
	rcu_assign_pointer(ml_model->options, options);
	spin_unlock(&ml_model->options_lock);
	synchronize_rcu();
	free_ml_model_options(old_options);

	atomic_set(&ml_model->state, ML_LIB_MODEL_INITIALIZED);

finish_model_init:
	return err;
}
EXPORT_SYMBOL(ml_model_init);

int ml_model_re_init(struct ml_lib_model *ml_model,
		     struct ml_lib_model_options *options)
{
	struct ml_lib_model_options *old_options;

	if (!ml_model)
		return -EINVAL;

	spin_lock(&ml_model->options_lock);
	old_options = rcu_dereference_protected(ml_model->options,
				lockdep_is_held(&ml_model->options_lock));
	rcu_assign_pointer(ml_model->options, options);
	spin_unlock(&ml_model->options_lock);
	synchronize_rcu();
	free_ml_model_options(old_options);

	return 0;
}
EXPORT_SYMBOL(ml_model_re_init);

int ml_model_start(struct ml_lib_model *ml_model,
		   struct ml_lib_model_run_config *config)
{
	if (!ml_model)
		return -EINVAL;

	/* TODO: implement ML model start logic*/
	atomic_set(&ml_model->state, ML_LIB_MODEL_STARTED);
	pr_err("ml_lib: TODO: implement start ML model\n");
	return 0;
}
EXPORT_SYMBOL(ml_model_start);

int ml_model_stop(struct ml_lib_model *ml_model)
{
	if (!ml_model)
		return -EINVAL;

	/* TODO: implement ML model stop logic*/
	atomic_set(&ml_model->state, ML_LIB_MODEL_STOPPED);
	pr_err("ml_lib: TODO: implement stop ML model\n");
	return 0;
}
EXPORT_SYMBOL(ml_model_stop);

void ml_model_destroy(struct ml_lib_model *ml_model)
{
	struct ml_lib_model_options *old_options;
	struct ml_lib_dataset *old_dataset;

	if (!ml_model)
		return;

	atomic_set(&ml_model->state, ML_LIB_MODEL_SHUTTING_DOWN);

	ml_model_delete_sysfs_group(ml_model);

	spin_lock(&ml_model->options_lock);
	old_options = rcu_dereference_protected(ml_model->options,
				lockdep_is_held(&ml_model->options_lock));
	rcu_assign_pointer(ml_model->options, NULL);
	spin_unlock(&ml_model->options_lock);
	synchronize_rcu();
	free_ml_model_options(old_options);

	spin_lock(&ml_model->dataset_lock);
	old_dataset = rcu_dereference_protected(ml_model->dataset,
				lockdep_is_held(&ml_model->dataset_lock));
	rcu_assign_pointer(ml_model->dataset, NULL);
	spin_unlock(&ml_model->dataset_lock);
	synchronize_rcu();

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->destroy) {
		/*
		 * Do nothing
		 */
	} else
		ml_model->dataset_ops->destroy(old_dataset);

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->free)
		free_dataset(old_dataset);
	else
		ml_model->dataset_ops->free(old_dataset);

	if (!ml_model->model_ops || !ml_model->model_ops->destroy) {
		atomic_set(&ml_model->parent->type,
			   ML_LIB_UNKNOWN_SUBSYSTEM_TYPE);
	} else
		ml_model->model_ops->destroy(ml_model);

	atomic_set(&ml_model->state, ML_LIB_MODEL_STATE_MAX);
}
EXPORT_SYMBOL(ml_model_destroy);

struct ml_lib_subsystem_state *get_system_state(struct ml_lib_model *ml_model)
{
	return NULL;
}
EXPORT_SYMBOL(get_system_state);

int ml_model_get_dataset(struct ml_lib_model *ml_model,
			 struct ml_lib_request_config *config,
			 struct ml_lib_user_space_request *request)
{
	struct ml_lib_dataset *old_dataset;
	struct ml_lib_dataset *new_dataset;
	size_t desc_size = sizeof(struct ml_lib_dataset);
	int state;
	int err = 0;

	if (!ml_model)
		return -EINVAL;

	atomic_set(&ml_model->state, ML_LIB_MODEL_RUNNING);

	rcu_read_lock();
	old_dataset = rcu_dereference(ml_model->dataset);
	if (old_dataset)
		state = atomic_read(&old_dataset->state);
	else
		state = ML_LIB_UNKNOWN_DATASET_STATE;
	rcu_read_unlock();

	switch (state) {
	case ML_LIB_DATASET_CLEAN:
	case ML_LIB_DATASET_EXTRACTED_PARTIALLY:
	case ML_LIB_DATASET_EXTRACTED_COMPLETELY:
		/* nothing should be done */
		goto finish_get_dataset;

	default:
		/* continue logic */
		break;
	}

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->allocate)
		new_dataset = allocate_dataset(desc_size, GFP_KERNEL);
	else {
		new_dataset = ml_model->dataset_ops->allocate(desc_size,
							      GFP_KERNEL);
	}

	if (IS_ERR(new_dataset)) {
		err = PTR_ERR(new_dataset);
		pr_err("ml_lib: Failed to allocate dataset\n");
		return err;
	} else if (!new_dataset) {
		err = -ENOMEM;
		pr_err("ml_lib: Failed to allocate dataset\n");
		return err;
	}

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->init) {
		/*
		 * Do nothing
		 */
	} else {
		err = ml_model->dataset_ops->init(new_dataset);
		if (err) {
			pr_err("ml_lib: Failed to init dataset: err %d\n",
				err);
			goto fail_get_dataset;
		}
	}

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->extract) {
		atomic_set(&new_dataset->type, ML_LIB_EMPTY_DATASET);
		atomic_set(&new_dataset->state, ML_LIB_DATASET_CLEAN);
		new_dataset->allocated_size = 0;
		new_dataset->portion_offset = 0;
		new_dataset->portion_size = 0;
	} else {
		err = ml_model->dataset_ops->extract(ml_model, new_dataset);
		if (err) {
			pr_err("ml_lib: Failed to extract dataset: err %d\n",
				err);
			goto fail_get_dataset;
		}
	}

	spin_lock(&ml_model->dataset_lock);
	old_dataset = rcu_dereference_protected(ml_model->dataset,
				lockdep_is_held(&ml_model->dataset_lock));
	rcu_assign_pointer(ml_model->dataset, new_dataset);
	spin_unlock(&ml_model->dataset_lock);
	synchronize_rcu();

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->destroy) {
		/*
		 * Do nothing
		 */
	} else
		ml_model->dataset_ops->destroy(old_dataset);

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->free)
		free_dataset(old_dataset);
	else
		ml_model->dataset_ops->free(old_dataset);

finish_get_dataset:
	return 0;

fail_get_dataset:
	if (!ml_model->dataset_ops || !ml_model->dataset_ops->destroy) {
		/*
		 * Do nothing
		 */
	} else
		ml_model->dataset_ops->destroy(new_dataset);

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->free)
		free_dataset(new_dataset);
	else
		ml_model->dataset_ops->free(new_dataset);

	return err;
}
EXPORT_SYMBOL(ml_model_get_dataset);

int ml_model_discard_dataset(struct ml_lib_model *ml_model)
{
	struct ml_lib_dataset *old_dataset;
	struct ml_lib_dataset *new_dataset;
	size_t desc_size = sizeof(struct ml_lib_dataset);
	int err;

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->allocate)
		new_dataset = allocate_dataset(desc_size, GFP_KERNEL);
	else {
		new_dataset = ml_model->dataset_ops->allocate(desc_size,
							      GFP_KERNEL);
	}

	if (IS_ERR(new_dataset)) {
		err = PTR_ERR(new_dataset);
		pr_err("ml_lib: Failed to allocate dataset\n");
		return err;
	} else if (!new_dataset) {
		err = -ENOMEM;
		pr_err("ml_lib: Failed to allocate dataset\n");
		return err;
	}

	spin_lock(&ml_model->dataset_lock);
	old_dataset = rcu_dereference_protected(ml_model->dataset,
				lockdep_is_held(&ml_model->dataset_lock));
	if (old_dataset) {
		atomic_set(&new_dataset->type, atomic_read(&old_dataset->type));
		new_dataset->allocated_size = old_dataset->allocated_size;
		new_dataset->portion_offset = old_dataset->portion_offset;
		new_dataset->portion_size = old_dataset->portion_size;
	} else {
		atomic_set(&new_dataset->type, ML_LIB_EMPTY_DATASET);
		new_dataset->allocated_size = 0;
		new_dataset->portion_offset = 0;
		new_dataset->portion_size = 0;
	}
	atomic_set(&new_dataset->state, ML_LIB_DATASET_OBSOLETE);
	rcu_assign_pointer(ml_model->dataset, new_dataset);
	spin_unlock(&ml_model->dataset_lock);
	synchronize_rcu();

	if (!ml_model->dataset_ops || !ml_model->dataset_ops->free)
		free_dataset(old_dataset);
	else
		ml_model->dataset_ops->free(old_dataset);

	return 0;
}
EXPORT_SYMBOL(ml_model_discard_dataset);

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
	size_t size = sizeof(struct ml_lib_subsystem);

	ml_model->parent = allocate_subsystem_object(size, GFP_KERNEL);
	if (unlikely(!ml_model->parent))
		return -ENOMEM;

	atomic_set(&ml_model->parent->type, ML_LIB_GENERIC_SUBSYSTEM);
	atomic_set(&ml_model->mode, ML_LIB_EMERGENCY_MODE);

	return 0;
}
EXPORT_SYMBOL(generic_create_ml_model);

int generic_init_ml_model(struct ml_lib_model *ml_model,
			  struct ml_lib_model_options *options)
{
	options->sleep_timeout = ML_LIB_SLEEP_TIMEOUT_DEFAULT;
	return 0;
}
EXPORT_SYMBOL(generic_init_ml_model);

int generic_re_init_ml_model(struct ml_lib_model *ml_model,
			     struct ml_lib_model_options *options)
{
	options->sleep_timeout = ML_LIB_SLEEP_TIMEOUT_DEFAULT;
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
	atomic_set(&ml_model->parent->type, ML_LIB_UNKNOWN_SUBSYSTEM_TYPE);
	atomic_set(&ml_model->mode, ML_LIB_UNKNOWN_MODE);
}
EXPORT_SYMBOL(generic_destroy_ml_model);

struct ml_lib_subsystem_state *
generic_get_system_state(struct ml_lib_model *ml_model)
{
	return NULL;
}
EXPORT_SYMBOL(generic_get_system_state);

int generic_get_dataset(struct ml_lib_model *ml_model,
			struct ml_lib_dataset *dataset)
{
	atomic_set(&dataset->type, ML_LIB_EMPTY_DATASET);
	atomic_set(&dataset->state, ML_LIB_DATASET_CLEAN);
	dataset->allocated_size = 0;
	dataset->portion_offset = 0;
	dataset->portion_size = 0;

	return 0;
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
