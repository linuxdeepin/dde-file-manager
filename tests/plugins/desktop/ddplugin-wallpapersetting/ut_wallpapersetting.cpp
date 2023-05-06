// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallpapersettings.h"
#include "private/wallpapersettings_p.h"
#include "wallaperpreview.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"
#include <QRect>

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-framework/dpf.h>
#include <QBoxLayout>

#include "stubext.h"
#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DDP_WALLPAERSETTING_USE_NAMESPACE

class UT_wallPaperSettings : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        setting = new WallpaperSettings("testSetting");
    }
    virtual void TearDown() override
    {
        delete setting;
        stub.clear();
    }
    WallpaperSettings *setting = nullptr;
    stub_ext::StubExt stub;
};

TEST(wallPaperSettings, wallpapersettings)
{
    stub_ext::StubExt stub;
    bool init = false;
    stub.set_lamda(&WallpaperSettings::init, [&init]() {
        init = true;
        return;
    });
    WallpaperSettings *setting = new WallpaperSettings("testSetting");
    EXPECT_TRUE(init);
    EXPECT_EQ(setting->d->screenName, QString("testSetting"));
    EXPECT_EQ(setting->d->mode, WallpaperSettings::Mode::WallpaperMode);
    delete setting;

    setting = new WallpaperSettings("testSetting", WallpaperSettings::Mode::ScreenSaverMode);
    EXPECT_TRUE(init);
    EXPECT_EQ(setting->d->screenName, QString("testSetting"));
    EXPECT_EQ(setting->d->mode, WallpaperSettings::Mode::ScreenSaverMode);
    delete setting;
}

TEST(wallPaperSettings, Init)
{
    WallpaperSettings *setting = new WallpaperSettings("testSetting");
    stub_ext::StubExt stub;
    stub.set_lamda(&WallpaperSettingsPrivate::initUI, []() {
        return;
    });
    stub.set_lamda(&WallpaperSettings::adjustGeometry, []() {
        return;
    });

    const QStringList allSig = {
        "signal_ScreenProxy_ScreenChanged",
        "signal_ScreenProxy_DisplayModeChanged",
        "signal_ScreenProxy_ScreenGeometryChanged",
    };

    for (auto sig : allSig) {
        if (auto ptr = dpfSignalDispatcher->dispatcherMap.value(
                    EventConverter::convert("ddplugin_core", sig))) {
            __DBG_STUB_INVOKE__
            ASSERT_TRUE(ptr->handlerList.isEmpty());
        }
    }

    setting->init();
    WallpaperSettingsPrivate *d = setting->d;
    EXPECT_EQ(d->wmInter->service(), QString("com.deepin.wm"));
    EXPECT_TRUE(d->regionMonitor);
    EXPECT_EQ(d->appearanceIfs->service(), QString("com.deepin.daemon.Appearance"));
    EXPECT_EQ(d->screenSaverIfs->service(), QString("com.deepin.ScreenSaver"));
    EXPECT_TRUE(d->sessionIfs);
    for (auto sig : allSig) {
        if (auto ptr = dpfSignalDispatcher->dispatcherMap.value(
                    EventConverter::convert("ddplugin_core", sig))) {
            __DBG_STUB_INVOKE__
            EXPECT_FALSE(ptr->handlerList.isEmpty());
        }
    }

    delete setting;

    for (auto sig : allSig) {
        if (auto ptr = dpfSignalDispatcher->dispatcherMap.value(
                    EventConverter::convert("ddplugin_core", sig))) {
            __DBG_STUB_INVOKE__
            EXPECT_TRUE(ptr->handlerList.isEmpty());
        }
    }
}

TEST_F(UT_wallPaperSettings, timeFormat)
{
    int second = 356521;
    QString res;
    res = setting->d->timeFormat(second);
    EXPECT_EQ(res, QString("4d 3h 2m 1s"));

    second = 10925;
    res = setting->d->timeFormat(second);
    EXPECT_EQ(res, QString("3h 2m 5s"));

    second = 345725;
    res = setting->d->timeFormat(second);
    EXPECT_EQ(res, QString("4d 2m 5s"));
}

TEST_F(UT_wallPaperSettings, relayout)
{
    stub.set_lamda(&QWidget::show, []() {
        __DBG_STUB_INVOKE__
        return;
    });

    stub.set_lamda(&QWidget::hide, []() {
        __DBG_STUB_INVOKE__
        return;
    });

    setting->d->relaylout();
    EXPECT_EQ(static_cast<QBoxLayout *>(setting->layout())->itemAt(0), setting->d->carouselLayout);

    setting->d->mode = WallpaperSettings::Mode::ScreenSaverMode;
    setting->d->relaylout();
    EXPECT_EQ(static_cast<QBoxLayout *>(setting->layout())->itemAt(0), setting->d->toolLayout);
}

TEST_F(UT_wallPaperSettings, handleNeedCloseButton)
{
    QString itemData = "testData";
    setting->d->handleNeedCloseButton(itemData, QPoint(0, 0));
    EXPECT_EQ(setting->d->closeButton->property("background"), itemData);
    EXPECT_FALSE(setting->d->closeButton->isHidden());

    itemData = "";
    setting->d->handleNeedCloseButton(itemData, QPoint(0, 0));
    EXPECT_EQ(setting->d->closeButton->property("background"), itemData);
    EXPECT_TRUE(setting->d->closeButton->isHidden());
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

TEST_F(UT_wallPaperSettings, onScreenChanged)
{
    stub.set_lamda(&WallaperPreview::buildWidgets, []() {
        return;
    });
    stub.set_lamda(&WallaperPreview::updateWallpaper, []() {
        return;
    });
    stub.set_lamda(&WallaperPreview::setVisible, []() {
        return;
    });
    stub.set_lamda(&WallaperPreview::widget, []() {
        __DBG_STUB_INVOKE__
        return PreviewWidgetPtr(new BackgroundPreview("test"));
    });

    bool call = false;
    stub.set_lamda(&WallpaperSettings::onGeometryChanged, [&call]() {
        call = true;
        return;
    });
    setting->d->onScreenChanged();
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(&WallaperPreview::widget, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });
    setting->d->onScreenChanged();
    EXPECT_FALSE(call);
}

TEST_F(UT_wallPaperSettings, switchMode)
{
    setting->d->mode = WallpaperSettings::Mode::WallpaperMode;
    bool calladjust = false;
    bool callrefresh = false;
    bool callrelay = false;

    stub.set_lamda(&WallpaperSettings::adjustGeometry, [&calladjust]() {
        calladjust = true;
        return;
    });
    stub.set_lamda(&WallpaperSettings::refreshList, [&callrefresh]() {
        callrefresh = true;
        return;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::relaylout, [&callrelay]() {
        callrelay = true;
        return;
    });

    setting->switchMode(WallpaperSettings::Mode::ScreenSaverMode);
    EXPECT_EQ(setting->d->mode, WallpaperSettings::Mode::ScreenSaverMode);
    EXPECT_TRUE(calladjust);
    EXPECT_TRUE(callrefresh);
    EXPECT_TRUE(callrelay);
}

TEST_F(UT_wallPaperSettings, loadingLabel)
{
    bool start = false;
    stub.set_lamda(&LoadingLabel::start, [&start]() {
        start = true;
    });
    stub.set_lamda(&WallpaperList::setMaskWidget, []() {
        return;
    });
    setting->showLoading();
    EXPECT_TRUE(start);
    EXPECT_EQ(setting->d->loadingLabel->contant->text(), QString("Loading wallpapers..."));

    setting->d->mode = WallpaperSettings::Mode::ScreenSaverMode;
    setting->showLoading();
    EXPECT_EQ(setting->d->loadingLabel->contant->text(), QString("Loading screensavers..."));

    bool remove = false;
    stub.set_lamda(&WallpaperList::removeMaskWidget, [&remove]() {
        remove = true;
        return nullptr;
    });
    setting->closeLoading();
    EXPECT_EQ(setting->d->loadingLabel, nullptr);
    EXPECT_TRUE(remove);
}

TEST_F(UT_wallPaperSettings, wallpaperSlideShow)
{
    QMap<QString, QString> wallpaperSlideshow;
    QString res;
    stub.set_lamda(
            &QDBusAbstractInterface::asyncCallWithArgumentList,
            [&wallpaperSlideshow, &res](QDBusAbstractInterface *&, const QString &method, const QList<QVariant> &args) {
                if (method == "SetWallpaperSlideShow")
                    wallpaperSlideshow.insert(args.at(0).value<QString>(), args.at(1).value<QString>());
                if (method == "GetWallpaperSlideShow")
                    res = wallpaperSlideshow.value(args.at(0).value<QString>());
                return QDBusPendingCall::fromError(QDBusError());
            });

    setting->d->screenName = "testScName";
    setting->setWallpaperSlideShow("testPeriod");
    EXPECT_EQ(wallpaperSlideshow.value(setting->d->screenName), QString("testPeriod"));

    setting->wallpaperSlideShow();
    EXPECT_EQ(res, QString("testPeriod"));
}

TEST_F(UT_wallPaperSettings, refresh)
{
    bool showLoading = false;
    stub.set_lamda(&WallpaperSettings::showLoading, [&showLoading]() {
        showLoading = true;
        return;
    });
    bool loadWallpaper = false;
    bool loadScrennSaver = false;
    stub.set_lamda(&WallpaperSettings::loadWallpaper, [&loadWallpaper]() {
        loadWallpaper = true;
        return;
    });
    stub.set_lamda(&WallpaperSettings::loadScreenSaver, [&loadScrennSaver]() {
        loadScrennSaver = true;
        return;
    });
    stub.set_lamda(&WallpaperSettings::isVisible, []() {
        return true;
    });
    {
        setting->d->mode = WallpaperSettings::Mode::WallpaperMode;
        setting->refreshList();
        EXPECT_TRUE(showLoading);
        EXPECT_TRUE(loadWallpaper);
        EXPECT_FALSE(loadScrennSaver);
    }
    {
        loadWallpaper = false;
        setting->d->mode = WallpaperSettings::Mode::ScreenSaverMode;
        setting->refreshList();
        EXPECT_FALSE(loadWallpaper);
        EXPECT_TRUE(loadScrennSaver);
    }
}
