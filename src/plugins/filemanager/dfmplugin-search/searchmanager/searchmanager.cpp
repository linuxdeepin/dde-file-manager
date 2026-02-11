// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager.h"
#include "maincontroller/maincontroller.h"
#include "utils/searchhelper.h"

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
    fmInfo() << "Starting search task - winId:" << winId << "taskId:" << taskId << "url:" << url.toString() << "keyword:" << keyword;

    // Save task information
    taskInfoMap[taskId] = qMakePair(url, keyword);

    // Track all search tasks for this window
    winTasksMap.insert(winId, taskId);

    // Perform search
    if (mainController) {
        taskIdMap[winId] = taskId;
        return mainController->doSearchTask(taskId, url, keyword);
    }

    fmWarning() << "MainController not available, cannot start search task:" << taskId;
    return false;
}

DFMSearchResultMap SearchManager::matchedResults(const QString &taskId)
{
    // Get real-time results from controller
    if (mainController)
        return mainController->getResults(taskId);

    fmWarning() << "MainController not available, cannot retrieve results for taskId:" << taskId;
    return {};
}

QList<QUrl> SearchManager::matchedResultUrls(const QString &taskId)
{
    // Get real-time result URLs from controller
    if (mainController)
        return mainController->getResultUrls(taskId);

    fmWarning() << "MainController not available, cannot retrieve result URLs for taskId:" << taskId;
    return {};
}

void SearchManager::stop(const QString &taskId)
{
    if (mainController)
        mainController->stop(taskId);

    emit searchStoped(taskId);
}

void SearchManager::stop(quint64 winId)
{
    if (taskIdMap.contains(winId)) {
        QString taskId = taskIdMap[winId];

        // Remove all tasks associated with this window
        winTasksMap.remove(winId);

        stop(taskId);
    } else {
        fmDebug() << "No active search task found for window:" << winId;
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

    fmInfo() << "Full-text search configuration changed - enabled:" << enabled;

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
    connect(mainController, &MainController::matched, this, &SearchManager::matched, Qt::QueuedConnection);
    connect(mainController, &MainController::searchCompleted, this, &SearchManager::searchCompleted, Qt::QueuedConnection);
}
