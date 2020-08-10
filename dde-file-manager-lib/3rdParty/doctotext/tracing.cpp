/****************************************************************************
**
** DocToText - Converts DOC, XLS, XLSB, PPT, RTF, ODF (ODT, ODS, ODP),
**             OOXML (DOCX, XLSX, PPTX), iWork (PAGES, NUMBERS, KEYNOTE),
**             ODFXML (FODP, FODS, FODT), PDF, EML and HTML documents to plain text.
**             Extracts metadata and annotations.
**
** Copyright (c) 2006-2013, SILVERCODERS(R)
** http://silvercoders.com
**
** Project homepage: http://silvercoders.com/en/products/doctotext
**
** This program may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING.GPL included in the
** packaging of this file.
**
** Please remember that any attempt to workaround the GNU General Public
** License using wrappers, pipes, client/server protocols, and so on
** is considered as license violation. If your program, published on license
** other than GNU General Public License version 2, calls some part of this
** code directly or indirectly, you have to buy commercial license.
** If you do not like our point of view, simply do not use the product.
**
** Licensees holding valid commercial license for this product
** may use this file in accordance with the license published by
** SILVERCODERS and appearing in the file COPYING.COM
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
*****************************************************************************/

#include "tracing.h"

#include <stdio.h>
#include <stdlib.h>

static int level = 0;
static FILE* file = NULL;

void doctotext_init_tracing(const char* filename) __attribute__((no_instrument_function));
static void close_tracing(void)  __attribute__((no_instrument_function, destructor));
static void trace_func(bool call, void* func, void* caller) __attribute__((no_instrument_function));

void doctotext_init_tracing(const char* filename)
{
	file = fopen(filename, "w");
	if (file == NULL)
	{
		fprintf(stderr, "Error opening %s trace file.\n", filename);
		exit(1);
	}
}

static void close_tracing(void)
{
	if (file != NULL)
		fclose(file);
	file = NULL;
}

static void trace_func(bool call, void* func, void* caller)
{
	if (!call)
		level--;
	if (file != NULL)
	{
		#warning TODO: Make this thread safe and add thread id to output.
		fprintf(file, "%i %c %p %p\n", level, call ? 'C' : 'R', func, caller);
		fflush(file);
	}
	if (call)
		level++;
}

extern "C"
{
	void __cyg_profile_func_enter(void* func,  void* caller) __attribute__((no_instrument_function));
	void __cyg_profile_func_exit (void* func, void* caller) __attribute__((no_instrument_function));

	void __cyg_profile_func_enter(void* func, void* caller)
	{
		trace_func(true, func, caller);
	}

	void __cyg_profile_func_exit (void* func, void* caller)
	{
		trace_func(false, func, caller);
	}
}
