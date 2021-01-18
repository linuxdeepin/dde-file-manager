/**
 * Copyright (C) 2017 Uniontech Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#pragma once

#include <stdbool.h>

typedef struct
{
    char *query;
    bool match_case;
    bool enable_regex;
    bool auto_search_in_path;
    bool search_in_path;

    void (*callback)(void *, void*);
    void *callback_data;
    void *sender;
} FsearchQuery;

FsearchQuery *
fsearch_query_new (const char *query,
                   void (*callback)(void *, void *),
                   void *callback_data,
                   void *sender,
                   bool match_case,
                   bool enable_regex,
                   bool auto_search_in_path,
                   bool search_in_path);

void
fsearch_query_free (FsearchQuery *query);
