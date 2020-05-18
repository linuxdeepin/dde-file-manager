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
#include <locale.h>
#include "fsearch_config.h"
#include "fsearch.h"
#include <stdlib.h>
#include <string.h>
#define DEBUG_SEARCH    1
bool m_searchState=FALSE;
typedef void (*callbackFunc)(void *);

FsearchApplication* app = NULL;
static GPtrArray *results;
static unsigned int  num_results = 0;
static char keyworld[256]={0};
/*
 *搜索初始化
*/
void fsearch_Init(void)
{
    app=(FsearchApplication *) calloc(1,sizeof(FsearchApplication));
    app->config = (FsearchConfig *)calloc (1, sizeof (FsearchConfig));
    if (!config_load_default (app->config)) {
    }
    app->config->locations = g_list_append (app->config->locations, "/");//默认初始化根目录
    app->db = NULL;
    app->search = NULL;
    g_mutex_init (&app->mutex);
    load_database(app);//更新数据库
    app->pool = fsearch_thread_pool_init ();//初始化线程池
    app->search = db_search_new (fsearch_application_get_thread_pool (app));//开启线程池
    m_searchState=TRUE;
}
/*
 * 更新数据库
*/
void fsearch_UpdateDb(char*path)
{
    if(m_searchState==TRUE)
    {
        app->config->locations = g_list_append (app->config->locations, path);
        load_database(app);
    }

}
/*
 *fsearch_Find 的回调函数，用于把数据用回调的方式传送给fsearch_Find的callback
*/
gboolean
update_model_cb (gpointer user_data)
{
    DatabaseSearchResult *result = user_data;
    results=NULL;
    db_search_results_clear (app->search);
    num_results = 0;

    results = result->results;
    if (results) {
        app->search->results = results;
        app->search->num_folders = result->num_folders;;
        app->search->num_files = result->num_files;
        num_results = results->len;
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
    if(m_searchState==TRUE){
        return results;
    }
    else {
        return NULL;
    }
}
unsigned int get_ResultLen(void)
{
    if(m_searchState==TRUE){
       return num_results;
    }
    else {
        return 0;
    }
}
char*  get_keyworld(void)
{
    return keyworld;
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
void fsearch_Find(const char*text)
{
    if(m_searchState==TRUE){
        Database *db = app->db;
        memcpy(keyworld,text,strlen(text));
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
    }
    return ;
}

/*
 *搜索不使用，要销毁不必要的内存
 *
*/
void fsearch_Close(void)
{
    if(m_searchState==TRUE){
        if(app){
            FsearchApplication *fsearch = app;

            if (fsearch->db) {
                db_save_locations (fsearch->db);
                db_clear (fsearch->db);
            }
            if (fsearch->pool) {
                fsearch_thread_pool_free (fsearch->pool);
            }
            config_free (fsearch->config);
            g_mutex_clear (&fsearch->mutex);
            app=0;
        }
    }

    m_searchState=FALSE;
}
/*
 * 返回搜索引擎状态，用于在搜索和不搜索状态间的切换
*/
bool get_search_state(void)
{
    return m_searchState;
}
