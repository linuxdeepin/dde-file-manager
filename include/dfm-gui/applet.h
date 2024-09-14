// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLET_H
#define APPLET_H

#include <dfm-gui/dfm_gui_global.h>
#include <dfm-framework/lifecycle/pluginquickmetadata.h>

#include <QObject>

class QQuickItem;

DFMGUI_BEGIN_NAMESPACE

class Containment;
class Panel;

class AppletPrivate;
class Applet : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl currentUrl READ currentUrl WRITE setCurrentUrl NOTIFY currentUrlChanged FINAL)
    Q_PROPERTY(QObject *rootObject NOTIFY rootObjectChanged FINAL)

public:
    explicit Applet(QObject *parent = nullptr);
    ~Applet() override;

    enum Flag {
        kUnknown,
        kApplet = 0x1,   // 基础组件
        kContainment = 0x2,   // 容器组件，可管理基础控件
        kPanel = 0x4 | kContainment,   // 独特的容器组件，用于顶层窗体
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    Q_FLAG(Flags)
    Flags flags() const;

    enum State {
        kNull,
        kLoading,
        kReady,
        kError,
    };
    State state() const;
    Q_SIGNAL void stateChanged(State s);

    bool createRootObject(bool async = false);
    QObject *rootObject() const;
    Q_SIGNAL void rootObjectChanged(QObject *object);

    QUrl currentUrl() const;
    Q_SLOT void setCurrentUrl(const QUrl &url);
    Q_SIGNAL void currentUrlChanged(const QUrl &url);

    Containment *containment() const;
    Panel *panel() const;

    QString plugin() const;
    QString id() const;

    QUrl componentUrl() const;
    void setComponentUrl(const QUrl &url);
    Q_SIGNAL void componentUrlChanged(const QUrl &url);

    void dumpAppletTree();

protected:
    explicit Applet(AppletPrivate &dd, QObject *parent = nullptr);
    QScopedPointer<AppletPrivate> dptr;

private:
    Q_DECLARE_PRIVATE_D(dptr, Applet)
    Q_DISABLE_COPY(Applet)

    friend class AppletItem;
    friend class ContainmentItem;
    friend class SharedQmlEngine;
    friend class AppletManager;
    friend class WindowManager;
    friend class WindowManagerPrivate;
};

DFMGUI_END_NAMESPACE

#endif   // APPLET_H
