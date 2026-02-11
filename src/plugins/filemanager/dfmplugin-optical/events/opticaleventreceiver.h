// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALEVENTRECEIVER_H
#define OPTICALEVENTRECEIVER_H

#include "dfmplugin_optical_global.h"

#include <QObject>
#include <QUrl>

namespace dfmplugin_optical {

class OpticalEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OpticalEventReceiver)

public:
    static OpticalEventReceiver &instance();

public slots:
    bool handleDeleteFilesShortcut(quint64, const QList<QUrl> &urls, const QUrl &);
    bool handleCheckDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool handleMoveToTrashShortcut(quint64 winId, const QList<QUrl> &urls, const QUrl &rootUrl);
    bool handleCutFilesShortcut(quint64 winId, const QList<QUrl> &urls, const QUrl &rootUrl);
    bool handlePasteFilesShortcut(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to);
    bool sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);
    bool handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    bool handleBlockShortcutPaste(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to);
    bool detailViewIcon(const QUrl &url, QString *iconName);
    bool handleTabCloseable(const QUrl &currentUrl, const QUrl &rootUrl);

private:
    bool isContainPWSubDirFile(const QList<QUrl> &urls);

public:
    explicit OpticalEventReceiver(QObject *parent = nullptr);
};

}

#endif   // OPTICALEVENTRECEIVER_H
