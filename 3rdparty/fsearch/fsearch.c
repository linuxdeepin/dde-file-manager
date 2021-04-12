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

//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include "fsearch.h"
#include "fsearch_config.h"
#include "fsearch_limits.h"
//#include "utils.h"
#include "database.h"
#include "database_search.h"
//#include "debug.h"
enum {
    DATABASE_UPDATE,
    DATABASE_UPDATED,
    LAST_SIGNAL
};

//static guint signals [LAST_SIGNAL];

gpointer
load_database (FsearchApplication*app, const char *path, bool *state);

Database *
fsearch_application_get_db (FsearchApplication *fsearch)
{
    return fsearch->db;
}

FsearchThreadPool *
fsearch_application_get_thread_pool (FsearchApplication *fsearch)
{
    return fsearch->pool;
}

FsearchConfig *
fsearch_application_get_config (FsearchApplication *fsearch)
{
    return fsearch->config;
}
static void
build_location_callback (const char *text)
{
    if (text) {
        //g_idle_add (update_db_cb, g_strdup (text));
    }
}

static bool
make_location_dir (void)
{
    gchar config_dir[PATH_MAX] = "";
    config_build_dir (config_dir, sizeof (config_dir));
    gchar location_dir[PATH_MAX] = "";
    g_assert (0 <= snprintf (location_dir, sizeof (location_dir), "%s/%s", config_dir, "database"));
    return !g_mkdir_with_parents (location_dir, 0700);
}

void
fsearch_application_init (FsearchApplication *app)
{
    config_make_dir ();
    make_location_dir ();
    app->config = calloc (1, sizeof (FsearchConfig));
    if (!config_load (app->config)) {
        if (!config_load_default (app->config)) {
        }
    }
    app->db = NULL;
    app->search = NULL;
    g_mutex_init (&app->mutex);
}

static void
fsearch_application_shutdown (FsearchApplication *app)
{
    FsearchApplication *fsearch = app;

    if (fsearch->db) {
        db_save_locations (fsearch->db);
        db_clear (fsearch->db);
    }
    if (fsearch->pool) {
        fsearch_thread_pool_free (fsearch->pool);
    }
    config_save (fsearch->config);
    config_free (fsearch->config);
    g_mutex_clear (&fsearch->mutex);
//    G_APPLICATION_CLASS (fsearch_application_parent_class)->shutdown (app);
}
static void
prepare_windows_for_db_update (FsearchApplication *app)
{
    return;
}

#ifdef DEBUG
static struct timeval tm1;
#endif

static inline void timer_start()
{
#ifdef DEBUG
    gettimeofday(&tm1, NULL);
#endif
}

static inline void timer_stop()
{
#ifdef DEBUG
    struct timeval tm2;
    gettimeofday(&tm2, NULL);

    unsigned long long t = 1000 * (tm2.tv_sec - tm1.tv_sec) + (tm2.tv_usec - tm1.tv_usec) / 1000;
    trace ("%llu ms\n", t);
#endif
}

 gpointer
load_database (FsearchApplication*app, const char *path, bool *state)
{
    timer_start ();
    app->db = db_new ();
    if (db_location_add (app->db, path, app->config, state, build_location_callback)) {
       db_build_initial_entries_list (app->db);
    }
    timer_stop ();

    return NULL;
}

void
fsearch_application_startup (FsearchApplication* app)
{
    (app)->pool = fsearch_thread_pool_init ();
}

