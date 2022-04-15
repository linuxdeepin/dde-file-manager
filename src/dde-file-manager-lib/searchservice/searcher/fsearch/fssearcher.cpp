/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
 */
#include "fssearcher.h"
#include "utils/searchhelper.h"
#include "interfaces/dfileservices.h"

#include <QDebug>

namespace {
static int kEmitInterval = 50;   // 推送时间间隔（ms）
static uint32_t kMaxCount = 50000;   // 最大搜索结果数量
}

FsSearcher::FsSearcher(const DUrl &url, const QString &key, QObject *parent)
    :AbstractSearcher (url, key, parent)
{
    initApp();
}

FsSearcher::~FsSearcher()
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

bool FsSearcher::search()
{
    Q_ASSERT(app);
    isWorking = true;
    //准备状态切运行中，否则直接返回
    if (!status.testAndSetRelease(kReady, kRuning))
        return false;

    if (keyword.isEmpty()) {
        status.storeRelease(kCompleted);
        return false;
    }

    notifyTimer.start();

    auto info = DFileService::instance()->createFileInfo(nullptr, searchUrl);
    if (!info)
        return false;

    searchUrl = DUrl::fromLocalFile(info->absoluteFilePath());
    auto searchPath = info->absoluteFilePath().toLocal8Bit();
    load_database(app, searchPath.data(), &isWorking);//加载数据库
    if (!isWorking) return false;
    Q_ASSERT(app && app->search);

    db_search_results_clear(app->search);
    Database *db = app->db;
    if (!db_try_lock(db)) {
        return true;
    }

    if (app->search) {
        db_search_update(app->search,
                         db_get_entries(db),
                         db_get_num_entries(db),
                         kMaxCount,
                         FsearchFilter::FSEARCH_FILTER_NONE,
                         keyword.toLocal8Bit().data(),
                         app->config->hide_results_on_empty_search,
                         app->config->match_case,
                         app->config->enable_regex,
                         app->config->auto_search_in_path,
                         app->config->search_in_path);

        conditionMtx.lock();
        db_perform_search(app->search, cbReceiveResults, app, this);
        waitCondition.wait(&conditionMtx);
        conditionMtx.unlock();
    }
    db_unlock(db);

    //检查是否还有数据
    if (status.testAndSetRelease(kRuning, kCompleted)) {
        //发送数据
        if (hasItem())
            emit unearthed(this);
    }

    return true;
}

void FsSearcher::stop()
{
    isWorking = false;
    status.storeRelease(kTerminated);
}

bool FsSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

QList<DUrl> FsSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    return std::move(allResults);
}

void FsSearcher::initApp()
{
    app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));
    app->config = static_cast<FsearchConfig *>(calloc(1, sizeof(FsearchConfig)));
    config_load_default(app->config);
    app->db = nullptr;
    app->search = nullptr;
    app->config->locations = nullptr;
    g_mutex_init(&app->mutex);

    app->pool = fsearch_thread_pool_init(); //初始化线程池
    app->search = db_search_new(fsearch_application_get_thread_pool(app));
}

void FsSearcher::tryNotify()
{
    int cur = notifyTimer.elapsed();
    if (hasItem() && (cur - lastEmit) > kEmitInterval) {
        lastEmit = cur;
        qDebug() << "unearthed, current spend:" << cur;
        emit unearthed(this);
    }
}

bool FsSearcher::isSupported(const DUrl &url)
{
    if (!url.isValid())
        return false;

    auto info = DFileService::instance()->createFileInfo(nullptr, url);
    if (!info || info->isVirtualEntry())
        return false;

    QByteArray searchPath = info->absoluteFilePath().toLocal8Bit();
    return db_support(searchPath.data(), searchPath.startsWith("/data"));
}

void FsSearcher::cbReceiveResults(void *data, void *sender)
{
    DatabaseSearchResult *result = static_cast<DatabaseSearchResult *>(data);
    FsSearcher *self = static_cast<FsSearcher *>(sender);
    Q_ASSERT(result && self);

    if (self->app->search == nullptr) {
        self->conditionMtx.lock();
        self->waitCondition.wakeAll();
        self->conditionMtx.unlock();
        return;
    }

    uint32_t num_results = 0;
    if (result->results && result->results->len > 0) {
        num_results = result->results->len;
        for (uint32_t i = 0; i < num_results; ++i) {
            //中断
            if (self->status.loadAcquire() != kRuning) {
                self->conditionMtx.lock();
                self->waitCondition.wakeAll();
                self->conditionMtx.unlock();
                return;
            }

            QString fileName;
            DatabaseSearchEntry *entry = static_cast<DatabaseSearchEntry *>(g_ptr_array_index(result->results, i));
            if (entry && entry->node) {
                auto *pNode = entry->node;
                while (pNode != nullptr) {
                    //中断
                    if (self->status.loadAcquire() != kRuning) {
                        self->conditionMtx.lock();
                        self->waitCondition.wakeAll();
                        self->conditionMtx.unlock();
                        return;
                    }

                    if (pNode->name != nullptr) {
                        fileName.prepend(pNode->name);
                        if (pNode->parent && strcmp(pNode->name, "") != 0)
                            fileName.prepend("/");
                    }
                    pNode = pNode->parent;
                }
            }

            // 过滤文管设置的隐藏文件
            if (!SearchHelper::isHiddenFile(fileName, self->hiddenFilters, self->searchUrl.toLocalFile())) {
                QMutexLocker lk(&self->mutex);
                self->allResults << DUrl::fromLocalFile(fileName);
            }

            //推送
            self->tryNotify();
        }
    }

    self->conditionMtx.lock();
    self->waitCondition.wakeAll();
    self->conditionMtx.unlock();
}
