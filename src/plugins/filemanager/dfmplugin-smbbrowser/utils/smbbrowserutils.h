/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

    bool mountSmb(const quint64 windowId, const QList<QUrl> urls);

public:
    static QMutex mutex;
    static QMap<QUrl, SmbShareNode> shareNodes;

private:
    SmbBrowserUtils() = default;
};

}

#endif   // SMBBROWSERUTILS_H
