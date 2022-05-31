/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include "ddplugin_core_global.h"

#include "interfaces/screen/abstractscreenproxy.h"
#include "base/application/application.h"
#include "interfaces/abstractdesktopframe.h"

#include "dfm-framework/dpf.h"

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
    QVector<DFMBASE_NAMESPACE::ScreenPointer> screens();
    QVector<DFMBASE_NAMESPACE::ScreenPointer> logicScreens();
    DFMBASE_NAMESPACE::ScreenPointer screen(const QString &name);
    qreal devicePixelRatio();
    int displayMode();
    int lastChangedMode();
    void reset();

    DFMBASE_NAMESPACE::AbstractDesktopFrame *desktopFrame();
    QList<QWidget *> rootWindows();
    void layoutWidget();
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
    virtual dpf::Plugin::ShutdownFlag stop() override;

protected slots:
    void onStart();
private:
    DFMBASE_NAMESPACE::Application *app = nullptr;
    EventHandle *handle = nullptr;
private:
    DPF_EVENT_NAMESPACE(DDPCORE_NAMESPACE)
    // AbstractScreenProxy begin
    DPF_EVENT_REG_SIGNAL(signal_ScreenProxy_ScreenChanged)
    DPF_EVENT_REG_SIGNAL(signal_ScreenProxy_DisplayModeChanged)
    DPF_EVENT_REG_SIGNAL(signal_ScreenProxy_ScreenGeometryChanged)
    DPF_EVENT_REG_SIGNAL(signal_ScreenProxy_ScreenAvailableGeometryChanged)

    //DPF_EVENT_REG_SLOT(slot_ScreenProxy_Instance)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_PrimaryScreen)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_Screens)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_LogicScreens)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_Screen)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_DevicePixelRatio)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_DisplayMode)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_LastChangedMode)
    DPF_EVENT_REG_SLOT(slot_ScreenProxy_Reset)

    // WindowFrame begin
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_WindowAboutToBeBuilded)
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_WindowBuilded)
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_WindowShowed)
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_GeometryChanged)
    DPF_EVENT_REG_SIGNAL(signal_DesktopFrame_AvailableGeometryChanged)

    //DPF_EVENT_REG_SLOT(slot_DesktopFrame_Instance)
    DPF_EVENT_REG_SLOT(slot_DesktopFrame_RootWindows)
    DPF_EVENT_REG_SLOT(slot_DesktopFrame_LayoutWidget)
};

DDPCORE_END_NAMESPACE

#endif   // COREPLUGIN_H
