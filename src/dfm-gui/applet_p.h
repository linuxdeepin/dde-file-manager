// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLET_P_H
#define APPLET_P_H

#include <dfm-gui/applet.h>
#include <dfm-gui/sharedqmlengine.h>
#include <dfm-framework/lifecycle/pluginquickmetadata.h>

#include <QPointer>
#include <QQuickItem>
#include <QUrl>

DFMGUI_BEGIN_NAMESPACE

class AppletPrivate
{
    Q_DECLARE_PUBLIC(Applet)

public:
    explicit AppletPrivate(Applet *q);
    virtual ~AppletPrivate();

    virtual void setRootObject(QObject *item);
    void setState(Applet::State s);

    bool createComplete(SharedQmlEngine *engine);

    void dumpAppletTreeImpl(int level = 0);

    Applet *q_ptr;
    QUrl currentUrl;   // 当前 Applet 操作的文件 Url

    Applet::Flags flag { Applet::kApplet };
    Applet::State state { Applet::kNull };
    QUrl componentUrl;   // 加载的 QML 组件 Url
    QPointer<QObject> rootObject;   // Applet 对应的 QML 组件
    dpf::PluginQuickMetaPtr metaPtr;   // Applet 对应元信息
};

DFMGUI_END_NAMESPACE

#endif   // APPLET_P_H
