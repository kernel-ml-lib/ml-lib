// SPDX-License-Identifier: GPL-2.0-only
/*
 * Machine Learning (ML) library
 *
 * Copyright (C) 2025-2026 Viacheslav Dubeyko <slava@dubeyko.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/ml-lib/ml_lib.h>

/******************************************************************************
 *                             ML library API                                 *
 ******************************************************************************/

struct ml_lib_model *allocate_ml_model(size_t size, gfp_t gfp)
{
	return NULL;
}

void free_ml_model(struct ml_lib_model *ml_model)
{
}

struct ml_lib_subsystem *allocate_subsystem_object(size_t size, gfp_t gfp)
{
	return NULL;
}

void free_subsystem_object(struct ml_lib_subsystem *object)
{
}

struct ml_lib_subsystem_state *allocate_subsystem_state(size_t size, gfp_t gfp)
{
	return NULL;
}

void free_subsystem_state(struct ml_lib_subsystem_state *state)
{
}

struct ml_lib_dataset *allocate_dataset(size_t size, gfp_t gfp)
{
	return NULL;
}

void free_dataset(struct ml_lib_dataset *dataset)
{
}

struct ml_lib_request_config *allocate_request_config(size_t size, gfp_t gfp)
{
	return NULL;
}

void free_request_config(struct ml_lib_request_config *config)
{
}

int ml_model_create(struct ml_lib_model *ml_model,
		    const char *subsystem_name,
		    const char *model_name)
{
	return -EOPNOTSUPP;
}

int ml_model_init(struct ml_lib_model *ml_model,
		  struct ml_lib_model_options *options)
{
	return -EOPNOTSUPP;
}

int ml_model_re_init(struct ml_lib_model *ml_model,
		     struct ml_lib_model_options *options)
{
	return -EOPNOTSUPP;
}

int ml_model_start(struct ml_lib_model *ml_model,
		   struct ml_lib_model_run_config *config)
{
	return -EOPNOTSUPP;
}

int ml_model_stop(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}

void ml_model_destroy(struct ml_lib_model *ml_model)
{
}

struct ml_lib_subsystem_state *get_system_state(struct ml_lib_model *ml_model)
{
	return NULL;
}

struct ml_lib_dataset *get_dataset(struct ml_lib_model *ml_model,
				   struct ml_lib_request_config *config,
				   struct ml_lib_user_space_request *request)
{
	return NULL;
}

int ml_model_preprocess_data(struct ml_lib_model *ml_model,
			     struct ml_lib_dataset *dataset)
{
	return -EOPNOTSUPP;
}

int ml_model_publish_data(struct ml_lib_model *ml_model,
			  struct ml_lib_dataset *dataset,
			  struct ml_lib_user_space_notification *notify)
{
	return -EOPNOTSUPP;
}

int ml_model_preprocess_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint)
{
	return -EOPNOTSUPP;
}

int estimate_system_state(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}

int apply_ml_model_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint)
{
	return -EOPNOTSUPP;
}

int execute_ml_model_operation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request)
{
	return -EOPNOTSUPP;
}

int estimate_ml_model_efficiency(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request)
{
	return -EOPNOTSUPP;
}

int ml_model_error_backpropagation(struct ml_lib_model *ml_model,
			    struct ml_lib_backpropagation_feedback *feedback,
			    struct ml_lib_user_space_notification *notify)
{
	return -EOPNOTSUPP;
}

int correct_system_state(struct ml_lib_model *ml_model)
{
	return -EOPNOTSUPP;
}

/******************************************************************************
 *              Generic implementation of ML model's methods                  *
 ******************************************************************************/

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
MODULE_DESCRIPTION("ML library");
MODULE_VERSION("1.0");
