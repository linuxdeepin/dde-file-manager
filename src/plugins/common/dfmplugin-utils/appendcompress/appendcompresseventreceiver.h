/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef APPENDCOMPRESSEVENTRECEIVER_H
#define APPENDCOMPRESSEVENTRECEIVER_H

#include "dfmplugin_utils_global.h"

#include <QObject>

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
};

}

#endif   // APPENDCOMPRESSEVENTRECEIVER_H
