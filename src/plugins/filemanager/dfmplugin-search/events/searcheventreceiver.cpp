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
#include "searcheventreceiver.h"
#include "searcheventcaller.h"
#include "utils/searchhelper.h"
#include "searchmanager/searchmanager.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

DFMBASE_USE_NAMESPACE
namespace dfmplugin_search {

dfmplugin_search::SearchEventReceiver *dfmplugin_search::SearchEventReceiver::instance()
{
    static SearchEventReceiver ins;
    return &ins;
}

void SearchEventReceiver::handleSearch(quint64 winId, const QString &keyword)
{
    auto window = FMWindowsIns.findWindowById(winId);
    Q_ASSERT(window);

    const auto &curUrl = window->currentUrl();
    QUrl searchUrl;
    if (SearchHelper::isSearchFile(curUrl)) {
        const QUrl &targetUrl = SearchHelper::searchTargetUrl(curUrl);
        searchUrl = SearchHelper::fromSearchFile(targetUrl, keyword, QString::number(winId));
    } else {
        searchUrl = SearchHelper::fromSearchFile(curUrl, keyword, QString::number(winId));
    }

    SearchEventCaller::sendChangeCurrentUrl(winId, searchUrl);
}

void SearchEventReceiver::handleStopSearch(quint64 winId)
{
    SearchManager::instance()->stop(winId);
}

void SearchEventReceiver::handleShowAdvanceSearchBar(quint64 winId, bool visible)
{
    SearchEventCaller::sendShowAdvanceSearchBar(winId, visible);
}

void SearchEventReceiver::handleUrlChanged(quint64 winId, const QUrl &u)
{
    if (u.scheme() != SearchHelper::scheme())
        SearchEventReceiver::handleStopSearch(winId);
}

SearchEventReceiver::SearchEventReceiver(QObject *parent)
    : QObject(parent)
{
}

}
