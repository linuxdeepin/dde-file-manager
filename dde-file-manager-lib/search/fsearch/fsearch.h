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
#include <inttypes.h>
//#include <glib.h>
//#include <gtk/gtk.h>
#include "database.h"
#include "fsearch_config.h"
#include "fsearch_thread_pool.h"
//#include "glib_support.h"
#include "database_search.h"

struct _FsearchApplication {
    Database *db;
    DatabaseSearch *search;
    FsearchConfig *config;
    FsearchThreadPool *pool;
    DatabaseSearchResult *result;
    GMutex mutex;
};
typedef struct _FsearchApplication FsearchApplication;


void
fsearch_update_database(FsearchApplication *fsearch);

Database *
fsearch_application_get_db(FsearchApplication *fsearch);

FsearchConfig *
fsearch_application_get_config(FsearchApplication *fsearch);

FsearchThreadPool *
fsearch_application_get_thread_pool(FsearchApplication *fsearch);

void
fsearch_application_activate(FsearchApplication *app);

void
fsearch_application_startup(FsearchApplication *app);

gpointer
load_database(FsearchApplication *app, const char *path, bool *state);

void
fsearch_application_init(FsearchApplication *app);
