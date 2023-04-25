// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "frame/windowframe_p.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/screen/abstractscreen.h>

#include <dfm-framework/dpf.h>

#include "stubext.h"

#include <gtest/gtest.h>

DDPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

TEST(WindowFrame, init)
{
    WindowFrame *frame = new WindowFrame;

    const QStringList allSig = {
        "signal_ScreenProxy_ScreenChanged",
        "signal_ScreenProxy_DisplayModeChanged",
        "signal_ScreenProxy_ScreenGeometryChanged",
        "signal_ScreenProxy_ScreenAvailableGeometryChanged",
    };

    for (auto sig : allSig) {
        if (auto ptr = dpfSignalDispatcher->dispatcherMap.value(
                    EventConverter::convert("ddplugin_core", sig))) {
            ASSERT_TRUE(ptr->handlerList.isEmpty());
        }
    }

    EXPECT_TRUE(frame->init());

    for (auto sig : allSig) {
        if (auto ptr = dpfSignalDispatcher->dispatcherMap.value(
                    EventConverter::convert("ddplugin_core", sig))) {
            EXPECT_FALSE(ptr->handlerList.isEmpty());
        }
    }

    delete frame;

    for (auto sig : allSig) {
        if (auto ptr = dpfSignalDispatcher->dispatcherMap.value(
                    EventConverter::convert("ddplugin_core", sig))) {
            EXPECT_TRUE(ptr->handlerList.isEmpty());
        }
    }
}

class TestNullScreen : public AbstractScreen
{
public:
    virtual QString name() const { return scname; }
    QRect geometry() const { return QRect(0, 0, 200, 200); }
    QRect availableGeometry() const { return QRect(0, 0, 200, 180); }
    QRect handleGeometry() const { return QRect(0, 0, 250, 250); }
    QString scname;
};

TEST(WindowFrame, rootWindows)
{
    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "sc1";
    ScreenPointer sc1(tmp);
    tmp = new TestNullScreen;
    tmp->scname = "sc2";
    ScreenPointer sc2(tmp);
    QList<DFMBASE_NAMESPACE::ScreenPointer> rets { sc1, sc2 };
    stub_ext::StubExt stub;
    bool callflag = false;
    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&callflag, &rets](EventChannelManager *, const QString &t1, const QString &t2) {
                       if (t1 == "ddplugin_core" && t2 == "slot_ScreenProxy_LogicScreens") {
                           callflag = true;
                           return QVariant::fromValue(rets);
                       }

                       return QVariant();
                   });

    WindowFrame frame;
    frame.d->windows.insert("sc1", BaseWindowPointer(new BaseWindow));
    frame.d->windows.insert("sc2", BaseWindowPointer(new BaseWindow));
    frame.d->windows.insert("sc3", BaseWindowPointer(new BaseWindow));

    auto wins = frame.rootWindows();

    ASSERT_TRUE(callflag);
    ASSERT_EQ(wins.size(), 2);
    EXPECT_EQ(wins.first(), frame.d->windows.value("sc1").get());
    EXPECT_EQ(wins.last(), frame.d->windows.value("sc2").get());
}

TEST(WindowFrame, layoutChildren)
{
    WindowFrame frame;
    BaseWindowPointer bw(new BaseWindow);
    frame.d->windows.insert("sc1", bw);

    QWidget *l3 = new QWidget(bw.get());
    l3->setProperty(DesktopFrameProperty::kPropWidgetLevel, 3);

    QWidget *l1 = new QWidget(bw.get());
    l1->setProperty(DesktopFrameProperty::kPropWidgetLevel, 1);

    QWidget *l4 = new QWidget(bw.get());
    l4->setProperty(DesktopFrameProperty::kPropWidgetLevel, 4);

    QWidget *l2 = new QWidget(bw.get());
    l2->setProperty(DesktopFrameProperty::kPropWidgetLevel, 2);

    frame.layoutChildren();
    QList<QWidget *> subWidgets;
    for (QObject *obj : bw->children()) {
        if (QWidget *wid = qobject_cast<QWidget *>(obj)) {
            subWidgets.append(wid);
        }
    }

    ASSERT_EQ(subWidgets.size(), 4);
    EXPECT_EQ(subWidgets.at(0), l1);
    EXPECT_EQ(subWidgets.at(1), l2);
    EXPECT_EQ(subWidgets.at(2), l3);
    EXPECT_EQ(subWidgets.at(3), l4);
}

TEST(WindowFrame, buildBaseWindow)
{
    WindowFrame frame;
    int aboutTo = -1;
    frame.connect(&frame, &WindowFrame::windowAboutToBeBuilded,
                  &frame, [&aboutTo]() {
                      aboutTo = 0;
                  });

    int builded = -1;
    frame.connect(&frame, &WindowFrame::windowBuilded,
                  &frame, [&builded]() {
                      builded = 1;
                  });

    int sigshow = -1;
    frame.connect(&frame, &WindowFrame::windowShowed,
                  &frame, [&sigshow]() {
                      sigshow = 2;
                  });

    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "sc1";
    ScreenPointer sc1(tmp);
    tmp = new TestNullScreen;
    tmp->scname = "sc2";
    ScreenPointer sc2(tmp);

    QList<DFMBASE_NAMESPACE::ScreenPointer> rets { sc1, sc2 };
    int displayMode = kDuplicate;

    stub_ext::StubExt stub;
    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&rets, &displayMode, &sc1](EventChannelManager *, const QString &t1, const QString &t2) {
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_ScreenProxy_LogicScreens") {
                               return QVariant::fromValue(rets);
                           } else if (t2 == "slot_ScreenProxy_LastChangedMode") {
                               return QVariant::fromValue(displayMode);
                           } else if (t2 == "slot_ScreenProxy_PrimaryScreen") {
                               return QVariant::fromValue(sc1);
                           }
                       }

                       return QVariant();
                   });

    bool upp = false;
    stub.set_lamda(&WindowFramePrivate::updateProperty, [&upp]() {
        upp = true;
    });

    bool hide = false;
    stub.set_lamda(&QWidget::hide, [&hide]() {
        hide = true;
    });

    bool show = false;
    stub.set_lamda(&QWidget::show, [&show]() {
        show = true;
    });

    bool layout = false;
    stub.set_lamda(VADDR(WindowFrame, layoutChildren), [&layout]() {
        layout = true;
    });

    {
        frame.buildBaseWindow();
        EXPECT_EQ(aboutTo, 0);
        EXPECT_EQ(builded, 1);
        EXPECT_EQ(sigshow, 2);
        EXPECT_TRUE(hide);
        EXPECT_TRUE(show);
        EXPECT_TRUE(layout);
        EXPECT_TRUE(upp);

        EXPECT_EQ(frame.d->windows.size(), 1);
        auto win = frame.d->windows.value("sc1");
        EXPECT_NE(win.get(), nullptr);
        frame.d->windows.clear();
    }

    {
        aboutTo = -1;
        builded = -1;
        sigshow = -1;
        hide = false;
        show = false;
        layout = false;
        upp = false;

        displayMode = kExtend;
        frame.buildBaseWindow();
        EXPECT_EQ(frame.d->windows.size(), 2);
        auto win = frame.d->windows.value("sc1");
        EXPECT_NE(win.get(), nullptr);
        win = frame.d->windows.value("sc2");
        EXPECT_NE(win.get(), nullptr);
    }
}

TEST(WindowFrame, onGeometryChanged)
{
    WindowFrame frame;
    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "sc1";
    ScreenPointer sc1(tmp);
    QList<DFMBASE_NAMESPACE::ScreenPointer> rets { sc1 };

    stub_ext::StubExt stub;
    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&rets, &sc1](EventChannelManager *, const QString &t1, const QString &t2) {
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_ScreenProxy_LogicScreens") {
                               return QVariant::fromValue(rets);
                           } else if (t2 == "slot_ScreenProxy_PrimaryScreen") {
                               return QVariant::fromValue(sc1);
                           }
                       }

                       return QVariant();
                   });

    bool upp = false;
    stub.set_lamda(&WindowFramePrivate::updateProperty, [&upp]() {
        upp = true;
    });

    bool changed = false;
    stub.set_lamda(&WindowFrame::geometryChanged, [&changed]() {
        changed = true;
    });

    BaseWindowPointer bw(new BaseWindow);
    frame.d->windows.insert("sc1", bw);
    bw->setGeometry(QRect(0, 20, 20, 300));

    frame.onGeometryChanged();

    EXPECT_EQ(bw->geometry(), sc1->geometry());
    EXPECT_TRUE(changed);
    EXPECT_TRUE(upp);

    upp = false;
    changed = false;
    bw->setGeometry(sc1->geometry());
    frame.onGeometryChanged();
    EXPECT_FALSE(changed);
    EXPECT_FALSE(upp);
}

TEST(WindowFrame, onAvailableGeometryChanged)
{
    WindowFrame frame;
    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "sc1";
    ScreenPointer sc1(tmp);
    QList<DFMBASE_NAMESPACE::ScreenPointer> rets { sc1 };

    stub_ext::StubExt stub;
    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&rets, &sc1](EventChannelManager *, const QString &t1, const QString &t2) {
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_ScreenProxy_LogicScreens") {
                               return QVariant::fromValue(rets);
                           } else if (t2 == "slot_ScreenProxy_PrimaryScreen") {
                               return QVariant::fromValue(sc1);
                           }
                       }

                       return QVariant();
                   });

    bool upp = false;
    stub.set_lamda(&WindowFramePrivate::updateProperty, [&upp]() {
        upp = true;
    });

    bool changed = false;
    stub.set_lamda(&WindowFrame::availableGeometryChanged, [&changed]() {
        changed = true;
    });

    BaseWindowPointer bw(new BaseWindow);
    frame.d->windows.insert("sc1", bw);
    bw->setProperty(DesktopFrameProperty::kPropScreenAvailableGeometry,
                    sc1->geometry());

    frame.onAvailableGeometryChanged();

    EXPECT_TRUE(changed);
    EXPECT_TRUE(upp);

    upp = false;
    changed = false;
    bw->setProperty(DesktopFrameProperty::kPropScreenAvailableGeometry,
                    sc1->availableGeometry());
    frame.onAvailableGeometryChanged();
    EXPECT_FALSE(changed);
    EXPECT_FALSE(upp);
}

TEST(WindowFramePrivate, updateProperty)
{
    WindowFrame frame;
    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "sc1";
    ScreenPointer screen(tmp);

    BaseWindowPointer win(new BaseWindow);
    frame.d->windows.insert("sc1", win);

    frame.d->updateProperty(win, screen, true);

    EXPECT_EQ(win->property(DesktopFrameProperty::kPropScreenName).toString(), QString("sc1"));
    EXPECT_EQ(win->property(DesktopFrameProperty::kPropIsPrimary).toBool(), true);
    EXPECT_EQ(win->property(DesktopFrameProperty::kPropScreenGeometry).toRect(), screen->geometry());
    EXPECT_EQ(win->property(DesktopFrameProperty::kPropScreenAvailableGeometry).toRect(), screen->availableGeometry());
    EXPECT_EQ(win->property(DesktopFrameProperty::kPropScreenHandleGeometry).toRect(), screen->handleGeometry());
    EXPECT_EQ(win->property(DesktopFrameProperty::kPropWidgetName), QString("root"));
    EXPECT_EQ(win->property(DesktopFrameProperty::kPropWidgetLevel), QString("0.0"));
}
