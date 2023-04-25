// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"
#include <dfm-base/utils/windowutils.h>
#include "screen/screenproxyqt.h"
#include "frame/windowframe.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/private/application_p.h>
#include <dfm-base/interfaces/screen/abstractscreen.h>

#include "stubext.h"

#include <gtest/gtest.h>

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DDPCORE_USE_NAMESPACE

TEST(TestCore, initialize)
{
    Core core;
    ASSERT_FALSE(UrlRoute::hasScheme(Global::Scheme::kFile));
    ASSERT_FALSE(InfoFactory::instance().constructList.contains(Global::Scheme::kFile));
    ASSERT_FALSE(DirIteratorFactory::instance().constructList.contains(Global::Scheme::kFile));
    ASSERT_FALSE(WatcherFactory::instance().constructList.contains(Global::Scheme::kFile));

    core.initialize();

    EXPECT_TRUE(UrlRoute::hasScheme(Global::Scheme::kFile));
    EXPECT_TRUE(InfoFactory::instance().constructList.contains(Global::Scheme::kFile));
    EXPECT_TRUE(DirIteratorFactory::instance().constructAguList.contains(Global::Scheme::kFile));
    EXPECT_TRUE(WatcherFactory::instance().constructList.contains(Global::Scheme::kFile));
}

TEST(TestCore, start)
{
    Core core;
    ASSERT_EQ(core.app, nullptr);
    ASSERT_EQ(core.handle, nullptr);
    ASSERT_EQ(ApplicationPrivate::self, nullptr);

    stub_ext::StubExt stub;
    bool flag = false;
    stub.set_lamda(&EventHandle::init, [&flag]() {
        flag = true;
        return false;
    });

    EXPECT_FALSE(core.start());
    EXPECT_NE(core.app, nullptr);
    EXPECT_NE(core.handle, nullptr);
    EXPECT_TRUE(flag);

    delete core.app;
    delete core.handle;
}

TEST(TestCore, stop)
{
    Core core;
    ASSERT_EQ(core.app, nullptr);
    ASSERT_EQ(core.handle, nullptr);

    core.app = new DFMBASE_NAMESPACE::Application();
    core.handle = new EventHandle();

    core.stop();

    EXPECT_EQ(core.app, nullptr);
    EXPECT_EQ(core.handle, nullptr);
}

TEST(TestCore, onStart)
{
    Core core;
    core.handle = new EventHandle();
    stub_ext::StubExt stub;
    bool flag = false;
    stub.set_lamda(&WindowFrame::buildBaseWindow, [&flag]() {
        flag = true;
    });
    core.handle->frame = new WindowFrame();

    core.onStart();

    EXPECT_TRUE(flag);

    delete core.handle;
}

TEST(TestEventHandle, init_event)
{
    EventHandle *handle = new EventHandle;

    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::isWayLand, []() {
        return false;
    });

    bool reset = false;
    stub.set_lamda(VADDR(ScreenProxyQt, reset), [&reset]() {
        reset = true;
    });

    bool init = false;
    stub.set_lamda(&WindowFrame::init, [&init]() {
        init = true;
        return true;
    });

    EXPECT_TRUE(handle->init());
    EXPECT_NE(handle->screenProxy, nullptr);
    EXPECT_NE(handle->frame, nullptr);
    EXPECT_TRUE(reset);
    EXPECT_TRUE(init);

    const QStringList allSlot = {
        "slot_ScreenProxy_PrimaryScreen",
        "slot_ScreenProxy_Screens",
        "slot_ScreenProxy_LogicScreens",
        "slot_ScreenProxy_Screen",
        "slot_ScreenProxy_DevicePixelRatio",
        "slot_ScreenProxy_DisplayMode",
        "slot_ScreenProxy_LastChangedMode",
        "slot_ScreenProxy_Reset",
        "slot_DesktopFrame_RootWindows",
        "slot_DesktopFrame_LayoutWidget"
    };
    for (const QString &topic : allSlot)
        EXPECT_TRUE(dpfSlotChannel->channelMap.contains(
                EventConverter::convert("ddplugin_core", topic)));

    delete handle;
    for (const QString &topic : allSlot)
        EXPECT_FALSE(dpfSlotChannel->channelMap.contains(
                EventConverter::convert("ddplugin_core", topic)));
}

TEST(TestEventHandle, functions)
{
    EventHandle *handle = new EventHandle;

    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::isWayLand, []() {
        return false;
    });

    stub.set_lamda(VADDR(ScreenProxyQt, reset), []() {
    });

    stub.set_lamda(&WindowFrame::init, []() {
        return true;
    });

    handle->init();
    ASSERT_NE(handle->screenProxy, nullptr);
    ASSERT_NE(handle->frame, nullptr);

    EXPECT_EQ(handle->screenProxyInstance(), handle->screenProxy);
    bool call = false;
    stub.set_lamda(VADDR(ScreenProxyQt, screens), [&call]() {
        call = true;
        return QList<ScreenPointer>();
    });
    handle->screens();
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(VADDR(ScreenProxyQt, primaryScreen), [&call]() {
        call = true;
        return ScreenPointer();
    });
    handle->primaryScreen();
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(VADDR(ScreenProxyQt, logicScreens), [&call]() {
        call = true;
        return QList<ScreenPointer>();
    });
    handle->logicScreens();
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(VADDR(ScreenProxyQt, screen), [&call]() {
        call = true;
        return ScreenPointer();
    });
    handle->screen("");
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(VADDR(ScreenProxyQt, devicePixelRatio), [&call]() {
        call = true;
        return 1.22;
    });
    EXPECT_EQ(handle->devicePixelRatio(), 1.22);
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(VADDR(ScreenProxyQt, displayMode), [&call]() {
        call = true;
        return kShowonly;
    });
    EXPECT_EQ(handle->displayMode(), kShowonly);
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(VADDR(ScreenProxyQt, lastChangedMode), [&call]() {
        call = true;
        return kShowonly;
    });
    EXPECT_EQ(handle->lastChangedMode(), kShowonly);
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(VADDR(ScreenProxyQt, reset), [&call]() {
        call = true;
    });
    handle->reset();
    EXPECT_TRUE(call);

    EXPECT_EQ(handle->desktopFrame(), handle->frame);

    call = false;
    stub.set_lamda(VADDR(WindowFrame, rootWindows), [&call]() {
        call = true;
        return QList<QWidget *>();
    });
    handle->rootWindows();
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(VADDR(WindowFrame, layoutChildren), [&call]() {
        call = true;
    });
    handle->layoutWidget();
    EXPECT_TRUE(call);

    delete handle;
}

TEST(TestEventHandle, signal_events)
{
    EventHandle *handle = new EventHandle;
    stub_ext::StubExt stub;
    QString space;
    QString topic;
    stub.set_lamda(((bool (EventDispatcherManager::*)(const QString &, const QString &))
                    & EventDispatcherManager::publish),
                   [&space, &topic](EventDispatcherManager *, const QString &t1, const QString &t2) {
                       space = t1;
                       topic = t2;
                       return true;
                   });

    handle->publishScreenChanged();
    EXPECT_EQ(QString("ddplugin_core"), space);
    EXPECT_EQ(QString("signal_ScreenProxy_ScreenChanged"), topic);

    handle->publishDisplayModeChanged();
    EXPECT_EQ(QString("ddplugin_core"), space);
    EXPECT_EQ(QString("signal_ScreenProxy_DisplayModeChanged"), topic);

    handle->publishScreenGeometryChanged();
    EXPECT_EQ(QString("ddplugin_core"), space);
    EXPECT_EQ(QString("signal_ScreenProxy_ScreenGeometryChanged"), topic);

    handle->publishScreenAvailableGeometryChanged();
    EXPECT_EQ(QString("ddplugin_core"), space);
    EXPECT_EQ(QString("signal_ScreenProxy_ScreenAvailableGeometryChanged"), topic);

    handle->publishWindowAboutToBeBuilded();
    EXPECT_EQ(QString("ddplugin_core"), space);
    EXPECT_EQ(QString("signal_DesktopFrame_WindowAboutToBeBuilded"), topic);

    handle->publishWindowBuilded();
    EXPECT_EQ(QString("ddplugin_core"), space);
    EXPECT_EQ(QString("signal_DesktopFrame_WindowBuilded"), topic);

    handle->publishWindowShowed();
    EXPECT_EQ(QString("ddplugin_core"), space);
    EXPECT_EQ(QString("signal_DesktopFrame_WindowShowed"), topic);

    handle->publishGeometryChanged();
    EXPECT_EQ(QString("ddplugin_core"), space);
    EXPECT_EQ(QString("signal_DesktopFrame_GeometryChanged"), topic);

    handle->publishAvailableGeometryChanged();
    EXPECT_EQ(QString("ddplugin_core"), space);
    EXPECT_EQ(QString("signal_DesktopFrame_AvailableGeometryChanged"), topic);
}
