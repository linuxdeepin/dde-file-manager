// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fsearchhandler.h"

#include <dfm-base/base/device/deviceutils.h>

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

FSearchHandler::FSearchHandler()
{
}

FSearchHandler::~FSearchHandler()
{
    isStop = true;
    syncMutex.lock();
    releaseApp();
}

bool FSearchHandler::checkPathSearchable(const QString &path)
{
    QMap<QString, QString> bindPathTable = DeviceUtils::fstabBindInfo();
    bool hasDataPrefix = false;
    for (const auto &key : bindPathTable.keys()) {
        if (path.startsWith(key)) {
            hasDataPrefix = true;
            break;
        }
    }

    return db_support(path.toLocal8Bit().data(), hasDataPrefix);
}

void FSearchHandler::init()
{
    if (app)
        reset();

    app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));
    app->config = static_cast<FsearchConfig *>(calloc(1, sizeof(FsearchConfig)));
    config_load_default(app->config);
    g_mutex_init(&app->mutex);

    app->db = db_new();
    app->pool = fsearch_thread_pool_init();
    app->search = db_search_new(fsearch_application_get_thread_pool(app));
}

void FSearchHandler::reset()
{
    setFlags(FSEARCH_FLAG_NONE);
    isStop = false;
    maxResults = DEFAULT_MAX_RESULTS;
    releaseApp();
}

bool FSearchHandler::loadDatabase(const QString &path, const QString &dbLocation)
{
    app->config->locations = g_list_append(app->config->locations, path.toLocal8Bit().data());
    return load_database(app->db, path.toLocal8Bit().data(),
                         dbLocation.isEmpty() ? nullptr : dbLocation.toLocal8Bit().data(),
                         &isStop);
}

bool FSearchHandler::updateDatabase()
{
    isStop = false;
    GList *locations = app->config->locations;
    for (GList *l = locations; l != nullptr; l = l->next) {
        char *path = static_cast<char *>(l->data);
        if (!load_database(app->db, path, nullptr, &isStop))
            return false;
    }

    return true;
}

bool FSearchHandler::saveDatabase(const QString &savePath)
{
    return db_save_locations(app->db, savePath.toLocal8Bit().data());
}

bool FSearchHandler::search(const QString &keyword, FSearchHandler::FSearchCallbackFunc callback)
{
    if (isStop)
        return false;

    callbackFunc = callback;
    db_search_results_clear(app->search);
    Database *db = app->db;
    if (!db_try_lock(db))
        return false;

    if (app->search) {
        db_search_update(app->search,
                         db_get_entries(db),
                         db_get_num_entries(db),
                         maxResults,
                         FsearchFilter::FSEARCH_FILTER_NONE,
                         keyword.toLocal8Bit().data(),
                         app->config->hide_results_on_empty_search,
                         app->config->match_case,
                         app->config->enable_regex,
                         app->config->auto_search_in_path,
                         app->config->search_in_path,
                         app->db->db_config->enable_py);
        syncMutex.lock();
        db_perform_search(app->search, FSearchHandler::reveiceResultsCallback, app, this);
    }

    db_unlock(db);
    return true;
}

void FSearchHandler::stop()
{
    isStop = true;
}

void FSearchHandler::setMaxResults(uint32_t max)
{
    maxResults = max;
}

void FSearchHandler::setFlags(FSearchFlags flags)
{
    if (flags.testFlag(FSEARCH_FLAG_FILTER_HIDDEN_FILE))
        app->db->db_config->filter_hidden_file = true;

    if (flags.testFlag(FSEARCH_FLAG_PINYIN))
        app->db->db_config->enable_py = true;

    if (flags.testFlag(FSEARCH_FLAG_REGEX))
        app->config->enable_regex = true;

    if (flags.testFlag(FSEARCH_FLAG_NONE)) {
        app->db->db_config->filter_hidden_file = false;
        app->db->db_config->enable_py = false;
        app->config->enable_regex = false;
    }
}

long FSearchHandler::dbTimeStamp()
{
    if (!app || !app->db)
        return 0;

    return app->db->timestamp;
}

void FSearchHandler::releaseApp()
{
    if (app) {
        if (app->db) {
            db_clear(app->db);
            db_free(app->db);
        }

        if (app->pool)
            fsearch_thread_pool_free(app->pool);

        config_free(app->config);
        db_search_free(app->search);
        g_mutex_clear(&app->mutex);
        free(app);
        app = nullptr;
    }
}

void FSearchHandler::reveiceResultsCallback(void *data, void *sender)
{
    DatabaseSearchResult *results = static_cast<DatabaseSearchResult *>(data);
    FSearchHandler *self = static_cast<FSearchHandler *>(sender);
    Q_ASSERT(results && self);

    if (self->isStop) {
        self->callbackFunc("", true);
        self->syncMutex.unlock();
        return;
    }

    if (results->results && results->results->len > 0) {
        uint32_t num_results = results->results->len;
        for (uint32_t i = 0; i < num_results; ++i) {
            if (self->isStop) {
                self->callbackFunc("", true);
                self->syncMutex.unlock();
                return;
            }

            std::string file_name { "" };
            auto *entry = static_cast<DatabaseSearchEntry *>(g_ptr_array_index(results->results, i));
            if (entry && entry->node) {
                auto *node = entry->node;
                while (node != nullptr) {
                    if (self->isStop) {
                        self->callbackFunc("", true);
                        self->syncMutex.unlock();
                        return;
                    }

                    if (node->name != nullptr) {
                        file_name.insert(0, node->name);
                        if (node->parent && strcmp(node->name, "") != 0)
                            file_name.insert(0, "/");
                    }

                    node = node->parent;
                }
            }

            self->callbackFunc(file_name.c_str(), false);
        }
    }

    self->callbackFunc("", true);
    self->syncMutex.unlock();
}
