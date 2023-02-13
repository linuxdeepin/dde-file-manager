// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBBROWSERUTILS_H
#define SMBBROWSERUTILS_H

#include "dfmplugin_smbbrowser_global.h"

#include "dfm-mount/base/dmount_global.h"

#include <QString>
#include <QIcon>
#include <QMutex>

namespace dfmplugin_smbbrowser {

class SmbBrowserUtils : public QObject
{
    Q_DISABLE_COPY(SmbBrowserUtils)
public:
    static SmbBrowserUtils *instance();

    static QString networkScheme();
    static QUrl netNeighborRootUrl();
    static QIcon icon();
    static QString getShareDirFromUrl(const QUrl &url);

    bool mountSmb(const quint64 windowId, const QList<QUrl> urls);

    bool isServiceRuning(const QString &service);
    bool startService(const QString &service);

public:
    static QMutex mutex;
    static QMap<QUrl, SmbShareNode> shareNodes;

private:
    SmbBrowserUtils() = default;
};

}

#endif   // SMBBROWSERUTILS_H
