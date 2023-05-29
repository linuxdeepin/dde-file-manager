// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallaperpreview.h"
#include "backgroundpreview.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-framework/dpf.h>

#include "stubext.h"
#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DDP_WALLPAERSETTING_USE_NAMESPACE

class UT_wallPaperPreview : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        preview = new WallaperPreview;
    }
    virtual void TearDown() override
    {
        delete preview;
        stub.clear();
    }
    WallaperPreview *preview = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_wallPaperPreview, setVisible)
{
    preview->setVisible(true);
    EXPECT_TRUE(preview->visible);

    preview->setVisible(false);
    EXPECT_FALSE(preview->visible);
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

TEST_F(UT_wallPaperPreview, pullImageSettings)
{
    preview->wallpaper().clear();
    EXPECT_TRUE(preview->wallpaper().isEmpty());

    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "test";
    ScreenPointer sc1(tmp);
    QList<DFMBASE_NAMESPACE::ScreenPointer> rets { sc1 };

    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&rets](EventChannelManager *, const QString &t1, const QString &t2) {
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_ScreenProxy_LogicScreens") {
                               return QVariant::fromValue(rets);
                           }
                       }

                       return QVariant();
                   });
    stub.set_lamda(&WallaperPreview::getBackground, []() {
        return QString(":/test");
    });

    preview->pullImageSettings();
    EXPECT_EQ(preview->wallpaper().value("test"), QString(":/test"));
}

TEST_F(UT_wallPaperPreview, updateWallpaper)
{
    BackgroundPreview *bgp = new BackgroundPreview("bpg1");
    PreviewWidgetPtr ptr(bgp);

    preview->previewWidgets.insert("test", ptr);
    EXPECT_TRUE(preview->wallpaper().isEmpty());

    stub.set_lamda(&WallaperPreview::getBackground, [](WallaperPreview *, const QString &in) {
        if (in == "test")
            return ":/test2";
        return "";
    });
    preview->updateWallpaper();
    EXPECT_EQ(preview->wallpapers.value("test"), QString(":/test2"));
    EXPECT_EQ(ptr->filePath, QString(":/test2"));

    preview->wallpapers.clear();
    preview->wallpapers.insert("test", ":/test");
    preview->updateWallpaper();
    EXPECT_EQ(preview->wallpapers.value("test"), QString(":/test"));
    EXPECT_EQ(ptr->filePath, QString(":/test"));
}

TEST_F(UT_wallPaperPreview, setWallPaper)
{
    EXPECT_TRUE(preview->wallpapers.isEmpty());
    bool call = false;
    stub.set_lamda(&WallaperPreview::updateWallpaper, [&call]() {
        call = true;
    });

    preview->setWallpaper("screen", "image");
    EXPECT_EQ(preview->wallpapers.value("screen"), QString("image"));
}

TEST_F(UT_wallPaperPreview, updateGeometry)
{
    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "testsc1";
    ScreenPointer sc1(tmp);
    tmp = new TestNullScreen;
    tmp->scname = "testsc2";
    ScreenPointer sc2(tmp);
    QList<ScreenPointer> scs { sc1, sc2 };

    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&scs](EventChannelManager *, const QString &t1, const QString &t2) {
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_ScreenProxy_Screens") {
                               return QVariant::fromValue(scs);
                           }
                       }
                       return QVariant();
                   });

    PreviewWidgetPtr pw1(new BackgroundPreview("pw1"));
    pw1->setGeometry(0, 0, 200, 200);
    PreviewWidgetPtr pw2(new BackgroundPreview("pw2"));
    pw2->setGeometry(0, 0, 100, 100);
    preview->previewWidgets.insert("testsc1", pw1);
    preview->previewWidgets.insert("testsc2", pw2);

    quint8 callupdatetimes = 0;
    stub.set_lamda(&BackgroundPreview::updateDisplay, [&callupdatetimes]() {
        __DBG_STUB_INVOKE__
        callupdatetimes++;
        return ; });

    preview->updateGeometry();
    EXPECT_EQ(pw2->geometry(), QRect(0, 0, 200, 200));
    EXPECT_EQ(callupdatetimes, 1);
}

TEST_F(UT_wallPaperPreview, createWidgets)
{
    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "testsc";
    ScreenPointer sc(tmp);

    PreviewWidgetPtr res = preview->createWidget(sc);
    EXPECT_TRUE(res->property("isPreview").value<bool>());
    EXPECT_EQ(res->property("myScreen"), QVariant::fromValue(QString("testsc")));
    EXPECT_EQ(res->geometry(), QRect(0, 0, 200, 200));
}

TEST_F(UT_wallPaperPreview, buildWidgets)
{
    int lastChangeMode = 3;
    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "testsc1";
    ScreenPointer sc1(tmp);
    QList<DFMBASE_NAMESPACE::ScreenPointer> rets2 { sc1 };

    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&sc1, &rets2, &lastChangeMode](EventChannelManager *, const QString &t1, const QString &t2) {
                       __DBG_STUB_INVOKE__
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_ScreenProxy_PrimaryScreen") {
                               return QVariant::fromValue(sc1);
                           } else if (t2 == "slot_ScreenProxy_LastChangedMode") {
                               return QVariant::fromValue(lastChangeMode);
                           } else if (t2 == "slot_ScreenProxy_LogicScreens") {
                               return QVariant::fromValue(rets2);
                           }
                       }
                       return QVariant();
                   });
    {
        PreviewWidgetPtr wid(new BackgroundPreview(sc1->name()));
        preview->previewWidgets.insert("testsc1", wid);
        preview->buildWidgets();
        EXPECT_EQ(preview->previewWidgets.value("testsc1"), wid);
        EXPECT_EQ(wid->geometry(), QRect(0, 0, 200, 200));
    }
    {
        lastChangeMode = 2;
        PreviewWidgetPtr wid1(new BackgroundPreview(sc1->name()));
        preview->previewWidgets.insert("testsc1", wid1);
        tmp = new TestNullScreen;
        tmp->scname = "testsc2";
        ScreenPointer sc2(tmp);
        rets2.append(sc2);
        PreviewWidgetPtr wid2(new BackgroundPreview(sc2->name()));
        preview->previewWidgets.insert("testsc2", wid2);

        preview->buildWidgets();
        EXPECT_EQ(preview->previewWidgets.value("testsc1")->geometry(), QRect(0, 0, 200, 200));
        EXPECT_EQ(preview->previewWidgets.value("testsc2")->geometry(), QRect(0, 0, 200, 200));
    }
    {
        rets2.clear();
        sc1 = nullptr;
        rets2.append(sc1);
        preview->buildWidgets();
        EXPECT_TRUE(preview->previewWidgets.isEmpty());
    }
}
