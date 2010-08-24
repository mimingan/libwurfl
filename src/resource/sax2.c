/*
 * handler.c
 *
 *  Created on: 23-mar-2009
 *      Author: filosganga
 */
#include "resource-impl.h"

#include "commons.h"
#include "repository/repository.h"
#include "repository/devicedef-impl.h"
#include "repository/hierarchy-impl.h"

#include "utils/collection/collections.h"
#include "utils/collection/hashmap.h"
#include "utils/collection/hashset.h"
#include "utils/collection/hashtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <libxml/encoding.h>
#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/SAX2.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>

#define ELEM_CAPABILITY BAD_CAST("capability")
#define ELEM_GROUP BAD_CAST("group")
#define ELEM_DEVICE BAD_CAST("device")

#define ATTR_ID BAD_CAST("id")
#define ATTR_NAME BAD_CAST("name")
#define ATTR_VALUE BAD_CAST("value")
#define ATTR_USER_AGENT BAD_CAST("user_agent")
#define ATTR_FALL_BACK BAD_CAST("fall_back")
#define ATTR_ACTUAL_DEVICE_ROOT BAD_CAST("actual_device_root")

extern allocator_t* main_allocator;

typedef struct {
	uint32_t size;

	bool current_device_actual_device_root;
	char* current_device_id;
	char* current_device_user_agent;
	char* current_device_fall_back;

	hashmap_t* devices;
	hashmap_t* capabilities;

	hashset_t* strings;
} parse_context_t;


static char* get_attribute(parse_context_t* context, int nb_attributes,	const xmlChar** attributes, xmlChar* name) {

	char* value = NULL;

	int index=0;
	while(index<(nb_attributes*5) && !xmlStrEqual(name, attributes[index])) index+=5;

	if(index<(nb_attributes*5)) {
        const xmlChar *localname = attributes[index];
        const xmlChar *prefix = attributes[index+1];
        const xmlChar *nsURI = attributes[index+2];
        const xmlChar *valueBegin = attributes[index+3];
        const xmlChar *valueEnd = attributes[index+4];

        int value_size = valueEnd - valueBegin;

        if(value_size>0) {
        	value = (char*)xmlStrndup(valueBegin, value_size);
        }
	}

	return value;
}


static void start_document(void *ctx){

	parse_context_t* context = (parse_context_t*) ctx;
	context->size = 0;
	context->capabilities=NULL;
	context->current_device_actual_device_root = false;
	context->current_device_fall_back = NULL;
	context->current_device_id = NULL;
	context->current_device_user_agent = NULL;
}

static void end_document(void *ctx){

	parse_context_t* context = (parse_context_t*) ctx;
	context->size = 0;
}

static void start_capability(parse_context_t* context, int nb_attributes, const xmlChar** attributes) {

	char* name = get_attribute(context, nb_attributes, attributes, ATTR_NAME);
	char* value = get_attribute(context, nb_attributes, attributes, ATTR_VALUE);

	// FIXME check notnull

	hashmap_put(context->capabilities, name, value);
}

static void end_capability(parse_context_t* context) {

}

static void start_device(parse_context_t* context, int nb_attributes, const xmlChar** attributes) {

	context->current_device_id = get_attribute(context, nb_attributes, attributes, ATTR_ID);
	context->current_device_user_agent = get_attribute(context, nb_attributes, attributes, ATTR_USER_AGENT);
	context->current_device_fall_back = get_attribute(context, nb_attributes, attributes, ATTR_FALL_BACK);

	char* att_root = get_attribute(context, nb_attributes, attributes, ATTR_ACTUAL_DEVICE_ROOT);
	context->current_device_actual_device_root = att_root!=NULL && strcmp(att_root, "true") == 0;

	context->capabilities = hashmap_create(&string_hash, &string_hash, &string_cmp);
}

static void end_device(parse_context_t* context) {

	devicedef_t* devicedef = devicedef_create(context->current_device_id,
			context->current_device_user_agent,
			context->current_device_fall_back,
			context->current_device_actual_device_root,
			context->capabilities);

	hashmap_put(context->devices, devicedef_get_id(devicedef), devicedef);

	// reset context
	context->current_device_id = NULL;
	context->current_device_user_agent = NULL;
	context->current_device_fall_back = NULL;
	context->current_device_actual_device_root = false;
	context->capabilities = NULL;

	context->size++;
}

static void startElementNs (void *ctx,
					const xmlChar *localname,
					const xmlChar *prefix,
					const xmlChar *URI,
					int nb_namespaces,
					const xmlChar **namespaces,
					int nb_attributes,
					int nb_defaulted,
					const xmlChar **attributes) {

	parse_context_t* context = (parse_context_t*) ctx;

	if (xmlStrEqual(localname,ELEM_DEVICE)) {
		start_device(context, nb_attributes, attributes);
	} else if (xmlStrEqual(localname, ELEM_CAPABILITY)) {
		start_capability(context, nb_attributes, attributes);
	} else {
		// ignore other
	}
}

void endElementNs(void *ctx,
					const xmlChar *localname,
					const xmlChar *prefix,
					const xmlChar *URI) {

	parse_context_t* context = (parse_context_t*) ctx;

	if (xmlStrEqual(localname, ELEM_DEVICE)) {
		end_device(context);
	} else if (xmlStrEqual(localname, ELEM_CAPABILITY)) {
		end_capability(context);
	} else {
		// ignore other
	}

}

static void attributef(void *ctx,
				const xmlChar *elem,
				const xmlChar *fullname,
				int type,
				int def,
				const xmlChar *defaultValue,
				xmlEnumerationPtr tree) {
	// Empty
}

static void error(void *ctx, char* msg, ...) {
	printf("%s", msg);
}

static void warn(void *ctx, char* msg, ...) {
	printf("%s",msg);
}

static void fatal(void *ctx, char* msg, ...) {
	printf("%s",msg);
}

static void nop(){
	// Empty
}

resource_data_t* resource_parse(resource_t* resource) {

	xmlSAXHandler saxHandler;
	memset(&saxHandler, 0, sizeof(saxHandler));

	xmlSAXVersion(&saxHandler, 2);
	saxHandler.startDocument = &start_document;
	saxHandler.endDocument = &nop;

	saxHandler.startElementNs = &startElementNs;
	saxHandler.endElementNs = &endElementNs;

	saxHandler.characters = &nop;
	saxHandler.cdataBlock = &nop;
	saxHandler.comment = &nop;
	saxHandler.error = &error;
	saxHandler.fatalError = &fatal;
	saxHandler.warning = &warn;

	// Creating context
	parse_context_t context;
	memset(&context, 0, sizeof(context));

	hashtable_options_t strings_ops;
	hashtable_init_options(&strings_ops);
	strings_ops.eq_fn = &string_eq;
	strings_ops.hash_fn = &string_hash;
	context.strings = hashtable_create(strings_ops, main_allocator);

	context.devices = hashmap_create(&string_hash, &string_eq, main_allocator);

	int sax_error = xmlSAXUserParseFile(&saxHandler, &context, resource->path);

	if(context.size != hashmap_size(context.devices)) {
		error(-1, 0, "Error parsing file %s: Bad number of devices", resource->path);
	}

	hashtable_destroy(context.strings, NULL);

	resource_data_t* resource_data = malloc(sizeof(resource_data_t));
	resource_data->devices = devicedefs_create_frommap(context.devices);

	xmlCleanupParser();

	return resource_data;
}