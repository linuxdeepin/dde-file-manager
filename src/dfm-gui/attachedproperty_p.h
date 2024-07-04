// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ATTACHEDPROPERTY_P_H
#define ATTACHEDPROPERTY_P_H

#include <dfm-gui/applet.h>
#include <dfm-gui/containment.h>
#include <dfm-gui/panel.h>

#include <qqml.h>

DFMGUI_BEGIN_NAMESPACE

class AppletAttached : public QObject
{
    Q_OBJECT
public:
    explicit AppletAttached(QObject *parent = nullptr);
    ~AppletAttached() override;

    static Applet *qmlAttachedProperties(QObject *object);
};

class ContainmentAttached : public QObject
{
    Q_OBJECT
public:
    explicit ContainmentAttached(QObject *parent = nullptr);
    ~ContainmentAttached() override;

    static Containment *qmlAttachedProperties(QObject *object);
};

class PanelAttached : public QObject
{
    Q_OBJECT
public:
    explicit PanelAttached(QObject *parent = nullptr);
    ~PanelAttached() override;

    static Panel *qmlAttachedProperties(QObject *object);
};

DFMGUI_END_NAMESPACE

// 此宏标记QML附加属性，需要完整命名空间
QML_DECLARE_TYPEINFO(DFMGUI_NAMESPACE::AppletAttached, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPEINFO(DFMGUI_NAMESPACE::ContainmentAttached, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPEINFO(DFMGUI_NAMESPACE::PanelAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif   // ATTACHEDPROPERTY_P_H
