// register_types.cpp
/*
 * Fantocci
 * Copyright (c) 2019  Emanuele Fornara
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "register_types.h"

#ifdef PRIVATE_BUILTIN

#include "main/fantocci.h"

// #include "?.h"

static fantocci_register_types_fn register_types_array[] = {
	// register_?_types,
	0
};

static fantocci_unregister_types_fn unregister_types_array[] = {
	// unregister_?_types,
	0
};

// guaranteed to be called before register_fantocci_types()
void register_private_types() {
	fantocci_register_types_array = register_types_array;
	fantocci_unregister_types_array = unregister_types_array;
}

void unregister_private_types() {
}

#else /* !PRIVATE_BUILTIN */

void register_private_types() {
}

void unregister_private_types() {
}

#endif
