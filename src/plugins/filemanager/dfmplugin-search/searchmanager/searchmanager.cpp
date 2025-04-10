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
    // 先保存任务信息，用于后续缓存结果
    taskInfoMap[taskId] = qMakePair(url, keyword);
    
    // 检查是否有可用缓存
    auto cacheManager = SearchCacheManager::instance();
    if (cacheManager->hasCache(url, keyword)) {
        DFMSearchResultMap cachedResults = cacheManager->getResultsFromCache(url, keyword);
        if (!cachedResults.isEmpty()) {
            fmInfo() << "使用缓存的搜索结果" << keyword << "in" << url.toString();
            taskIdMap[winId] = taskId;
            // 使用缓存结果，直接触发匹配和完成信号
            QMetaObject::invokeMethod(this, "matched", Qt::QueuedConnection, Q_ARG(QString, taskId));
            QMetaObject::invokeMethod(this, "searchCompleted", Qt::QueuedConnection, Q_ARG(QString, taskId));
            return true;
        }
    }
    
    // 没有缓存或缓存为空，执行实际搜索
    if (mainController) {
        taskIdMap[winId] = taskId;
        return mainController->doSearchTask(taskId, url, keyword);
    }

    return false;
}

DFMSearchResultMap SearchManager::matchedResults(const QString &taskId)
{
    // 如果有缓存且缓存有效，从缓存获取结果
    if (taskInfoMap.contains(taskId)) {
        const auto &info = taskInfoMap[taskId];
        const QUrl &url = info.first;
        const QString &keyword = info.second;
        
        auto cacheManager = SearchCacheManager::instance();
        if (cacheManager->hasCache(url, keyword)) {
            return cacheManager->getResultsFromCache(url, keyword);
        }
    }
    
    // 没有缓存，从控制器获取实时结果
    if (mainController)
        return mainController->getResults(taskId);

    return {};
}

QList<QUrl> SearchManager::matchedResultUrls(const QString &taskId)
{
    // 如果有缓存且缓存有效，从缓存获取结果URLs
    if (taskInfoMap.contains(taskId)) {
        const auto &info = taskInfoMap[taskId];
        const QUrl &url = info.first;
        const QString &keyword = info.second;
        
        auto cacheManager = SearchCacheManager::instance();
        if (cacheManager->hasCache(url, keyword)) {
            return cacheManager->getResultsFromCache(url, keyword).keys();
        }
    }
    
    // 没有缓存，从控制器获取实时结果URLs
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
    if (taskIdMap.contains(winId))
        stop(taskIdMap[winId]);
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
    //直连，防止被事件循环打乱时序
    connect(mainController, &MainController::matched, this, &SearchManager::matched, Qt::DirectConnection);
    connect(mainController, &MainController::searchCompleted, this, [this](const QString &taskId) {
        // 搜索完成时更新缓存
        if (taskInfoMap.contains(taskId)) {
            const auto &info = taskInfoMap[taskId];
            DFMSearchResultMap results = mainController->getResults(taskId);
            
            // 使用缓存管理器更新缓存
            if (!results.isEmpty()) {
                fmInfo() << "将搜索结果更新到缓存中" << info.second << "in" << info.first.toString() 
                         << "结果数量:" << results.size();
                SearchCacheManager::instance()->updateCache(taskId, info.first, info.second, results);
            }
        }
        
        emit searchCompleted(taskId);
    }, Qt::DirectConnection);
}
