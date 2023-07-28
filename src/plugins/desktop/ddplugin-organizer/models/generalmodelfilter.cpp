// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generalmodelfilter.h"

#include "filters/hiddenfilefilter.h"
#include "filters/innerdesktopappfilter.h"

using namespace ddplugin_organizer;
GeneralModelFilter::GeneralModelFilter() : ModelDataHandler()
{
    // the default filters
    modelFilters << QSharedPointer<ModelDataHandler>(new HiddenFileFilter());
    modelFilters << QSharedPointer<ModelDataHandler>(new InnerDesktopAppFilter());
}

bool GeneralModelFilter::installFilter(const QSharedPointer<ModelDataHandler> &filter)
{
    if (filter.get() && modelFilters.contains(filter))
        return false;
    modelFilters.append(filter);
    return true;
}

void GeneralModelFilter::removeFilter(const QSharedPointer<ModelDataHandler> &filter)
{
    modelFilters.removeAll(filter);
}

bool GeneralModelFilter::acceptInsert(const QUrl &url)
{
    bool accepted = std::all_of(modelFilters.begin(), modelFilters.end(),
                                [&url](const QSharedPointer<ModelDataHandler> &filter){
        return filter->acceptInsert(url);
    });
    if (!accepted)
        return false;

    return ModelDataHandler::acceptInsert(url);
}

QList<QUrl> GeneralModelFilter::acceptReset(const QList<QUrl> &urls)
{
    QList<QUrl> ret = urls;
    for (const auto &filter : modelFilters)
         ret = filter->acceptReset(ret);

    return ModelDataHandler::acceptReset(ret);
}

bool GeneralModelFilter::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    bool ret = true;
    for (const auto &filter : modelFilters)
        ret = ret && filter->acceptRename(oldUrl, newUrl);

    return ModelDataHandler::acceptRename(oldUrl, newUrl) && ret;
}

bool GeneralModelFilter::acceptUpdate(const QUrl &url, const QVector<int> &roles)
{
    // these filters is like Notifier.
    // so it will don't interrupt when some one return true.
    bool ret = true;
    for (const auto &filter : modelFilters)
        ret = ret && filter->acceptUpdate(url, roles);

    return ModelDataHandler::acceptUpdate(url, roles) && ret;
}
