/*
   FSearch - A fast file search utility
   Copyright © 2020 Christian Boxdörfer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
   */

#pragma once

#include <glib.h>
#include <stdint.h>
#include <stdbool.h>
#include "array.h"
#include "btree.h"

typedef struct _DatabaseConfig
{
    bool enable_py;
    bool filter_hidden_file;
} DatabaseConfig;

struct _Database
{
    GList *locations;
    GList *searches;
    DynamicArray *entries;
    uint32_t num_entries;
    DatabaseConfig *db_config;

    time_t timestamp;

    GMutex mutex;
};
typedef struct _Database Database;

typedef struct _DatabaseLocation DatabaseLocation;

typedef struct _FsearchConfig FsearchConfig;

void db_location_free(DatabaseLocation *location);

bool db_location_load(Database *db, const char *location_name);

bool db_location_add(Database *db,
                     const char *location_name,
                     bool *is_stop,
                     void (*callback)(const char *));

bool db_location_remove(Database *db, const char *path);

bool db_location_write_to_file(DatabaseLocation *location, const char *fname);

BTreeNode *
db_location_get_entries(DatabaseLocation *location);

void db_free(Database *db);

Database *
db_new();

Database *
db_copy(const Database *db);

gboolean
db_list_append_node(BTreeNode *node,
                    gpointer data);

void db_update_sort_index(Database *db);

bool db_save_locations(Database *db, const char *save_path);

void db_update_entries_list(Database *db);

void db_build_initial_entries_list(Database *db);

time_t
db_get_timestamp(Database *db);

uint32_t
db_get_num_entries(Database *db);

void db_unlock(Database *db);

void db_lock(Database *db);

bool db_try_lock(Database *db);

DynamicArray *
db_get_entries(Database *db);

void db_sort(Database *db);

bool db_clear(Database *db);

bool db_support(const char *search_path, bool has_data_prefix);
