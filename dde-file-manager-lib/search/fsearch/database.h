/**
 * Copyright (C) 2017 Uniontech Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#pragma once

#include <glib.h>
#include <stdint.h>
#include <stdbool.h>
#include "array.h"
#include "btree.h"

typedef struct _Database Database;

typedef struct _DatabaseLocation DatabaseLocation;

typedef struct _FsearchConfig FsearchConfig;

void
db_location_free (DatabaseLocation *location);

bool
db_location_load (Database *db, const char *location_name);

bool
db_location_add (Database *db,
                 const char *location_name,
                 FsearchConfig *config,
                 void (*callback)(const char *));

bool
db_location_remove (Database *db, const char *path);

bool
db_location_write_to_file (DatabaseLocation *location, const char *fname);

BTreeNode *
db_location_get_entries (DatabaseLocation *location);

void
db_free (Database *db);

Database *
db_new ();

gboolean
db_list_append_node (BTreeNode *node,
                     gpointer data);

void
db_update_sort_index (Database *db);

bool
db_save_locations (Database *db);

void
db_update_entries_list (Database *db);

void
db_build_initial_entries_list (Database *db);

time_t
db_get_timestamp (Database *db);

uint32_t
db_get_num_entries (Database *db);

void
db_unlock (Database *db);

void
db_lock (Database *db);

bool
db_try_lock (Database *db);

DynamicArray *
db_get_entries (Database *db);

void
db_sort (Database *db);

bool
db_clear (Database *db);
