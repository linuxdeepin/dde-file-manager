// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWFRAME_P_H
#define WINDOWFRAME_P_H

#include "windowframe.h"
#include "basewindow.h"

#include <dfm-base/interfaces/screen/abstractscreen.h>

#include <QMap>
#include <QReadWriteLock>

DDPCORE_BEGIN_NAMESPACE

class WindowFramePrivate : public QObject
{
    Q_OBJECT
public:
    explicit WindowFramePrivate(WindowFrame *parent);
    void updateProperty(BaseWindowPointer win, DFMBASE_NAMESPACE::ScreenPointer screen, bool primary);
    BaseWindowPointer createWindow(DFMBASE_NAMESPACE::ScreenPointer sp);
public slots:
public:
    QMap<QString, BaseWindowPointer> windows;
    QReadWriteLock locker;

private:
    WindowFrame *q;
};

DDPCORE_END_NAMESPACE

#endif   // WINDOWFRAME_P_H
