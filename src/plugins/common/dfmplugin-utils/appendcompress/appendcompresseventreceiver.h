// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPENDCOMPRESSEVENTRECEIVER_H
#define APPENDCOMPRESSEVENTRECEIVER_H

#include "dfmplugin_utils_global.h"

#include <QObject>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QMimeData;
QT_END_NAMESPACE

namespace dfmplugin_utils {

class AppendCompressEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AppendCompressEventReceiver)

public:
    explicit AppendCompressEventReceiver(QObject *parent = nullptr);

    void initEventConnect();

public slots:
    bool handleSetMouseStyle(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *type);
    bool handleDragDropCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    bool handleSetMouseStyleOnDesktop(int viewIndex, const QMimeData *mime, const QPoint &viewPos, void *extData);
    bool handleDragDropCompressOnDesktop(int viewIndex, const QMimeData *md, const QPoint &viewPos, void *extData);
    bool handleSetMouseStyleOnOrganizer(const QString &viewId, const QMimeData *mime, const QPoint &viewPos, void *extData);
    bool handleDragDropCompressOnOsrganizer(const QString &viewId, const QMimeData *md, const QPoint &viewPos, void *extData);
    bool handleIsDrop(const QUrl &toUrl);
};

}

#endif   // APPENDCOMPRESSEVENTRECEIVER_H
