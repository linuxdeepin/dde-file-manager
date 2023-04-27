// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
    bool cancelDelete(quint64, const QList<QUrl> &urls);

    bool hookSetTabName(const QUrl &url, QString *tabName);

private:
    explicit SmbBrowserEventReceiver(QObject *parent = nullptr);
};

}

#endif   // SMBBROWSEREVENTRECEIVER_H
