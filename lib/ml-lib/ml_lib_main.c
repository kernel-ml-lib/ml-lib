// SPDX-License-Identifier: GPL-2.0-only
/*
 * Machine Learning (ML) library
 *
 * Copyright (C) 2025-2026 Viacheslav Dubeyko <slava@dubeyko.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/ml-lib/ml_lib.h>

struct ml_lib_model *allocate_ml_model(size_t size, gfp_t gfp)
{
	return NULL;
}

void free_ml_model(struct ml_lib_model *ml_model)
{
}

int generic_create_ml_model(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}

int generic_init_ml_model(struct ml_lib_model *ml_model,
			  struct ml_lib_model_options *options)
{
	return -EOPNOTSUPP;
}

int generic_re_init_ml_model(struct ml_lib_model *ml_model,
			     struct ml_lib_model_options *options)
{
	return -EOPNOTSUPP;
}

int generic_start_ml_model(struct ml_lib_model *ml_model,
			   struct ml_lib_model_run_config *config)
{
	return -EOPNOTSUPP;
}

int generic_stop_ml_model(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}

void generic_destroy_ml_model(struct ml_lib_model *ml_model)
{
}

struct ml_lib_subsystem_state *
generic_get_system_state(struct ml_lib_model *ml_model)
{
	return NULL;
}

struct ml_lib_dataset *
generic_get_dataset(struct ml_lib_model *ml_model,
		    struct ml_lib_request_config *config,
		    struct ml_lib_user_space_request *request)
{
	return NULL;
}

int generic_preprocess_data(struct ml_lib_model *ml_model,
			    struct ml_lib_dataset *dataset)
{
	return -EOPNOTSUPP;
}

int generic_publish_data(struct ml_lib_model *ml_model,
			 struct ml_lib_dataset *dataset,
			 struct ml_lib_user_space_notification *notify)
{
	return -EOPNOTSUPP;
}

int generic_preprocess_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint)
{
	return -EOPNOTSUPP;
}

int generic_estimate_system_state(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}

int generic_apply_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint)
{
	return -EOPNOTSUPP;
}

int generic_execute_operation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request)
{
	return -EOPNOTSUPP;
}

int generic_estimate_efficiency(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request)
{
	return -EOPNOTSUPP;
}

int generic_error_backpropagation(struct ml_lib_model *ml_model,
			    struct ml_lib_backpropagation_feedback *feedback,
			    struct ml_lib_user_space_notification *notify)
{
	return -EOPNOTSUPP;
}

int generic_correct_system_state(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Viacheslav Dubeyko <slava@dubeyko.com>");
MODULE_DESCRIPTION("ML libraray");
MODULE_VERSION("1.0");
