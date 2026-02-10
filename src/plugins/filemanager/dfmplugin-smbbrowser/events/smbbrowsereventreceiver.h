// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBBROWSEREVENTRECEIVER_H
#define SMBBROWSEREVENTRECEIVER_H

#include "dfmplugin_smbbrowser_global.h"

#include <QObject>

namespace dfmplugin_smbbrowser {

class SmbBrowserEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SmbBrowserEventReceiver)

public:
    static SmbBrowserEventReceiver *instance();

public Q_SLOTS:
    bool detailViewIcon(const QUrl &url, QString *iconName);
    bool cancelDelete(quint64, const QList<QUrl> &urls, const QUrl &rootUrl);
    bool cancelMoveToTrash(quint64, const QList<QUrl> &, const QUrl &rootUrl);

    bool hookSetTabName(const QUrl &url, QString *tabName);

    bool hookTitleBarAddrHandle(QUrl *url);
    bool hookAllowRepeatUrl(const QUrl &cur, const QUrl &pre);
    bool hookCopyFilePath(quint64, const QList<QUrl> &urlList, const QUrl &rootUrl);

private:
    bool getOriginalUri(const QUrl &in, QUrl *out);

private:
    explicit SmbBrowserEventReceiver(QObject *parent = nullptr);
};

}

#endif   // SMBBROWSEREVENTRECEIVER_H
