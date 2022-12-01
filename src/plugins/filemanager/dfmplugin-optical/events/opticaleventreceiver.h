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
    bool detailViewIcon(const QUrl &url, QString *iconName);

public:
    explicit OpticalEventReceiver(QObject *parent = nullptr);
};

}

#endif   // OPTICALEVENTRECEIVER_H
