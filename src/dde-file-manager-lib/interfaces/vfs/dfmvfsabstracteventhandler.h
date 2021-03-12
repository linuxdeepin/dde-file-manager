/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#ifndef DFMVFSABSTRACTEVENTHANDLER_H
#define DFMVFSABSTRACTEVENTHANDLER_H

#include <dfmglobal.h>
#include <QPointer>
#include <QEventLoop>

DFM_BEGIN_NAMESPACE

class DFMVfsAbstractEventHandler
{
public:
    virtual ~DFMVfsAbstractEventHandler() {}
    virtual int handleAskQuestion(QString message, QStringList choiceList) = 0;
    virtual QJsonObject handleAskPassword(QJsonObject defaultFields) = 0;
    virtual void handleMountError(int gioErrorCode, QString errorMessage) = 0;
    virtual void handleUnmountError(int gioErrorCode, QString errorMessage) = 0;

    QPointer<QEventLoop> eventLoop;
};

DFM_END_NAMESPACE

#endif // DFMVFSABSTRACTEVENTHANDLER_H
