// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include "ddplugin_core_global.h"

#include <dfm-base/interfaces/screen/abstractscreenproxy.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/interfaces/abstractdesktopframe.h>

#include <dfm-framework/dpf.h>

DDPCORE_BEGIN_NAMESPACE
class WindowFrame;

class EventHandle : public QObject
{
    Q_OBJECT
public:
    explicit EventHandle(QObject *parent = nullptr);
    ~EventHandle();
    bool init();
public slots:
    DFMBASE_NAMESPACE::AbstractScreenProxy *screenProxyInstance();
    DFMBASE_NAMESPACE::ScreenPointer primaryScreen();
    QList<DFMBASE_NAMESPACE::ScreenPointer> screens();
    QList<DFMBASE_NAMESPACE::ScreenPointer> logicScreens();
    DFMBASE_NAMESPACE::ScreenPointer screen(const QString &name);
    qreal devicePixelRatio();
    int displayMode();
    int lastChangedMode();
    void reset();

    DFMBASE_NAMESPACE::AbstractDesktopFrame *desktopFrame();
    QList<QWidget *> rootWindows();
    void layoutWidget();

    bool screensInUse(QStringList *out);
private slots:
    void publishScreenChanged();
    void publishDisplayModeChanged();
    void publishScreenGeometryChanged();
    void publishScreenAvailableGeometryChanged();

    void publishWindowAboutToBeBuilded();
    void publishWindowBuilded();
    void publishWindowShowed();
    void publishGeometryChanged();
    void publishAvailableGeometryChanged();

public:
    WindowFrame *frame = nullptr;
    DFMBASE_NAMESPACE::AbstractScreenProxy *screenProxy = nullptr;
};

class Core : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "core.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

    Q_INVOKABLE void initializeAfterPainted();

protected slots:
    void onStart();
    void onFrameReady();
    void handleLoadPlugins(const QStringList &names);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void connectToServer();

private:
    DFMBASE_NAMESPACE::Application *app = nullptr;
    EventHandle *handle = nullptr;
    std::once_flag lazyFlag;

private:
    DPF_EVENT_NAMESPACE(DDPCORE_NAMESPACE)
    // AbstractScreenProxy begin
    DPF_EVENT_REG_SIGNAL(signal_ScreenProxy_ScreenChanged)
    DPF_EVENT_REG_SIGNAL(signal_ScreenProxy_DisplayModeChanged)
    DPF_EVENT_REG_SIGNAL(signal_ScreenProxy_ScreenGeometryChanged)
    DPF_EVENT_REG_SIGNAL(signal_ScreenProxy_ScreenAvailableGeometryChanged)

    // DPF_EVENT_REG_SLOT(slot_ScreenProxy_Instance)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_PrimaryScreen)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_Screens)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_LogicScreens)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_Screen)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_DevicePixelRatio)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_DisplayMode)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_LastChangedMode)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_Reset)

    DPF_EVENT_REG_HOOK(hook_ScreenProxy_ScreensInUse)

    // WindowFrame begin
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_WindowAboutToBeBuilded)
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_WindowBuilded)
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_WindowShowed)
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_GeometryChanged)
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_AvailableGeometryChanged)

    // DPF_EVENT_REG_SLOT(slot_DesktopFrame_Instance)
    DPF_EVENT_REG_SLOT(slot_DesktopFrame_RootWindows)
    DPF_EVENT_REG_SLOT(slot_DesktopFrame_LayoutWidget)
};

DDPCORE_END_NAMESPACE

#endif   // COREPLUGIN_H
