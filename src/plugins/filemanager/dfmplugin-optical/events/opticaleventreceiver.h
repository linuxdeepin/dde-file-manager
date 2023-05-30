// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALEVENTRECEIVER_H
#define OPTICALEVENTRECEIVER_H

#include "dfmplugin_optical_global.h"

#include <QObject>

namespace dfmplugin_optical {

class OpticalEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OpticalEventReceiver)

public:
    static OpticalEventReceiver &instance();

public slots:
    bool handleDeleteFilesShortcut(quint64, const QList<QUrl> &urls);
    bool handleCheckDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);
    bool handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    bool handleBlockShortcutPaste(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to);
    bool detailViewIcon(const QUrl &url, QString *iconName);
    bool handleTabClosable(const QUrl &currentUrl, const QUrl &rootUrl);

public:
    explicit OpticalEventReceiver(QObject *parent = nullptr);
};

}

#endif   // OPTICALEVENTRECEIVER_H
