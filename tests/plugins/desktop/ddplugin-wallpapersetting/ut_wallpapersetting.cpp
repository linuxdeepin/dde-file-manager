// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallpapersettings.h"
#include "private/wallpapersettings_p.h"
#include "wallaperpreview.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"
#include "desktoputils/widgetutil.h"
#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-framework/dpf.h>
#include "wlsetplugin.h"

#include <QRect>
#include <QBoxLayout>

#include "stubext.h"
#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DDP_WALLPAERSETTING_USE_NAMESPACE

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

    WallpaperSettings *setting = new WallpaperSettings("testSetting", WallpaperSettings::Mode::ScreenSaverMode);
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

TEST(WallpaperSettingsPrivate, initUi)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WallpaperSettings::init, []() {
        return;
    });
    WallpaperSettings *setting = new WallpaperSettings("testSetting", WallpaperSettings::Mode::ScreenSaverMode);

    bool initCloseButton = false;
    bool initCarousel = false;
    bool initScreensaver = false;
    bool relayout = false;

    stub.set_lamda(&WallpaperSettingsPrivate::initCloseButton, [&initCloseButton]() {
        initCloseButton = true;
        return;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::initCarousel, [&initCarousel]() {
        initCarousel = true;
        return;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::initScreenSaver, [&initScreensaver]() {
        initScreensaver = true;
        return;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::relaylout, [&relayout]() {
        relayout = true;
        return;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::initPreivew, []() {
        return;
    });

    bool connect1 = false;
    bool connect2 = false;
    bool connect3 = false;

    stub.set_lamda(&WallpaperSettingsPrivate::handleNeedCloseButton, [&connect1]() {
        connect1 = true;
        return;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::onItemPressed, [&connect2]() {
        connect2 = true;
        return;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::setMode, [&connect3]() {
        connect3 = true;
        return;
    });

    stub.set_lamda(&ddplugin_desktop_util::enableScreensaver, []() {
        return true;
    });

    setting->d->initUI();
    emit setting->d->wallpaperList->mouseOverItemChanged("", QPoint());
    emit setting->d->wallpaperList->itemPressed("");
    emit setting->d->switchModeControl->buttonToggled(new DButtonBoxButton("Wallpaper", setting), true);

    EXPECT_TRUE(initCloseButton);
    EXPECT_TRUE(initCarousel);
    EXPECT_TRUE(initScreensaver);

    EXPECT_TRUE(connect1);
    EXPECT_TRUE(connect2);
    EXPECT_TRUE(connect3);

    EXPECT_FALSE(setting->d->switchModeControl->buttonList().at(0)->isChecked());
    EXPECT_TRUE(setting->d->switchModeControl->buttonList().at(1)->isChecked());

    delete setting;

    setting = new WallpaperSettings("testSetting", WallpaperSettings::Mode::WallpaperMode);
    setting->d->initUI();

    EXPECT_TRUE(setting->d->switchModeControl->buttonList().at(0)->isChecked());
    EXPECT_FALSE(setting->d->switchModeControl->buttonList().at(1)->isChecked());
    delete setting;
}

TEST(WallpaperSettingsPrivate, initCloseButton)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WallpaperSettings::init, []() {
        return;
    });
    WallpaperSettings *setting = new WallpaperSettings("testSetting", WallpaperSettings::Mode::ScreenSaverMode);

    bool connect = false;
    stub.set_lamda(&WallpaperSettingsPrivate::onCloseButtonClicked, [&connect]() {
        connect = true;
        return;
    });

    EXPECT_NO_FATAL_FAILURE(setting->d->initCloseButton());
    EXPECT_TRUE(setting->d->closeButton->isHidden());
    setting->d->closeButton->click();
    EXPECT_TRUE(connect);
    delete setting;
}

TEST(WallpaperSettingsPrivate, initCarousel)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WallpaperSettings::init, []() {
        return;
    });
    WallpaperSettings *setting = new WallpaperSettings("testSetting", WallpaperSettings::Mode::ScreenSaverMode);

    bool connect1 = false;
    bool connect2 = false;
    stub.set_lamda(&WallpaperSettingsPrivate::carouselTurn, [&connect1]() {
        connect1 = true;
        return;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::switchCarousel, [&connect2]() {
        connect2 = true;
        return;
    });
    EXPECT_NO_FATAL_FAILURE(setting->d->initCarousel());
    EXPECT_TRUE(setting->d->carouselCheckBox);
    EXPECT_TRUE(setting->d->carouselControl);
    emit setting->d->carouselCheckBox->click();
    emit setting->d->carouselControl->buttonToggled(new DButtonBoxButton("Wallpaper", setting), true);

    EXPECT_TRUE(connect1);
    EXPECT_TRUE(connect2);

    delete setting;
}

TEST(WallpaperSettingsPrivate, initScreenSaver)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WallpaperSettings::init, []() {
        return;
    });
    WallpaperSettings *setting = new WallpaperSettings("testSetting", WallpaperSettings::Mode::ScreenSaverMode);

    bool connect1 = false;
    bool connect2 = false;
    stub.set_lamda(&WallpaperSettingsPrivate::switchWaitTime, [&connect1]() {
        connect1 = true;
    });
    stub.set_lamda(&ScreenSaverIfs::setLockScreenAtAwake, [&connect2]() {
        __DBG_STUB_INVOKE__
        connect2 = true;
    });

    EXPECT_NO_FATAL_FAILURE(setting->d->initScreenSaver());
    EXPECT_TRUE(setting->d->lockScreenBox);
    EXPECT_TRUE(setting->d->waitControl);
    emit setting->d->waitControl->buttonToggled(new DButtonBoxButton("Wallpaper", setting), true);
    emit setting->d->lockScreenBox->toggled(true);

    EXPECT_TRUE(connect1);
    EXPECT_TRUE(connect2);

    delete setting;
}

TEST(WallpaperSettingsPrivate, initpreview)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WallpaperSettings::init, []() {
        return;
    });
    WallpaperSettings *setting = new WallpaperSettings("testSetting", WallpaperSettings::Mode::ScreenSaverMode);

    bool init = false;
    stub.set_lamda(&WallaperPreview::init, [&init]() {
        init = true;
        return;
    });

    setting->d->initPreivew();
    EXPECT_TRUE(init);
    delete setting;
}

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

TEST_F(UT_wallPaperSettings, privateEventFilter)
{
    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::KeyboardModifier::NoModifier);

    bool emit1 = false;
    stub.set_lamda(&WallpaperSettingsPrivate::carouselTurn, [&emit1]() {
        emit1 = true;
        return;
    });

    setting->d->eventFilter(setting->d->carouselCheckBox, event);
    EXPECT_TRUE(emit1);

    emit1 = false;
    event->k = Qt::Key_Return;
    setting->d->eventFilter(setting->d->carouselCheckBox, event);
    EXPECT_TRUE(emit1);

    emit1 = false;
    event->k = Qt::Key_9;
    setting->d->eventFilter(setting->d->carouselCheckBox, event);
    EXPECT_FALSE(emit1);
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

TEST_F(UT_wallPaperSettings, loadScreensaver)
{
    stub.set_lamda(&ComDeepinScreenSaverInterface::ConfigurableItems, []() {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<QStringList>();
    });

    QStringList list;
    list << "screenSaver1"
         << "screenSaver2"
         << "flurry";
    stub.set_lamda(&ComDeepinScreenSaverInterface::allScreenSaver, [&list]() {
        __DBG_STUB_INVOKE__
        return list;
    });

    bool closeLoading = false;
    stub.set_lamda(&WallpaperSettings::closeLoading, [&closeLoading]() {
        closeLoading = true;
    });

    stub.set_lamda(&ComDeepinScreenSaverInterface::GetScreenSaverCover, []() {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<QString>();
    });
    QString currentItem = "screenSaver1";
    stub.set_lamda(&ComDeepinScreenSaverInterface::currentScreenSaver, [&currentItem]() {
        __DBG_STUB_INVOKE__
        return currentItem;
    });

    stub.set_lamda(&WallpaperList::updateItemThumb, []() {
        return;
    });
    bool setindex = false;
    stub.set_lamda(&WallpaperList::setCurrentIndex, [&setindex]() {
        setindex = true;
        return;
    });
    bool emit1 = false;
    stub.set_lamda(&WallpaperItem::pressed, [&emit1]() {
        emit1 = true;
        return;
    });

    bool connect1 = false;
    bool connect2 = false;
    bool connect3 = false;
    stub.set_lamda(&WallpaperSettingsPrivate::onItemButtonClicked, [&connect1]() {
        connect1 = true;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::onItemTab, [&connect2]() {
        connect2 = true;
    });
    stub.set_lamda(&WallpaperSettingsPrivate::onItemBacktab, [&connect3]() {
        connect3 = true;
    });

    {
        EXPECT_TRUE(setting->d->screenSaverIfs->isValid());
        setting->loadScreenSaver();
        EXPECT_TRUE(closeLoading);
        EXPECT_EQ(setting->d->wallpaperList->items.size(), 2);
        EXPECT_TRUE(emit1);
        EXPECT_FALSE(setindex);

        emit setting->d->wallpaperList->items.at(0)->buttonClicked(nullptr, QString(""));
        emit setting->d->wallpaperList->items.at(0)->tab(nullptr);
        emit setting->d->wallpaperList->items.at(0)->backtab(nullptr);
        EXPECT_TRUE(connect1);
        EXPECT_TRUE(connect2);
        EXPECT_TRUE(connect3);
    }
    {
        currentItem = "123";
        emit1 = false;
        setindex = false;

        setting->loadScreenSaver();
        EXPECT_FALSE(emit1);
        EXPECT_TRUE(setindex);
    }
    {
        closeLoading = false;
        setindex = false;
        stub.set_lamda(&QDBusAbstractInterface::isValid, []() {
            return false;
        });
        list.clear();
        setting->loadScreenSaver();
        EXPECT_FALSE(closeLoading);
        EXPECT_FALSE(setindex);
    }
}

TEST_F(UT_wallPaperSettings, applyToDesktop)
{
    setting->d->screenName = "test1";
    setting->d->currentSelectedWallpaper = "test2";

    bool call = false;
    QList<QVariant> input;
    stub.set_lamda(
            &QDBusAbstractInterface::asyncCallWithArgumentList,
            [&call, &input](QDBusAbstractInterface *&, const QString &method, const QList<QVariant> &args) {
                if (method == "SetMonitorBackground") {
                    call = true;
                    input = args;
                }
                return QDBusPendingCall::fromError(QDBusError());
            });
    bool emited = false;
    QObject::connect(setting, &WallpaperSettings::backgroundChanged, [&emited]() {
        emited = true;
    });
    {

        setting->applyToDesktop();
        EXPECT_EQ(input.at(0).value<QString>(), QString("test1"));
        EXPECT_EQ(input.at(1).value<QString>(), QString("test2"));
        EXPECT_TRUE(call);
        EXPECT_TRUE(emited);
    }
    {
        call = false;
        emited = false;
        input.clear();
        setting->d->currentSelectedWallpaper = "";
        setting->applyToDesktop();
        EXPECT_FALSE(call);
        EXPECT_TRUE(input.isEmpty());
        EXPECT_FALSE(emited);
    }
    {
        call = false;
        emited = false;
        input.clear();
        setting->d->currentSelectedWallpaper = "test1";
        setting->d->appearanceIfs->deleteLater();
        setting->d->appearanceIfs = nullptr;
        setting->applyToDesktop();
        EXPECT_TRUE(input.isEmpty());
        EXPECT_FALSE(call);
        EXPECT_FALSE(emited);
    }
}

TEST_F(UT_wallPaperSettings, applyToGreeter)
{
    setting->d->currentSelectedWallpaper = "test2";
    bool call = false;
    QList<QVariant> input;
    stub.set_lamda(
            &QDBusAbstractInterface::asyncCallWithArgumentList,
            [&call, &input](QDBusAbstractInterface *&, const QString &method, const QList<QVariant> &args) {
                if (method == "Set") {
                    call = true;
                    input = args;
                }
                return QDBusPendingCall::fromError(QDBusError());
            });

    {
        setting->applyToGreeter();
        EXPECT_EQ(input.at(0).value<QString>(), QString("greeterbackground"));
        EXPECT_EQ(input.at(1).value<QString>(), QString("test2"));
        EXPECT_TRUE(call);
    }
    {
        call = false;
        input.clear();
        setting->d->currentSelectedWallpaper = "";
        setting->applyToGreeter();
        EXPECT_FALSE(call);
        EXPECT_TRUE(input.isEmpty());
    }
    {
        call = false;
        input.clear();
        setting->d->currentSelectedWallpaper = "test1";
        setting->d->appearanceIfs->deleteLater();
        setting->d->appearanceIfs = nullptr;
        setting->applyToGreeter();
        EXPECT_TRUE(input.isEmpty());
        EXPECT_FALSE(call);
    }
}
