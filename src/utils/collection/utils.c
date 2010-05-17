/*
 * utils.c
 *
 *  Created on: 28-mar-2009
 *      Author: filosganga
 */

#include "collections.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static uint32_t hash_int_impl(uint32_t a);

void* clone_item_nop(const void *item) {
	return item;
}

void free_item_nop(void *item) {
	// Empty
}

uint32_t string_hash(const void* item) {
   unsigned long h;
   char* string = (char*)item;

   h = 0;
   while (*string) {
      h = h * 37UL + (unsigned char) *string++;
   }
   return h;
}

int string_cmp(const void* litem, const void *ritem) {
	char* lstring = (char*)litem;
	char* rstring = (char*)ritem;

	return strcmp(lstring, rstring);
}

bool string_eq(const void* litem, const void *ritem) {
	char* lstring = (char*)litem;
	char* rstring = (char*)ritem;

	return strcmp(lstring, rstring)==0;
}


uint32_t ref_hash(const void* ref) {
	uint32_t* integer = (uint32_t*)ref;

	return hash_int_impl(integer);
}

int ref_cmp(const void* litem, const void *ritem) {
	return litem - ritem;
}

bool ref_eq(const void* litem, const void *ritem) {
	return litem == ritem;
}

uint32_t int_hash(const void* item) {
	uint32_t* integer = (uint32_t*)item;

	return hash_int_impl(*integer);
}

int int_cmp(const void* litem, const void *ritem) {
	uint32_t* lint = (uint32_t*)litem;
	uint32_t* rint = (uint32_t*)ritem;

	return *lint - *rint;
}

bool int_eq(const void* litem, const void *ritem) {
	uint32_t* lint = (uint32_t*)litem;
	uint32_t* rint = (uint32_t*)ritem;

	return *lint==*rint;
}

/**
 * Robert Jenkins' 32 bit integer hash function
 */
static uint32_t hash_int_impl(uint32_t a) {
   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   return a;
}
