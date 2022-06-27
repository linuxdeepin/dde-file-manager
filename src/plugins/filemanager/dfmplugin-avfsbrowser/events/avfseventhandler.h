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
#ifndef AVFSEVENTHANDLER_H
#define AVFSEVENTHANDLER_H

#include "dfmplugin_avfsbrowser_global.h"

#include <QUrl>
#include <QList>
#include <QObject>

namespace dfmplugin_avfsbrowser {

class AvfsEventHandler : public QObject
{
    AvfsEventHandler() {};
    Q_DISABLE_COPY(AvfsEventHandler);

public:
    inline static AvfsEventHandler *instance()
    {
        static AvfsEventHandler ins;
        return &ins;
    }

    bool hookOpenFiles(quint64 winId, const QList<QUrl> &urls);
    bool hookEnterPressed(quint64 winId, const QList<QUrl> &urls);

    void openArchivesAsDir(quint64 winId, const QList<QUrl> &urls);
    void writeToClipbord(quint64 winId, const QList<QUrl> &urls);
    void showProperty(const QList<QUrl> &urls);
};

}

#endif   // AVFSEVENTHANDLER_H
