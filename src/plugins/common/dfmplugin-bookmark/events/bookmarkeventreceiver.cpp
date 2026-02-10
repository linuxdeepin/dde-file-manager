// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bookmarkeventreceiver.h"
#include "controller/bookmarkmanager.h"
#include "controller/defaultitemmanager.h"

#include <dfm-framework/dpf.h>

#include <QDebug>

using namespace dfmplugin_bookmark;

BookMarkEventReceiver *BookMarkEventReceiver::instance()
{
    static BookMarkEventReceiver instance;
    return &instance;
}

void BookMarkEventReceiver::handleRenameFile(quint64 windowId, const QMap<QUrl, QUrl> &renamedUrls, bool result, const QString &errorMsg)
{
    Q_UNUSED(windowId)
    Q_UNUSED(errorMsg)
    if (!renamedUrls.isEmpty() && result) {
        auto iter = renamedUrls.constBegin();
        for (; iter != renamedUrls.constEnd(); ++iter) {
            BookMarkManager::instance()->fileRenamed(iter.key(), iter.value());
        }
    }
}

void BookMarkEventReceiver::handleSidebarOrderChanged(quint64 winId, const QString &group, const QList<QUrl> &urls)
{
    if (group != "Group_Common")
        return;

    BookMarkManager::instance()->saveSortedItemsToConfigFile(urls);
}

BookMarkEventReceiver::BookMarkEventReceiver(QObject *parent)
    : QObject(parent) { }
