// fantocci.h
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

// internal functions

void register_fantocci_types();
void unregister_fantocci_types();

// user types

typedef void (*fantocci_register_types_fn)();
typedef void (*fantocci_unregister_types_fn)();

extern fantocci_register_types_fn *fantocci_register_types_array;
extern fantocci_unregister_types_fn *fantocci_unregister_types_array;

// implemented by platform

extern int fantocci_main(int argc, char *argv[]);

// editor

#ifdef TOOLS_ENABLED
extern const char *fantocci_types_prefix;
#endif
