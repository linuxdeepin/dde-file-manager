// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
