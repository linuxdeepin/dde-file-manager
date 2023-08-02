// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/framemanager_p.h"
#include "config/configpresenter.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasinterface.h"
#include "interface/canvasmanagershell.h"
#include "interface/canvasinterface_p.h"
#include <dfm-base/dfm_desktop_defines.h>
#include "mode/custommode.h"
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

class test_CanvasOrganizer :public CanvasOrganizer
{
public:
    virtual OrganizerMode mode() const { return OrganizerMode::kCustom;}
    virtual bool initialize(CollectionModel *) { return true;}
};

class test_CanvasOrganizer1 :public CanvasOrganizer
{
public:
    virtual OrganizerMode mode() const { return OrganizerMode::kNormalized;}
    virtual bool initialize(CollectionModel *) { return true;}
};

TEST(FrameManagerPrivate, switchToCustom)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&FrameManagerPrivate::buildOrganizer,[](){});
    FrameManagerPrivate obj(nullptr);

    obj.organizer = new test_CanvasOrganizer();
    EXPECT_NO_FATAL_FAILURE(obj.switchToCustom());
    delete obj.organizer;
    obj.organizer = new test_CanvasOrganizer1();
    EXPECT_NO_FATAL_FAILURE(obj.switchToCustom());
}

TEST(FrameManagerPrivate, switchToNormalized)
{
     stub_ext::StubExt stub;
     bool call = false;
     stub.set_lamda(&FrameManagerPrivate::buildOrganizer,[&call](){call = true;});
     FrameManagerPrivate obj(nullptr);
     obj.organizer = new test_CanvasOrganizer1();
     EXPECT_NO_FATAL_FAILURE(obj.switchToNormalized(1));
     EXPECT_FALSE(call);
     obj.organizer = new test_CanvasOrganizer();
     EXPECT_NO_FATAL_FAILURE(obj.switchToNormalized(1));
     EXPECT_TRUE(call);
}

TEST(FrameManagerPrivate, displaySizeChanged)
{
     stub_ext::StubExt stub;
     bool call = false;
     stub.set_lamda(&FrameManager::layout,[&call](){call = true;});

     FrameManagerPrivate obj(nullptr);
     obj.q = new FrameManager();
     EXPECT_NO_FATAL_FAILURE(obj.displaySizeChanged(0));
     EXPECT_FALSE(call);
     obj.canvas = new CanvasInterface();
     EXPECT_NO_FATAL_FAILURE(obj.displaySizeChanged(1));
     EXPECT_TRUE(call);
}

TEST(FrameManagerPrivate, filterShortcutkeyPress)
{
    FrameManagerPrivate obj(nullptr);
    EXPECT_TRUE(obj.filterShortcutkeyPress(1,Qt::Key_Equal ,Qt::ControlModifier));
    EXPECT_TRUE(obj.filterShortcutkeyPress(1,Qt::Key_Minus  ,Qt::ControlModifier));
    EXPECT_FALSE(obj.filterShortcutkeyPress(1,Qt::Key_Minus  ,Qt::NoModifier));
}

TEST(FrameManagerPrivate, findView)
{
    FrameManagerPrivate obj(nullptr);
    QWidget *root = new QWidget();
    QObjectList children;
    EXPECT_EQ(obj.findView(root),nullptr);
    QWidget *widget =  new QWidget();
    widget->setProperty(DesktopFrameProperty::kPropWidgetName,"canvas");
    children.push_back(widget);
    root->d_ptr->children = children;
    EXPECT_EQ(obj.findView(root),widget);
}

TEST(FrameManager, switchMode)
{
    stub_ext::StubExt stub;
    bool call = false;
    typedef bool(*fun_type)(CollectionModel*);
    stub.set_lamda((fun_type)&CustomMode::initialize,[&call](CollectionModel *){call = true; return true;});

    FrameManager obj(nullptr);
    obj.d->organizer = new test_CanvasOrganizer();
    CanvasInterface *canvas = new CanvasInterface();
    obj.d->canvas = canvas;
    canvas->d->canvaModel = new CanvasModelShell();
    canvas->d->canvasView = new CanvasViewShell();
    canvas->d->canvasGrid = new CanvasGridShell();
    canvas->d->canvasManager = new CanvasManagerShell();
    obj.switchMode(OrganizerMode::kCustom);
    EXPECT_TRUE(call);
}

TEST(FrameManagerPrivate, enableChanged)
{
    stub_ext::StubExt stub;
    bool callon = false;
    stub.set_lamda(&FrameManager::turnOn,[&callon](){callon = true;});
    bool calloff = false;
    stub.set_lamda(&FrameManager::turnOff,[&calloff](){calloff = true;});
    FrameManagerPrivate obj(nullptr);
    obj.q = new FrameManager();
    obj.enableChanged(true);
    EXPECT_TRUE(callon);

    CfgPresenter->enable = true;
    obj.enableChanged(false);
    EXPECT_TRUE(calloff);
}
