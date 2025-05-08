/*
Copyright (c) <2025>, <Ankush Patel> <ankushjp4@gmail.com>
The Source Code is released under a (3-Clause)BSD-Style Licence.
All rights reserved. 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of the organization nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL The COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ARRAY_H
#define ARRAY_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_EXPAND(x) x
#define ARRAY_ARGFUNC(_1, _2, _3, _4, NAME, ...) NAME

 struct arrfat { _Alignas(max_align_t)
	unsigned cap;
	unsigned len;
	size_t dsize; /* sizeof (data) */
};

static void *
arrmake(size_t dsize, unsigned cap, unsigned len, const void* data)
{
	struct arrfat* af;

	assert(dsize && cap && cap >= len);

	af = malloc(sizeof(struct arrfat) + cap * dsize);
	assert(af);

	af->cap = cap;
	af->len = len;
	af->dsize = dsize;

	if (data)
		memmove(af + 1, data, len * dsize);
	else
		memset(af + 1, 0, len);

	return (af + 1);
}

static void
arrfree(void* arr)
{
	struct arrfat *af;

	assert(arr);
	af = (struct arrfat *)arr - 1;

	free(af);
}

static unsigned
arrcap(void* arr)
{
	struct arrfat *af;

	assert(arr);
	af = (struct arrfat *)arr - 1;

	return (af->cap);
}

static unsigned
arrlen(void *arr)
{
	struct arrfat *af;

	assert(arr);
	af = (struct arrfat *)arr - 1;

	return (af->len);
}

/* if arr has been manually written to, set the number of elements */
static void
arrsetlen(void *arr, unsigned n)
{
	struct arrfat *af;

	assert(arr);
	af = (struct arrfat *)arr - 1;

	assert(n <= af->cap);
	af->len = n;
}

static void *
arrresize(struct arrfat *af, unsigned cap)
{
	struct arrfat* newaf;

	assert(af);

	af->cap = cap;
	if (af->cap < af->len) /* array is being shortened */
		af->len = af->cap;

	newaf = realloc(af, sizeof(struct arrfat) + af->cap * af->dsize);
	assert(newaf);
	af = newaf;

	return (af + 1);
}

/* arr must be a pointer-to-pointer (void **)! */
static void
arrins(void *_arr, unsigned idx, const void *val)
{
	struct arrfat* af;
	void **arr;

	assert(_arr);
	arr = _arr;

	af = (struct arrfat *)*arr - 1;
	if (af->len == af->cap) {
		*arr = arrresize(af, af->cap * 2);
		af = (struct arrfat *)*arr - 1;
	}
	memmove((char *)*arr + (idx + 1) * af->dsize, (char *)*arr + idx * af->dsize, af->dsize * (af->len - idx));
	memcpy((char *)*arr + af->dsize * idx, val, af->dsize);
	af->len++;
}

/* arr must be a pointer-to-pointer (void **)! arr and src must not overlap! len in bytes, not elements! */
static void
arrcpy(void *_arr, void *src, unsigned len)
{
	struct arrfat* af;
	void **arr;

	assert(_arr);
	arr = _arr;

	af = (struct arrfat *)*arr - 1;
	if (len > af->cap) {
		*arr = arrresize(af, len);
		af = (struct arrfat *)*arr - 1;
	}

	memmove(*arr, src, len);
	af->len = len / af->dsize;
}

/* arr must be a pointer-to-pointer (void **)! */
static void
arrrm(void *arr, unsigned idx)
{
	struct arrfat *af;

	assert(arr);
	af = (struct arrfat *)arr - 1;

	assert(idx < af->len);
	af->len--;
	memmove((char *)arr + idx * af->dsize, (char *)arr + (idx + 1) * af->dsize, af->dsize * (af->len - idx));
}

/* arr must be a pointer-to-pointer (void **)! */
static void
arrpush(void *arr, const void *val)
{
	arrins(arr, ((struct arrfat *)*(void **)arr - 1)->len, val);
}

static void
arrpop(void* arr, void* val)
{
	struct arrfat *af;

	assert(arr);
	af = (struct arrfat *)arr - 1;

	memcpy(val, (char *)arr + (af->len - 1) * af->dsize, af->dsize);
	arrrm(arr, af->len - 1);
}

#ifndef ARRAY_NSHORTNAME
#	define ARRAY_MAKE4(dsize, cap, len, data) arrmake(dsize, cap, len, data)
#	define ARRAY_MAKE3(dsize, cap, len) arrmake(dsize, cap, len, 0)
#	define ARRAY_MAKE2(dsize, cap) arrmake(dsize, cap, 0, 0)
#	define ARRAY_MAKE1(dsize) arrmake(dsize, 1, 0, 0)

#	define amake(dsize, ...) ARRAY_EXPAND(ARRAY_ARGFUNC(dsize, __VA_ARGS__, ARRAY_MAKE4, ARRAY_MAKE3, ARRAY_MAKE2, ARRAY_MAKE1)(dsize, __VA_ARGS__))
#	define afree(arr) arrfree(arr)
#	define acap(arr) arrcap(arr)
#	define alen(arr) arrlen(arr)
#	define asetlen(arr, n) arrsetlen(arr, n)
#	define ains(arr, idx, val) arrins(arr, idx, val)
#	define acpy(arr, src, len) arrcpy(arr, src, len)
#	define arm(arr, idx) arrrm(arr, idx)
#	define apush(arr, val) arrpush(arr, val)
#	define apop(arr, val) arrpop(arr, val)
#endif /* ARRAY_NSHORTNAME */

#endif /* ARRAY_H */
