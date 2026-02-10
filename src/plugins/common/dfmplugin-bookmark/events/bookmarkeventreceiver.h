// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARKEVENTRECEIVER_H
#define BOOKMARKEVENTRECEIVER_H

#include "dfmplugin_bookmark_global.h"

#include <QObject>

namespace dfmplugin_bookmark {

class BookMarkEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BookMarkEventReceiver)

public:
    static BookMarkEventReceiver *instance();

public Q_SLOTS:
    void handleRenameFile(quint64 windowId, const QMap<QUrl, QUrl> &renamedUrls, bool result, const QString &errorMsg);
    void handleSidebarOrderChanged(quint64 winId, const QString &group, const QList<QUrl> &urls);

private:
    explicit BookMarkEventReceiver(QObject *parent = nullptr);
};

}

#endif   // BOOKMARKEVENTRECEIVER_H
