// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTEVENTRECEIVER_H
#define RECENTEVENTRECEIVER_H

#include "dfmplugin_recent_global.h"

#include <QObject>

namespace dfmplugin_recent {

class RecentEventReceiver : public QObject
{
    Q_OBJECT

public:
    static RecentEventReceiver *instance();
    void initConnect();

public slots:
    void handleAddressInputStr(quint64 winId, QString *str);
    void handleWindowUrlChanged(quint64 winId, const QUrl &url);
    void handleUpdateRecent(const QList<QUrl> &urls, bool ok, const QString &errMsg);
    void handleFileRenameResult(quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg);
    void handleFileCutResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg);

private:
    explicit RecentEventReceiver(QObject *parent = nullptr);
};

}

#endif   // RECENTEVENTRECEIVER_H
