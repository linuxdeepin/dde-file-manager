// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generalmodelfilter.h"

#include "filters/hiddenfilefilter.h"
#include "filters/innerdesktopappfilter.h"

using namespace ddplugin_organizer;
GeneralModelFilter::GeneralModelFilter()
    : ModelDataHandler()
{
    // the default filters
    defaultFilters << new HiddenFileFilter();
    defaultFilters << new InnerDesktopAppFilter();
    modelFilters << defaultFilters;
}

GeneralModelFilter::~GeneralModelFilter()
{
    for (auto filter : defaultFilters)
        delete filter;
    defaultFilters.clear();
    modelFilters.clear();
}

bool GeneralModelFilter::installFilter(ModelDataHandler *filter)
{
    if (!filter)
        return false;
    if (modelFilters.contains(filter))
        return false;
    modelFilters.append(filter);
    return true;
}

void GeneralModelFilter::removeFilter(ModelDataHandler *filter)
{
    modelFilters.removeAll(filter);
}

bool GeneralModelFilter::acceptInsert(const QUrl &url)
{
    bool accepted = std::all_of(modelFilters.begin(), modelFilters.end(),
                                [&url](ModelDataHandler *filter) {
                                    return filter ? filter->acceptInsert(url) : false;
                                });
    if (!accepted)
        return false;

    return ModelDataHandler::acceptInsert(url);
}

QList<QUrl> GeneralModelFilter::acceptReset(const QList<QUrl> &urls)
{
    QList<QUrl> ret = urls;
    for (const auto &filter : modelFilters) {
        if (filter)
            ret = filter->acceptReset(ret);
    }

    return ModelDataHandler::acceptReset(ret);
}

bool GeneralModelFilter::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    bool ret = true;
    for (const auto &filter : modelFilters)
        ret = ret && filter && filter->acceptRename(oldUrl, newUrl);

    return ModelDataHandler::acceptRename(oldUrl, newUrl) && ret;
}

bool GeneralModelFilter::acceptUpdate(const QUrl &url, const QVector<int> &roles)
{
    // these filters is like Notifier.
    // so it will don't interrupt when some one return true.
    bool ret = true;
    for (const auto &filter : modelFilters)
        ret = ret && filter && filter->acceptUpdate(url, roles);

    return ModelDataHandler::acceptUpdate(url, roles) && ret;
}
