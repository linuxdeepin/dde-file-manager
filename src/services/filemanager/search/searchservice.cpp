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
#include "searchservice.h"
#include "private/searchservice_p.h"

#include "dfm-base/base/urlroute.h"

#include <dfm-framework/framework.h>

#include <QtConcurrent>

DFMBASE_USE_NAMESPACE
DSB_FM_BEGIN_NAMESPACE

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

SearchService *SearchService::service()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::SearchService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::SearchService>(DSB_FM_NAMESPACE::SearchService::name());
}

bool SearchService::search(const QString &taskId, const QUrl &url, const QString &keyword)
{
    if (d->mainController)
        return d->mainController->doSearchTask(taskId, url, keyword);

    return false;
}

QList<QUrl> SearchService::matchedResults(const QString &taskId)
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

bool SearchService::regCustomSearchInfo(const Search::CustomSearchInfo &info)
{
    if (info.scheme.isEmpty() || !UrlRoute::hasScheme(info.scheme)) {
        qWarning() << QString("the scheme \"%1\" has not register!").arg(info.scheme);
        return false;
    }

    if (!info.redirectedPath.isEmpty() && !QFile::exists(info.redirectedPath)) {
        qWarning() << QString("the redirectedPath \"%1\" is not exists!").arg(info.redirectedPath);
        return false;
    }

    if (d->customSearchInfoHash.contains(info.scheme)) {
        qWarning() << QString("the scheme \"%1\" has already registered!").arg(info.scheme);
        return false;
    }

    d->customSearchInfoHash.insert(info.scheme, info);
    return true;
}

Search::CustomSearchInfo SearchService::findCustomSearchInfo(const QString &scheme)
{
    if (!d->customSearchInfoHash.contains(scheme))
        return {};

    return d->customSearchInfoHash[scheme];
}

SearchService::SearchService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<SearchService>(),
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

DSB_FM_END_NAMESPACE
