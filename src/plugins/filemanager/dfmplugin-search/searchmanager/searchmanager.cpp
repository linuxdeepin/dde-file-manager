// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager.h"
#include "maincontroller/maincontroller.h"
#include "utils/searchhelper.h"
#include "cache/searchcachemanager.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include "plugins/common/dfmplugin-utils/reportlog/datas/searchreportdata.h"

#include <dfm-framework/dpf.h>

Q_DECLARE_METATYPE(const char *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_search;

SearchManager *SearchManager::instance()
{
    static SearchManager instance;
    return &instance;
}

bool SearchManager::search(quint64 winId, const QString &taskId, const QUrl &url, const QString &keyword)
{
    // Save task information first for caching results later
    taskInfoMap[taskId] = qMakePair(url, keyword);
    
    // Track all search tasks for this window
    winTasksMap.insert(winId, taskId);
    
    // Check if cache is available
    auto cacheManager = SearchCacheManager::instance();
    if (cacheManager->hasCache(url, keyword)) {
        DFMSearchResultMap cachedResults = cacheManager->getResultsFromCache(url, keyword);
        if (!cachedResults.isEmpty()) {
            fmInfo() << "Using cached search results for" << keyword << "in" << url.toString();
            taskIdMap[winId] = taskId;
            // Use cached results, directly trigger match and completion signals
            QMetaObject::invokeMethod(this, "matched", Qt::QueuedConnection, Q_ARG(QString, taskId));
            QMetaObject::invokeMethod(this, "searchCompleted", Qt::QueuedConnection, Q_ARG(QString, taskId));
            return true;
        }
    }
    
    // No cache or empty cache, perform actual search
    if (mainController) {
        taskIdMap[winId] = taskId;
        return mainController->doSearchTask(taskId, url, keyword);
    }

    return false;
}

DFMSearchResultMap SearchManager::matchedResults(const QString &taskId)
{
    // If cache exists and is valid, get results from cache
    if (taskInfoMap.contains(taskId)) {
        const auto &info = taskInfoMap[taskId];
        const QUrl &url = info.first;
        const QString &keyword = info.second;
        
        auto cacheManager = SearchCacheManager::instance();
        if (cacheManager->hasCache(url, keyword)) {
            return cacheManager->getResultsFromCache(url, keyword);
        }
    }
    
    // No cache, get real-time results from controller
    if (mainController)
        return mainController->getResults(taskId);

    return {};
}

QList<QUrl> SearchManager::matchedResultUrls(const QString &taskId)
{
    // If cache exists and is valid, get result URLs from cache
    if (taskInfoMap.contains(taskId)) {
        const auto &info = taskInfoMap[taskId];
        const QUrl &url = info.first;
        const QString &keyword = info.second;
        
        auto cacheManager = SearchCacheManager::instance();
        if (cacheManager->hasCache(url, keyword)) {
            return cacheManager->getResultsFromCache(url, keyword).keys();
        }
    }
    
    // No cache, get real-time result URLs from controller
    if (mainController)
        return mainController->getResultUrls(taskId);

    return {};
}

void SearchManager::stop(const QString &taskId)
{
    emit searchStoped(taskId);
}

void SearchManager::stop(quint64 winId)
{
    if (taskIdMap.contains(winId)) {
        QString taskId = taskIdMap[winId];
        
        // Clear all search caches for this window
        QList<QString> taskIds = winTasksMap.values(winId);
        for (const QString &tid : taskIds) {
            if (taskInfoMap.contains(tid)) {
                const auto &info = taskInfoMap[tid];
                const QUrl &url = info.first;
                const QString &keyword = info.second;
                
                // Clear the search cache for this specific search
                SearchCacheManager::instance()->clearSearchCache(url, keyword);
                fmInfo() << "Cleared search cache for" << keyword << "in" << url.toString() << "on stop";
            }
        }
        
        // Remove all tasks associated with this window
        winTasksMap.remove(winId);
        
        stop(taskId);
    }
}

void SearchManager::onDConfigValueChanged(const QString &config, const QString &key)
{
    if (config != DConfig::kSearchCfgPath || key != DConfig::kEnableFullTextSearch)
        return;

    using namespace dfmplugin_utils;

    QVariantMap data;
    bool enabled = DConfigManager::instance()->value(config, key, false).toBool();
    data.insert("mode", enabled ? SearchReportData::kTurnOn : SearchReportData::kTurnOff);
    dpfSignalDispatcher->publish("dfmplugin_search", "signal_ReportLog_Commit", QString("Search"), data);

    emit enableFullTextSearchChanged(enabled);
}

SearchManager::SearchManager(QObject *parent)
    : QObject(parent)
{
    init();
}

SearchManager::~SearchManager()
{
}

void SearchManager::init()
{
    Q_ASSERT(mainController == nullptr);

    mainController = new MainController(this);
    // Direct connection to prevent event loop from disrupting the sequence
    connect(mainController, &MainController::matched, this, &SearchManager::matched, Qt::DirectConnection);
    connect(mainController, &MainController::searchCompleted, this, [this](const QString &taskId) {
        // Update cache when search is completed
        if (taskInfoMap.contains(taskId)) {
            const auto &info = taskInfoMap[taskId];
            DFMSearchResultMap results = mainController->getResults(taskId);
            
            // Update cache using cache manager
            if (!results.isEmpty()) {
                fmInfo() << "Updating search results to cache for" << info.second << "in" << info.first.toString() 
                         << "Result count:" << results.size();
                SearchCacheManager::instance()->updateCache(taskId, info.first, info.second, results);
            }
        }
        
        emit searchCompleted(taskId);
    }, Qt::DirectConnection);
}
