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

#include "device-impl.h"

#include "devicedef.h"
#include "utils/error.h"
#include "utils/utils.h"
#include "utils/functors.h"
#include "utils/hashmap.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>

extern int errno;

static bool capability_toarray(void* item, void* xtra);

static hashmap_t* explode_capabilities(const devicedef_t* devicedef, hashmap_t* devices);

device_t* device_init(hashmap_t* devices, const devicedef_t* devicedef) {

	device_t* device = malloc(sizeof(device_t));
	if(device==NULL) {
		error(1, errno, "error allocating device");
	}

	// TODO copy values?
	device->id = devicedef->id;
	device->user_agent = devicedef->user_agent;
	device->capabilities = explode_capabilities(devicedef, devices);

	return device;
}

void device_release(device_t* device) {
	hashmap_free(device->capabilities, NULL, NULL);
	free(device);
}

char* device_capability(const device_t* device, const char* name) {

	return (char*)hashmap_get(device->capabilities, name);
}

char** device_capabilities(const device_t* device, void* (dupe)(size_t size)) {

	if(dupe==NULL) {
		dupe = &malloc;
	}

	size_t size = hashmap_size(device->capabilities);

	functor_toarray_data_t toarray_data;
	toarray_data.index = 0;
	toarray_data.size = (size * 2) + 1;
	toarray_data.array = dupe(sizeof(char*) * toarray_data.size);
	if(!toarray_data.array) {
		error(1, errno, "error allocating array for capabilities");
	}
	memset(toarray_data.array, NULL, toarray_data.size);

	hashmap_foreach(device->capabilities, &capability_toarray, &toarray_data);

	return (char**)toarray_data.array;
}

char* device_id(const device_t* device) {

	return (char*)device->id;
}

char* device_user_agent(const device_t* device) {

	return (char*)device->user_agent;
}

int device_cmp(const void* left, const void* right) {

	device_t* ldevice = (device_t*)left;
	device_t* rdevice = (device_t*)right;

	return strcmp(ldevice->id, rdevice->id);
}

uint32_t device_hash(const void* item) {

	device_t* device = (device_t*)item;

	return string_hash(device->id);
}

bool device_eq(const void* left, const void* right) {

	device_t* ldevice = (device_t*)left;
	device_t* rdevice = (device_t*)right;

	return string_eq(ldevice->id, rdevice->id);
}

static bool capability_toarray(void* item, void* xtra) {
	functor_toarray_data_t* data = (functor_toarray_data_t*)xtra;
	char** kv = (char**)item;

	char* name = kv[0];
	char* value = kv[1];

	data->array[data->index] = name;
	data->array[data->index + 1] = value;
	data->index+=2;

	return false;
}

static hashmap_t* explode_capabilities(const devicedef_t* devicedef, hashmap_t* devices) {

	hashmap_t* parent_capabilities;
	if(devicedef->fall_back!=NULL) {
		parent_capabilities = explode_capabilities(hashmap_get(devices, devicedef->fall_back), devices);
	}
	else {
		parent_capabilities = hashmap_init(&string_eq, &string_hash, NULL);
	}

	hashmap_putall(parent_capabilities, devicedef->capabilities);
	return parent_capabilities;
}

