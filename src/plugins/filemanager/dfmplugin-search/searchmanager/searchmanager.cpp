// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager.h"
#include "maincontroller/maincontroller.h"
#include "utils/searchhelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/highlightprovider.h>
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
        const bool ok = mainController->doSearchTask(taskId, url, keyword);

        // T1: auto-apply the MatchMethod grouping for this search when a
        // SemanticAdapter will actually be created (C2). Stateless per-search
        // — re-applied on every search start. The user can still switch to any
        // other grouping manually; leaving search view restores the target
        // URL's persisted strategy via the existing per-URL persistence.
        //
        // doSearch() runs on the iterator's worker thread, but the workspace
        // model (slot_Model_SetGroup's target) lives on the main thread.
        // dpfSlotChannel->push is synchronous and does NOT hop threads, so a
        // direct push here would mutate the model from the wrong thread and
        // silently no-op. Dispatch to SearchManager's thread (main) instead.
        //
        // Guard: only push when the current strategy differs. Pushing the same
        // strategy again triggers setGroupingStrategy's toggle behavior
        // (Ascending↔Descending) — that's a UI affordance for menu clicks, not
        // desired here. Re-pushing on every keyword change would otherwise
        // flip the group order (Exact/Smart) on the second search.
        if (ok && SearchHelper::shouldEnableSemanticSearch(keyword)) {
            QMetaObject::invokeMethod(this, [winId]() {
                const QString current = dpfSlotChannel->push(
                    "dfmplugin_workspace", "slot_Model_CurrentGroupStrategy", winId).toString();
                if (current != MatchMethod::kStrategyName) {
                    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetGroup",
                                         winId, QString(MatchMethod::kStrategyName));
                }
            }, Qt::QueuedConnection);
        }
        return ok;
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

    // 取消与该搜索任务关联的 highlight 延迟加载请求
    auto it = taskInfoMap.find(taskId);
    if (it != taskInfoMap.end()) {
        const auto &keyword = it.value().second;
        HighlightProvider::instance()->cancelTask(keyword);
    }

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
    if (config != DConfig::kSearchCfgPath)
        return;

    using namespace dfmplugin_utils;

    if (key == DConfig::kEnableFullTextSearch) {
        QVariantMap data;
        bool enabled = DConfigManager::instance()->value(config, key, false).toBool();
        data.insert("mode", enabled ? SearchReportData::kTurnOn : SearchReportData::kTurnOff);

        fmInfo() << "Full-text search configuration changed - enabled:" << enabled;

        dpfSignalDispatcher->publish("dfmplugin_search", "signal_ReportLog_Commit", QString("Search"), data);

        emit enableFullTextSearchChanged(enabled);
    } else if (key == DConfig::kEnableOcrTextSearch) {
        bool enabled = DConfigManager::instance()->value(config, key, false).toBool();
        fmInfo() << "OCR text search configuration changed - enabled:" << enabled;
        emit enableOcrTextSearchChanged(enabled);
    } else if (key == DConfig::kEnableFileIndexSearch) {
        bool enabled = DConfigManager::instance()->value(config, key, true).toBool();
        fmInfo() << "File index search configuration changed - enabled:" << enabled;
        emit enableFileIndexSearchChanged(enabled);
    } else if (key == DConfig::kEnableSemanticSearch) {
        bool enabled = DConfigManager::instance()->value(config, key, false).toBool();
        fmInfo() << "Semantic search configuration changed - enabled:" << enabled;
        emit enableSemanticSearchChanged(enabled);
    }
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
