// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/framemanager_p.h"
#include "config/configpresenter.h"

#include <dfm-base/dfm_desktop_defines.h>

#include "dfm-framework/dpf.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;
DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(FrameManager, initialize_enable)
{
    FrameManager fm;
    stub_ext::StubExt stub;
    bool turned = false;
    stub.set_lamda(&ConfigPresenter::isEnable, [](){
                       return true;
                   });
    stub.set_lamda(&FrameManager::turnOn, [&turned](){
                 turned = true;
             });

    fm.initialize();
    EXPECT_TRUE(turned);
}

TEST(FrameManager, initialize_disbale)
{
    FrameManager fm;
    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigPresenter::isEnable, [](){
                 return false;
             });

    bool turned = false;
    stub.set_lamda(&FrameManager::turnOn, [&turned](){
                 turned = true;
             });
    fm.initialize();
    EXPECT_FALSE(turned);
}

TEST(FrameManager, turnon_true)
{
    FrameManager fm;
    stub_ext::StubExt stub;
    bool builded = false;
    stub.set_lamda(&FrameManager::onBuild, [&builded](){
                 builded = true;;
             });

    fm.turnOn(true);
    EXPECT_TRUE(builded);
}

TEST(FrameManager, turnon_off)
{
    FrameManager fm;
    stub_ext::StubExt stub;
    bool builded = false;
    stub.set_lamda(&FrameManager::onBuild, [&builded](){
                 builded = true;;
             });

    fm.turnOn(false);
    EXPECT_FALSE(builded);
}

TEST(FrameManagerPrivate, showOptionWindow)
{
    FrameManagerPrivate obj(nullptr);
    ASSERT_EQ(obj.options, nullptr);

    stub_ext::StubExt stub;
    bool show = false;
    stub.set_lamda(&QWidget::show, [&show](){
        show = true;
    });

    obj.showOptionWindow();

    ASSERT_NE(obj.options, nullptr);
    EXPECT_TRUE(show);

    bool ac = false;
    stub.set_lamda(&QWidget::activateWindow, [&ac](){
        ac = true;
    });

    show = false;
    obj.showOptionWindow();
    auto old = obj.options;

    EXPECT_EQ(obj.options, old);
    EXPECT_TRUE(ac);
    EXPECT_FALSE(show);
}

TEST(FrameManagerPrivate, buildSurface)
{
    stub_ext::StubExt stub;
    QWidget root;
    root.setProperty(DesktopFrameProperty::kPropScreenName, "1");
    root.setGeometry(100, 100, 800, 600);

    QList<QWidget *> roots { &root };
    bool call = false;
    stub.set_lamda(((QVariant (EventChannelManager::*)(const QString &, const QString &))
                   &EventChannelManager::push), [&roots, &call]
                   (EventChannelManager *, const QString &t1, const QString &t2) {
        if (t1 == "ddplugin_core") {
            if (t2 == "slot_DesktopFrame_RootWindows") {
                call = true;
                return QVariant::fromValue(roots);
            }
        }

        return QVariant();
    });

   FrameManagerPrivate obj(nullptr);
   obj.buildSurface();
   EXPECT_TRUE(call);
   ASSERT_EQ(obj.surfaces().size(), 1);
   ASSERT_TRUE(obj.surfaceWidgets.contains("1"));

   auto sur = obj.surfaceWidgets.value("1");
   ASSERT_NE(sur, nullptr);
   EXPECT_EQ(sur->geometry(), QRect(0,0,800,600));
   EXPECT_EQ(sur->property(DesktopFrameProperty::kPropScreenName).toString(),
             QString("1"));
   EXPECT_EQ(sur->property(DesktopFrameProperty::kPropWidgetName).toString(),
             QString("organizersurface"));
   EXPECT_EQ(sur->property(DesktopFrameProperty::kPropWidgetLevel).toDouble()
             , 11.0);
   EXPECT_EQ(sur->parent(), &root);
}

TEST(FrameManagerPrivate, surfaces)
{
    stub_ext::StubExt stub;
    QWidget root;
    root.setProperty(DesktopFrameProperty::kPropScreenName, "1");
    root.setGeometry(100, 100, 800, 600);

    QList<QWidget *> roots { &root };
    bool call = false;
    stub.set_lamda(((QVariant (EventChannelManager::*)(const QString &, const QString &))
                   &EventChannelManager::push), [&roots, &call]
                   (EventChannelManager *, const QString &t1, const QString &t2) {
        if (t1 == "ddplugin_core") {
            if (t2 == "slot_DesktopFrame_RootWindows") {
                call = true;
                return QVariant::fromValue(roots);
            }
        }

        return QVariant();
    });

    FrameManagerPrivate obj(nullptr);
    obj.surfaceWidgets.insert("1", SurfacePointer(new Surface));
    auto sur = obj.surfaceWidgets.value("1");
    obj.surfaceWidgets.insert("2", SurfacePointer(new Surface));

    auto rets = obj.surfaces();
    ASSERT_EQ(rets.size(), 1);
    EXPECT_EQ(rets.first(), sur);
    EXPECT_TRUE(call);
}
