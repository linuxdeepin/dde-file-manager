// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
    if (mainController) {
        taskIdMap[winId] = taskId;
        return mainController->doSearchTask(taskId, url, keyword);
    }

    return false;
}

QList<QUrl> SearchManager::matchedResults(const QString &taskId)
{
    if (mainController)
        return mainController->getResults(taskId);

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
    connect(mainController, &MainController::searchCompleted, this, &SearchManager::searchCompleted, Qt::DirectConnection);
}
