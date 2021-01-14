/*	Copyright 2012 Christoph Gärtner
	Distributed under the Boost Software License, Version 1.0
*/

#ifndef DECODE_HTML_ENTITIES_UTF8_
#define DECODE_HTML_ENTITIES_UTF8_

#include <stddef.h>
#include <string>

extern size_t decode_html_entities_utf8(char* dest, const char* src);
/*
	Takes input from <src> and decodes into <dest>
	The function returns the length of the decoded string.
*/

#endif
