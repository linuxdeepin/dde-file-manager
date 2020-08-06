/*
* Copyright (C) 2019 ~ 2020 Uniontech Technology Co., Ltd.
*
* Author:     hujianzhong <hujianhzong@uniontech.com>
*
* Maintainer: hujianzhong <hujianhzong@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* This program is the full text search at dde-file-manager.
*/
//#include <gtk/gtk.h>
//#include <glib/gi18n.h>
#include <locale.h>
#include "fsearch_config.h"
#include "fsearch.h"
#include <stdlib.h>
#include <string.h>
void fsearch_Close(void);
typedef void (*callbackFunc)(void *);

callbackFunc callBackFunc = NULL;
FsearchApplication* app = NULL;
static GPtrArray *results;
static unsigned int  num_results = 0;
static char keyworld[256]={0};

void fsearch_Init(const char*path)
{
    app=(FsearchApplication *) calloc(1,sizeof(FsearchApplication));
    app->config = (FsearchConfig *)calloc (1, sizeof (FsearchConfig));
    if (!config_load_default (app->config)) {
    }
    app->db = NULL;
    app->search = NULL;
    app->config->locations=NULL;
    g_mutex_init (&app->mutex);
    /*fix task 30348 针对搜索不能搜索部分目录，可以将根目录加入索引库，搜索结果出来以后进行当前目录过滤就可以*/
    app->config->locations = g_list_append (app->config->locations, path);//默认初始化根目录
    load_database(app);//更新数据库
    app->pool = fsearch_thread_pool_init ();//初始化线程池
    app->search = db_search_new (fsearch_application_get_thread_pool (app));
}
void fsearch_DeInit(char*path)
{
    app=(FsearchApplication *) calloc(1,sizeof(FsearchApplication));
    app->config = (FsearchConfig *)calloc (1, sizeof (FsearchConfig));
    if (!config_load_default (app->config)) {
    }
    app->db = NULL;
    app->search = NULL;
    app->config->locations=NULL;
    g_mutex_init (&app->mutex);
    app->config->locations = g_list_append (app->config->locations, path);//默认初始化根目录
    load_database(app);//更新数据库
    app->pool = fsearch_thread_pool_init ();//初始化线程池
    app->search = db_search_new (fsearch_application_get_thread_pool (app));
}
/*
 * 更新数据库
*/
void fsearch_UpdateDb(char*path)
{
    fsearch_Close();
    app->config->locations = g_list_append (app->config->locations, path);
    load_database(app);
}
/*
 *fsearch_Find 的回调函数，用于把数据用回调的方式传送给fsearch_Find的callback
*/
gboolean
update_model_cb (gpointer user_data)
{
    DatabaseSearchResult *result = user_data;
    // 防止在db_search_results_clear中触发断言，导致文管退出，先自行判断一下
    if (app->search == NULL) return FALSE;
    db_search_results_clear (app->search);
    results=NULL;
    num_results = 0;
    results = result->results;
    if (results) {
        app->search->results = results;
        app->search->num_folders = result->num_folders;;
        app->search->num_files = result->num_files;
        num_results = results->len;
        callBackFunc(app->search);
    }
    else {
        app->search->results = NULL;
        app->search->num_folders = 0;
        app->search->num_files = 0;
        num_results = 0;
    }
    free (result);
    result = NULL;
    return FALSE;
}
GPtrArray * get_Result(void)
{
    if(results!=NULL)
        return results;
    else {
        return NULL;
    }
}
unsigned int get_ResultLen(void)
{
  return num_results;

}
/*
 *  fsearch_Find的回调函数
 *
*/
void
fsearch_application_window_update_results (void *data)
{
    g_idle_add (update_model_cb, data);
}
/*
 *用于在线程池里面查找text
*/
#if 0
void fsearch_Find(const char*text)
{
    db_search_results_clear (app->search);
//    fsearch_UpdateDb(path);
    Database *db = app->db;
    if (!db_try_lock (db)) {
        return ;
    }
    if (app->search) {
            db_search_update (app->search,
                  db_get_entries (db),
                  db_get_num_entries (db),
                  1000,
                  0,
                  text,
                  app->config->hide_results_on_empty_search,
                  app->config->match_case,
                  app->config->enable_regex,
                  app->config->auto_search_in_path,
                  app->config->search_in_path);

            db_perform_search (app->search, fsearch_application_window_update_results, app);
        }
        db_unlock (db);
        return ;
}
#else
void fsearch_Find(const char*text,void (*callback)(void *))
{
    // 防止在db_search_results_clear中触发断言，导致文管退出，先自行判断一下
    if (app->search == NULL) return;

    db_search_results_clear (app->search);
    Database *db = app->db;
    if (!db_try_lock (db)) {
        return ;
    }
    if (app->search) {
            db_search_update (app->search,
                  db_get_entries (db),
                  db_get_num_entries (db),
                  500000,
                  0,
                  text,
                  app->config->hide_results_on_empty_search,
                  app->config->match_case,
                  app->config->enable_regex,
                  app->config->auto_search_in_path,
                  app->config->search_in_path);

            db_perform_search (app->search, fsearch_application_window_update_results, app);
        }
        db_unlock (db);
        callBackFunc = callback;
        return ;
}
#endif
/*
 *不再使用以后销毁不必要的内存
 *
*/
void fsearch_Close(void)
{
    FsearchApplication *fsearch = app;
    if(app==NULL)
    {
        return;
    }

    if (fsearch->db) {
//        db_save_locations (fsearch->db);
        db_clear (fsearch->db);
        db_free(fsearch->db);//内存数据库释放
    }
    if (fsearch->pool) {
        fsearch_thread_pool_free (fsearch->pool);
    }
//    config_save (fsearch->config);
    config_free (fsearch->config);
    db_search_free(fsearch->search);
    g_mutex_clear (&fsearch->mutex);\
    free(app);
    app = NULL;
}
