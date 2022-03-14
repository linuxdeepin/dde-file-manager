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
#include "searchservice.h"
#include "private/searchservice_p.h"

SearchServicePrivate::SearchServicePrivate(SearchService *parent)
    : QObject(parent)
{
}

SearchServicePrivate::~SearchServicePrivate()
{
    if (mainController) {
        delete mainController;
        mainController = nullptr;
    }
}

SearchService::SearchService(QObject *parent)
    : QObject(parent),
      d(new SearchServicePrivate(this))
{
    init();
}

SearchService::~SearchService()
{
}

void SearchService::init()
{
    Q_ASSERT(d->mainController == nullptr);

    d->mainController = new MainController;
    //直连，防止被事件循环打乱时序
    connect(d->mainController, &MainController::matched, this, &SearchService::matched, Qt::DirectConnection);
    connect(d->mainController, &MainController::searchCompleted, this, &SearchService::searchCompleted, Qt::DirectConnection);
}

SearchService *SearchService::instance()
{
    static SearchService ins;
    return &ins;
}

bool SearchService::search(const QString &taskId, const DUrl &url, const QString &keyword)
{
    if (d->mainController)
        return d->mainController->doSearchTask(taskId, url, keyword);

    return false;
}

QList<DUrl> SearchService::matchedResults(const QString &taskId)
{
    if (d->mainController)
        return d->mainController->getResults(taskId);

    return {};
}

void SearchService::stop(const QString &taskId)
{
    if (d->mainController)
        d->mainController->stop(taskId);

    emit searchStoped(taskId);
}

void SearchService::createFullTextIndex()
{
    if (d->mainController)
        d->mainController->createFullTextIndex();
}
