// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcheventreceiver.h"
#include "searcheventcaller.h"
#include "utils/searchhelper.h"
#include "searchmanager/searchmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

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

void SearchEventReceiver::handleAddressInputStr(quint64 windId, QString *str)
{
    if (str->startsWith("search:?") && !str->contains("winId=")) {
        QString winId = "&winId=" + QString::number(windId);
        str->append(winId);
    }
}

void SearchEventReceiver::handleFileAdd(const QUrl &url)
{
    emit SearchManager::instance()->fileAdd(url);
}

void SearchEventReceiver::handleFileDelete(const QUrl &url)
{
    emit SearchManager::instance()->fileDelete(url);
}

void SearchEventReceiver::handleFileRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    emit SearchManager::instance()->fileRename(oldUrl, newUrl);
}

SearchEventReceiver::SearchEventReceiver(QObject *parent)
    : QObject(parent)
{
}

}
