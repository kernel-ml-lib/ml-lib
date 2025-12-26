// SPDX-License-Identifier: GPL-2.0-only
/*
 *  Machine Learning (ML) library
 *
 *  Copyright (C) 2025-2026 Viacheslav Dubeyko <slava@dubeyko.com>
 */

#ifndef _LINUX_ML_LIB_H
#define _LINUX_ML_LIB_H

/*
 * Any kernel subsystem can be in several states
 * that define how this subsystem interacts with
 * ML model in user-space:
 * (1) EMERGENCY_MODE - ignore ML model and run default algorithm(s).
 * (2) LEARNING_MODE - ML model is learning and any recommendations
 *                     requires of checking and errors back-propagation.
 * (3) COLLABORATION_MODE - ML model has good prediction but still
 *                          requires correction by default algorithm(s).
 * (4) RECOMMENDATION_MODE - ML model is capable to substitute
 *                           the default algorithm(s).
 */
enum ml_lib_system_state {
	ML_LIB_UNKNOWN_MODE,
	ML_LIB_EMERGENCY_MODE,
	ML_LIB_LEARNING_MODE,
	ML_LIB_COLLABORATION_MODE,
	ML_LIB_RECOMMENDATION_MODE,
	ML_LIB_MODE_MAX
};

struct ml_lib_model;
struct ml_lib_model_options;
struct ml_lib_model_run_config;
struct ml_lib_subsystem;
struct ml_lib_subsystem_state;
struct ml_lib_dataset;
struct ml_lib_request_config;
struct ml_lib_user_space_request;
struct ml_lib_user_space_notification;
struct ml_lib_user_space_recommendation;

struct ml_lib_user_space_request_operations {
	int (*operation)(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_request *request);
};

struct ml_lib_user_space_notification_operations {
	int (*operation)(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_notification *notify);
};

struct ml_lib_user_space_recommendation_operations {
	int (*operation)(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint);
};

struct ml_lib_model_operations {
	int (*create)(struct ml_lib_model *ml_model);
	int (*init)(struct ml_lib_model *ml_model,
		    struct ml_lib_model_options *options);
	int (*re_init)(struct ml_lib_model *ml_model,
			struct ml_lib_model_options *options);
	int (*start)(struct ml_lib_model *ml_model,
		     struct ml_lib_model_run_config *config);
	int (*stop)(struct ml_lib_model *ml_model);
	void (*destroy)(struct ml_lib_model *ml_model);
	struct ml_lib_subsystem_state *
		(*get_system_state)(struct ml_lib_model *ml_model);
	struct ml_lib_dataset *
		(*get_data)(struct ml_lib_model *ml_model,
			    struct ml_lib_request_config *config,
			    struct ml_lib_user_space_request *request);
	int (*preprocess_data)(struct ml_lib_model *ml_model,
				struct ml_lib_dataset *dataset);
	int (*publish_data)(struct ml_lib_model *ml_model,
			    struct ml_lib_dataset *dataset,
			    struct ml_lib_user_space_notification *notify);
	int (*preprocess_recommendation)(struct ml_lib_model *ml_model,
			    struct ml_lib_user_space_recommendation *hint);
	int (*estimate_system_state)(struct ml_lib_model *ml_model);
	int (*apply_recommendation)(struct ml_lib_model *ml_model,
			    struct ml_lib_user_space_recommendation *hint);
	execute_operation();
	estimate_effieciency();
	execute_error_backpropagation();
	correct_system_state();
};

/*
 * struct ml_lib_model - ML model declaration
 * @state: ML model state (enum ml_lib_system_state)
 * @parent: parent kernel subsystem
 * @parent_state: parent kernel subsystem's state
 * @ops: ML model specialized operations
 */
struct ml_lib_model {
	atomic_t state;
	struct ml_lib_subsystem *parent;
	struct ml_lib_subsystem_state * __rcu parent_state;

	struct ml_lib_model_operations *ops;
};

struct ml_lib_model *allocate_ml_model(size_t size, gfp_t gfp);
void free_ml_model(struct ml_lib_model *ml_model);

int generic_create_ml_model(struct ml_lib_model *ml_model);
int generic_init_ml_model(struct ml_lib_model *ml_model,
			  struct ml_lib_model_options *options);
int generic_re_init_ml_model(struct ml_lib_model *ml_model,
			     struct ml_lib_model_options *options);
int generic_start_ml_model(struct ml_lib_model *ml_model,
			   struct ml_lib_model_run_config *config);
int generic_stop_ml_model(struct ml_lib_model *ml_model);
void generic_destroy_ml_model(struct ml_lib_model *ml_model);
struct ml_lib_subsystem_state *
generic_get_system_state(struct ml_lib_model *ml_model);
struct ml_lib_dataset *
generic_get_data(struct ml_lib_model *ml_model,
		 struct ml_lib_request_config *config,
		 struct ml_lib_user_space_request *request);
int generic_preprocess_data(struct ml_lib_model *ml_model,
			    struct ml_lib_dataset *dataset);
int generic_publish_data(struct ml_lib_model *ml_model,
			 struct ml_lib_dataset *dataset,
			 struct ml_lib_user_space_notification *notify);
int generic_preprocess_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint);
int generic_estimate_system_state(struct ml_lib_model *ml_model);
int generic_apply_recommendation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint);

#endif /* _LINUX_ML_LIB_H */
