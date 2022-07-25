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
#ifndef SHAREEVENTHELPER_H
#define SHAREEVENTHELPER_H

#include "dfmplugin_myshares_global.h"

#include <QUrl>
#include <QList>
#include <QObject>

namespace dfmplugin_myshares {

class ShareEventHelper : public QObject
{
public:
    static ShareEventHelper *instance();
    bool blockPaste(quint64 winId, const QUrl &to);
    bool blockDelete(quint64 winId, const QList<QUrl> &urls);
    bool blockMoveToTrash(quint64 winId, const QList<QUrl> &urls);
    bool hookSendOpenWindow(const QList<QUrl> &urls);
    bool hookSendChangeCurrentUrl(quint64 winId, const QUrl &url);

protected:
    explicit ShareEventHelper(QObject *parent = nullptr);
    bool containsShareUrl(const QList<QUrl> &urls);
};

}

#endif   // SHAREEVENTHELPER_H
