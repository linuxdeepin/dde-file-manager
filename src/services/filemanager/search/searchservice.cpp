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

bool SearchService::regSearchPath(const QString &scheme, const QString &path, QString *errMsg)
{
    if (!UrlRoute::hasScheme(scheme)) {
        if (errMsg)
            *errMsg = QString("the scheme \"%1\" has not register!").arg(scheme);
        return false;
    }

    if (!QFile::exists(path)) {
        if (errMsg)
            *errMsg = QString("the path \"%1\" is not exists!").arg(path);
        return false;
    }

    if (d->registerInfos.contains(scheme)) {
        if (errMsg)
            *errMsg = QString("the scheme \"%1\" already registered!").arg(scheme);
        return false;
    }

    d->registerInfos.insert(scheme, path);
    return true;
}

QHash<QString, QString> SearchService::regInfos()
{
    return d->registerInfos;
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
