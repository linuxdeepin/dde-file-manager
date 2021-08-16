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
#include "dfsearch.h"
#include <locale.h>
#include <stdlib.h>
#include <string.h>

extern "C"
{
#include "fsearch_config.h"
}


DFSearch::DFSearch(const QString &searchPath, void *parent)
    : caller(parent)
    , pathForSearching(searchPath)
{
    app = (FsearchApplication *) calloc(1, sizeof(FsearchApplication));
    app->config = (FsearchConfig *)calloc(1, sizeof(FsearchConfig));
    if (!config_load_default(app->config)) {
    }
    app->db = nullptr;
    app->search = nullptr;
    app->config->locations = nullptr;
    g_mutex_init(&app->mutex);
    app->pool = fsearch_thread_pool_init(); //初始化线程池
    app->search = db_search_new(fsearch_application_get_thread_pool(app));
}

DFSearch::~DFSearch()
{
    FsearchApplication *fsearch = app;
    if (app == nullptr) {
        return;
    }

    if (fsearch->db) {
//        db_save_locations (fsearch->db);
        db_clear(fsearch->db);
        db_free(fsearch->db);//内存数据库释放
    }
    if (fsearch->pool) {
        fsearch_thread_pool_free(fsearch->pool);
    }
//    config_save (fsearch->config);
    config_free(fsearch->config);
    db_search_free(fsearch->search);
    g_mutex_clear(&fsearch->mutex);
    free(app);
    app = nullptr;
}

void DFSearch::searchByKeyWord(const QString &key, void (*callback)(void *, void *))
{
    // 防止在db_search_results_clear中触发断言，导致文管退出，先自行判断一下
    if (app->search == nullptr) return;

    QByteArray pathBytes(pathForSearching.toLocal8Bit());
    load_database(app, pathBytes.data(), &state);//加载数据库
    if (!state)
        return;

    cbFunc = callback;
    db_search_results_clear(app->search);
    Database *db = app->db;
    if (!db_try_lock(db)) {
        return ;
    }
    if (app->search) {
        db_search_update(app->search,
                         db_get_entries(db),
                         db_get_num_entries(db),
                         50000,
                         FsearchFilter::FSEARCH_FILTER_NONE,
                         key.toStdString().c_str(),
                         app->config->hide_results_on_empty_search,
                         app->config->match_case,
                         app->config->enable_regex,
                         app->config->auto_search_in_path,
                         app->config->search_in_path);

        db_perform_search(app->search, fsearch_application_window_update_results, app, this);
    }
    db_unlock(db);    
    return ;
}

void DFSearch::stop()
{
    state = false;
}

bool DFSearch::isSupportFSearch(const QString &path)
{
    if (path.isEmpty())
        return false;

    QByteArray searchPath = path.toLocal8Bit();
    return db_support(searchPath.data(), path.startsWith("/data"));
}

void DFSearch::fsearch_application_window_update_results(void *data, void *sender)
{
//    g_idle_add (update_model_cb, data);
    update_model_cb(data, sender);
}

gboolean DFSearch::update_model_cb(gpointer user_data, gpointer sender)
{
    DatabaseSearchResult *result = static_cast<DatabaseSearchResult *>(user_data);
    DFSearch *self = static_cast<DFSearch *>(sender);
    // 防止在db_search_results_clear中触发断言，导致文管退出，先自行判断一下
    if (self->app->search == nullptr) return FALSE;
    db_search_results_clear(self->app->search);
    self->results = nullptr;
    self->num_results = 0;
    self->results = result->results;
    if (self->results) {
        self->app->search->results = self->results;
        self->app->search->num_folders = result->num_folders;;
        self->app->search->num_files = result->num_files;
        self->num_results = self->results->len;
        self->cbFunc(self->app->search, self->caller);
    } else {
        self->app->search->results = nullptr;
        self->app->search->num_folders = 0;
        self->app->search->num_files = 0;
        self->num_results = 0;
    }
    free(result);
    result = nullptr;
    return FALSE;
}
