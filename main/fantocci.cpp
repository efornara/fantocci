// fantocci.cpp
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

#include "fantocci.h"

fantocci_register_types_fn *fantocci_register_types_array = 0;
fantocci_unregister_types_fn *fantocci_unregister_types_array = 0;

void register_fantocci_types() {
	if (!fantocci_register_types_array)
		return;
	for (int i = 0; fantocci_register_types_array[i]; i++) {
		fantocci_register_types_fn f = fantocci_register_types_array[i];
		f();
	}
}

void unregister_fantocci_types() {
	if (!fantocci_unregister_types_array)
		return;
	for (int i = 0; fantocci_unregister_types_array[i]; i++) {
		fantocci_unregister_types_fn f = fantocci_unregister_types_array[i];
		f();
	}
}

#ifdef TOOLS_ENABLED
const char *fantocci_types_prefix = 0;
#endif
