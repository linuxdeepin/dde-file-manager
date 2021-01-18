/**
 * Copyright (C) 2017 Uniontech Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
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
load_database (FsearchApplication*app, bool *state);

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

static gint
fsearch_options_handler(FsearchApplication *gapp,
                            GVariantDict *options,
                            gpointer data )
{
    gboolean version = FALSE, updatedb = FALSE;
    g_variant_dict_lookup(options, "version", "b", &version);
    g_variant_dict_lookup(options, "updatedb", "b", &updatedb);

    if (version) {
    }

    if (updatedb) {
        g_printf ("Updating database...");
        fflush(stdout);
        gapp->config->update_database_on_launch = true;

        load_database(gapp, true);
        if (gapp->db) {
            db_save_locations (gapp->db);
            db_clear (gapp->db);
        }
    }

    return (version || updatedb) ? 0 : -1;
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
load_database (FsearchApplication*app, bool *state)
{
    if (!app->db) {
        // create new database
        timer_start ();
        app->db = db_new ();

        bool loaded = false;
        bool build_new = false;
        for (GList *l = app->config->locations; l != NULL; l = l->next) {
            char *data = strdup(l->data);
            if (app->config->update_database_on_launch) {
                if (db_location_add (app->db, data, app->config, state, build_location_callback)) {
                    loaded = true;
                    build_new = true;
                }
            }
            else {
                if (!db_location_load (app->db, data)) {
                    if (db_location_add (app->db, data, app->config, state, build_location_callback)) {
                        loaded = true;
                        build_new = true;
                    }
                }
                else {
                    loaded = true;
                }
            }

            if (data){
                free(data);
                data = NULL;
            }
        }
        if (loaded) {
            if (build_new) {
                db_build_initial_entries_list (app->db);
            }
            else {
                db_update_entries_list (app->db);
            }
        }
        timer_stop ();
    }
    else {
        timer_start ();
        if (app->config->locations) {
            for (GList *l = app->config->locations; l != NULL; l = l->next) {
                char *data = strdup(l->data);
                db_location_add (app->db, data, app->config, state, build_location_callback);

                if (data){
                    free(data);
                    data = NULL;
                }
            }
            db_build_initial_entries_list (app->db);
        }
        timer_stop ();
    }
    return NULL;
}

GThread *
load_database_thread (FsearchApplication *app)
{
    return g_thread_new("fsearch_db_load_thread", load_database, app);
}
void
fsearch_update_database (FsearchApplication *app)
{
    prepare_windows_for_db_update (app);
    g_thread_new("fsearch_db_update_thread", load_database, app);
    return;
}

void
fsearch_application_startup (FsearchApplication* app)
{
    (app)->pool = fsearch_thread_pool_init ();
}

void
fsearch_application_activate (FsearchApplication *app)
{
    load_database_thread (app);
}
