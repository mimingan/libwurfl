/*
 * Copyright 2011 ff-dev.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Written by Filippo De Luca <me@filippodeluca.com>.  */

#ifndef CHAIN_H_
#define CHAIN_H_

#include <stdlib.h>
#include <stdint.h>

typedef enum {
	CHN_FINISHED=1,
	CHN_CONTINUE=0
} handler_result_t;

typedef handler_result_t (*handler_execute_f)(void* context, void* data);

typedef struct {
	handler_execute_f execute;
	void* data;
} handler_t;

/** The chain_t type */
typedef struct _chain_t chain_t;

/**
 * This function create a chain using given allocator
 *
 * @param allocator allocator_t used to allocate chain.
 */
chain_t* chain_create();

void chain_destroy(chain_t* chain);

void chain_add_handler(chain_t* chain, handler_t* handler);

void chain_remove_handler(chain_t* chain, handler_t* handler);

int chain_execute(chain_t* chain, void* context);

#endif /* CHAIN_H_ */
