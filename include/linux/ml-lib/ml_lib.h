// SPDX-License-Identifier: GPL-2.0-only
/*
 *  Machine Learning (ML) library
 *
 *  Copyright (C) 2025-2026 Viacheslav Dubeyko <slava@dubeyko.com>
 */

#ifndef _LINUX_ML_LIB_H
#define _LINUX_ML_LIB_H

/*
 * Any kernel subsystem can be in several modes
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
enum ml_lib_system_mode {
	ML_LIB_UNKNOWN_MODE,
	ML_LIB_EMERGENCY_MODE,
	ML_LIB_LEARNING_MODE,
	ML_LIB_COLLABORATION_MODE,
	ML_LIB_RECOMMENDATION_MODE,
	ML_LIB_MODE_MAX
};

struct ml_lib_model;

/*
 * struct ml_lib_model_options - ML model global options
 * @sleep_timeout: main thread's sleep timeout
 *
 * These options define behavior of ML model.
 * The options can be defined during init() or re-init() call.
 */
struct ml_lib_model_options {
	u32 sleep_timeout;
};

/*
 * struct ml_lib_model_run_config - ML model run config
 * @sleep_timeout: main thread's sleep timeout
 *
 * The run config is used for correction of ML model options
 * by means of start/stop methods pair.
 */
struct ml_lib_model_run_config {
	u32 sleep_timeout;
};

/*
 * struct ml_lib_subsystem - kernel subsystem object
 * @type: object type
 * @size: number of bytes in allocated object
 */
struct ml_lib_subsystem {
	atomic_t type;
	size_t size;
};

enum {
	ML_LIB_UNKNOWN_SUBSYSTEM_TYPE,
	ML_LIB_GENERIC_SUBSYSTEM,
	ML_LIB_SPECIALIZED_SUBSYSTEM,
	ML_LIB_SUBSYSTEM_TYPE_MAX
};

/*
 * struct ml_lib_subsystem_state - shared kernel subsystem state
 * @state: object state
 * @size: number of bytes in allocated object
 */
struct ml_lib_subsystem_state {
	atomic_t state;
	size_t size;
};

enum {
	ML_LIB_UNKNOWN_SUBSYSTEM_STATE,
	ML_LIB_SUBSYSTEM_CREATED,
	ML_LIB_SUBSYSTEM_INITIALIZED,
	ML_LIB_SUBSYSTEM_STARTED,
	ML_LIB_SUBSYSTEM_RUNNING,
	ML_LIB_SUBSYSTEM_SHUTTING_DOWN,
	ML_LIB_SUBSYSTEM_STOPPED,
	ML_LIB_SUBSYSTEM_STATE_MAX
};

struct ml_lib_subsystem_state_operations {
	void *(*allocate)(size_t size, gfp_t gfp);
	void (*free)(struct ml_lib_subsystem_state *state);
	int (*init)(struct ml_lib_subsystem_state *state);
	int (*destroy)(struct ml_lib_subsystem_state *state);
	int (*check_state)(struct ml_lib_subsystem_state *state);
	struct ml_lib_subsystem_state *
	    (*snapshot_state)(struct ml_lib_subsystem *object);
	int (*estimate_system_state)(struct ml_lib_model *ml_model);
	int (*correct_system_state)(struct ml_lib_model *ml_model);
};

/*
 * struct ml_lib_dataset - exported subsystem's dataset
 * @type: object type
 * @state: object state
 * @allocated_size: number of bytes in allocated object
 * @portion_offset: portion offset in the data stream
 * @portion_size: extracted portion size
 */
struct ml_lib_dataset {
	atomic_t type;
	atomic_t state;
	size_t allocated_size;

	u64 portion_offset;
	u32 portion_size;
};

enum {
	ML_LIB_UNKNOWN_DATASET_TYPE,
	ML_LIB_EMPTY_DATASET,
	ML_LIB_DATASET_TYPE_MAX
};

enum {
	ML_LIB_UNKNOWN_DATASET_STATE,
	ML_LIB_DATASET_ALLOCATED,
	ML_LIB_DATASET_CLEAN,
	ML_LIB_DATASET_EXTRACTED_PARTIALLY,
	ML_LIB_DATASET_EXTRACTED_COMPLETELY,
	ML_LIB_DATASET_OBSOLETE,
	ML_LIB_DATASET_EXTRACTION_FAILURE,
	ML_LIB_DATASET_CORRUPTED,
	ML_LIB_DATASET_STATE_MAX
};

struct ml_lib_dataset_operations {
	void *(*allocate)(size_t size, gfp_t gfp);
	void (*free)(struct ml_lib_dataset *dataset);
	int (*init)(struct ml_lib_dataset *dataset);
	int (*destroy)(struct ml_lib_dataset *dataset);
	struct ml_lib_dataset *
	    (*extract)(struct ml_lib_model *ml_model);
	int (*preprocess_data)(struct ml_lib_model *ml_model,
				struct ml_lib_dataset *dataset);
	int (*publish_data)(struct ml_lib_model *ml_model,
			    struct ml_lib_dataset *dataset);
};

/*
 * struct ml_lib_request_config - dataset operation configuration
 * @type: object type
 * @state: object state
 * @size: number of bytes in allocated object
 */
struct ml_lib_request_config {
	atomic_t type;
	atomic_t state;
	size_t size;
};

enum {
	ML_LIB_UNKNOWN_REQUEST_CONFIG_TYPE,
	ML_LIB_EMPTY_REQUEST_CONFIG,
	ML_LIB_REQUEST_CONFIG_TYPE_MAX
};

enum {
	ML_LIB_UNKNOWN_REQUEST_CONFIG_STATE,
	ML_LIB_REQUEST_CONFIG_ALLOCATED,
	ML_LIB_REQUEST_CONFIG_INITIALIZED,
	ML_LIB_REQUEST_CONFIG_STATE_MAX
};

struct ml_lib_request_config_operations {
	void *(*allocate)(size_t size, gfp_t gfp);
	void (*free)(struct ml_lib_request_config *config);
	int (*init)(struct ml_lib_request_config *config);
	int (*destroy)(struct ml_lib_request_config *config);
};

struct ml_lib_user_space_request {
};

struct ml_lib_user_space_request_operations {
	int (*operation)(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_request *request);
};

struct ml_lib_user_space_notification {
};

struct ml_lib_user_space_notification_operations {
	int (*operation)(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_notification *notify);
};

struct ml_lib_user_space_recommendation {
};

struct ml_lib_user_space_recommendation_operations {
	int (*operation)(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint);
};

struct ml_lib_backpropagation_feedback {
};

struct ml_lib_backpropagation_operations {
	int (*operation)(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint);
};

/*
 * struct ml_lib_model_operations - ML model operations
 * @create: specialized method of ML model creation
 * @init: specialized method of ML model initialization
 * @re_init: specialized method of ML model re-initialization
 * @start: specialized method of ML model start
 * @stop: specialized method of ML model stop
 * @destroy: specialized method of ML model destroy
 * @get_system_state: specialized method of getting subsystem state
 * @get_dataset: specialized method of getting a dataset
 * @preprocess_data: specialized method of data preprocessing
 * @publish_data: specialized method of sharing data with user-space
 * @preprocess_recommendation: specialized method of preprocess recomendations
 * @estimate_system_state: specialized method of system state estimation
 * @apply_recommendation: specialized method of recommendations applying
 * @execute_operation: specialized method of operation execution
 * @estimate_efficiency: specialized method of operation efficiency estimation
 * @error_backpropagation: specialized method of error backpropagation
 * @correct_system_state: specialized method of subsystem state correction
 */
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
		(*get_dataset)(struct ml_lib_model *ml_model,
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
	int (*execute_operation)(struct ml_lib_model *ml_model,
			    struct ml_lib_user_space_recommendation *hint,
			    struct ml_lib_user_space_request *request);
	int (*estimate_efficiency)(struct ml_lib_model *ml_model,
			    struct ml_lib_user_space_recommendation *hint,
			    struct ml_lib_user_space_request *request);
	int (*error_backpropagation)(struct ml_lib_model *ml_model,
			    struct ml_lib_backpropagation_feedback *feedback,
			    struct ml_lib_user_space_notification *notify);
	int (*correct_system_state)(struct ml_lib_model *ml_model);
};

/*
 * struct ml_lib_model - ML model declaration
 * @mode: ML model mode (enum ml_lib_system_mode)
 * @parent: parent kernel subsystem
 * @parent_state: parent kernel subsystem's state
 * @options: ML model options
 * @model_ops: ML model specialized operations
 * @system_state_ops: subsystem state specialized operations
 * @dataset_ops: dataset specialized operations
 * @request_config_ops: specialized dataset configuration operations
 */
struct ml_lib_model {
	atomic_t mode;

	struct ml_lib_subsystem *parent;
	struct ml_lib_subsystem_state * __rcu parent_state;
	struct ml_lib_model_options * __rcu options;

	struct ml_lib_model_operations *model_ops;
	struct ml_lib_subsystem_state_operations *system_state_ops;
	struct ml_lib_dataset_operations *dataset_ops;
	struct ml_lib_request_config_operations *request_config_ops;
};

struct ml_lib_model *allocate_ml_model(size_t size, gfp_t gfp);
void free_ml_model(struct ml_lib_model *ml_model);
struct ml_lib_model_options *allocate_ml_model_options(size_t size, gfp_t gfp);
void free_ml_model_options(struct ml_lib_model_options *options);
struct ml_lib_subsystem *allocate_subsystem_object(size_t size, gfp_t gfp);
void free_subsystem_object(struct ml_lib_subsystem *object);
struct ml_lib_subsystem_state *allocate_subsystem_state(size_t size, gfp_t gfp);
void free_subsystem_state(struct ml_lib_subsystem_state *state);
struct ml_lib_dataset *allocate_dataset(size_t size, gfp_t gfp);
void free_dataset(struct ml_lib_dataset *dataset);
struct ml_lib_request_config *allocate_request_config(size_t size, gfp_t gfp);
void free_request_config(struct ml_lib_request_config *config);

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
generic_get_dataset(struct ml_lib_model *ml_model,
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
int generic_execute_operation(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request);
int generic_estimate_efficiency(struct ml_lib_model *ml_model,
			 struct ml_lib_user_space_recommendation *hint,
			 struct ml_lib_user_space_request *request);
int generic_error_backpropagation(struct ml_lib_model *ml_model,
			    struct ml_lib_backpropagation_feedback *feedback,
			    struct ml_lib_user_space_notification *notify);
int generic_correct_system_state(struct ml_lib_model *ml_model);

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

#endif /* _LINUX_ML_LIB_H */
