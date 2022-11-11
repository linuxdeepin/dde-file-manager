/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "searchmanager.h"
#include "maincontroller/maincontroller.h"
#include "utils/searchhelper.h"

#include "dfm-base/base/urlroute.h"

#include <dfm-framework/dpf.h>

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
