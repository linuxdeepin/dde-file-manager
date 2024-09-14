// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREDQMLENGINE_P_H
#define SHAREDQMLENGINE_P_H

#include <dfm-gui/sharedqmlengine.h>
#include <dfm-gui/applet.h>

#include <QQmlEngine>

DFMGUI_BEGIN_NAMESPACE

class SharedQmlEnginePrivate
{
    Q_DECLARE_PUBLIC(SharedQmlEngine)
public:
    explicit SharedQmlEnginePrivate(SharedQmlEngine *q);

    bool continueLoading();
    static QSharedPointer<QQmlEngine> engine();

    SharedQmlEngine *q_ptr;

    bool delay = false;
    Applet *curApplet = nullptr;
    QObject *rootObject = nullptr;
    QQmlContext *rootContext = nullptr;
    QQmlComponent *component = nullptr;

    /*!
     * \brief 如此设计是为手动控制程序退出时优先释放 QQmlEngine
     */
    QSharedPointer<QQmlEngine> sharedEngine;
    static QWeakPointer<QQmlEngine> s_globalEngine;
};

DFMGUI_END_NAMESPACE

#endif   // SHAREDQMLENGINE_P_H
