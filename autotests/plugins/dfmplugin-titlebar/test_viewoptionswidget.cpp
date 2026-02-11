// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/viewoptionswidget.h"
#include "views/private/viewoptionswidget_p.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/viewdefines.h>

#include <DFontSizeManager>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QCheckBox>
#include <QApplication>
#include <QScreen>
#include <QToolTip>
#include <QEventLoop>
#include <QHideEvent>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class ViewOptionsWidgetTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub DConfigManager
        stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &defaultValue) {
            __DBG_STUB_INVOKE__
            return defaultValue;
        });

        // Stub Application methods
        stub.set_lamda(&Application::appAttribute, [](Application::ApplicationAttribute attr) {
            __DBG_STUB_INVOKE__
            if (attr == Application::kIconSizeLevel)
                return QVariant(2);
            if (attr == Application::kGridDensityLevel)
                return QVariant(1);
            if (attr == Application::kListHeightLevel)
                return QVariant(1);
            return QVariant();
        });

        // Stub Settings::sync
        stub.set_lamda(&Settings::sync, [] {
            __DBG_STUB_INVOKE__
            return true;
        });

        // Stub TitleBarHelper methods
        stub.set_lamda(&TitleBarHelper::getFileViewStateValue, [](const QUrl &, const QString &, const QVariant &defaultValue) {
            __DBG_STUB_INVOKE__
            return defaultValue;
        });

        stub.set_lamda(&TitleBarHelper::setFileViewStateValue, [](const QUrl &, const QString &, const QVariant &) {
            __DBG_STUB_INVOKE__
        });

        // Stub icon loading
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });

        // Stub DFontSizeManager
        typedef void (DFontSizeManager::*Bind)(QWidget *, DFontSizeManager::SizeType, int);
        stub.set_lamda(static_cast<Bind>(&DFontSizeManager::bind), [](DFontSizeManager *, QWidget *, DFontSizeManager::SizeType, int) {
            __DBG_STUB_INVOKE__
        });

        // Stub DGuiApplicationHelper
        stub.set_lamda(&DGuiApplicationHelper::themeType, [](DGuiApplicationHelper *) {
            __DBG_STUB_INVOKE__
            return DGuiApplicationHelper::LightType;
        });

        // Stub DPaletteHelper
        stub.set_lamda(&DPaletteHelper::palette, [] {
            __DBG_STUB_INVOKE__
            return DPalette();
        });

        stub.set_lamda(&ViewOptionsWidget::show, [] { __DBG_STUB_INVOKE__ });

        widget = new ViewOptionsWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    ViewOptionsWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(ViewOptionsWidgetTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(widget, nullptr);
    EXPECT_NE(widget->d, nullptr);
}

TEST_F(ViewOptionsWidgetTest, Constructor_InitializesPrivate_AllWidgetsCreated)
{
    EXPECT_NE(widget->d->title, nullptr);
    EXPECT_NE(widget->d->iconSizeFrame, nullptr);
    EXPECT_NE(widget->d->iconSizeSlider, nullptr);
    EXPECT_NE(widget->d->gridDensityFrame, nullptr);
    EXPECT_NE(widget->d->gridDensitySlider, nullptr);
    EXPECT_NE(widget->d->listHeightFrame, nullptr);
    EXPECT_NE(widget->d->listHeightSlider, nullptr);
    EXPECT_NE(widget->d->displayPreviewWidget, nullptr);
    EXPECT_NE(widget->d->displayPreviewCheckBox, nullptr);
}

TEST_F(ViewOptionsWidgetTest, Constructor_WindowFlag_IsPopup)
{
    EXPECT_TRUE(widget->testAttribute(Qt::WA_X11NetWmWindowTypePopupMenu) || widget->windowFlags().testFlag(Qt::Popup));
}

TEST_F(ViewOptionsWidgetTest, Constructor_BlurEffect_Enabled)
{
    EXPECT_TRUE(widget->blurEnabled());
    EXPECT_EQ(widget->mode(), DBlurEffectWidget::GaussianBlur);
}

TEST_F(ViewOptionsWidgetTest, InitializeUi_IconSizeSlider_ConfiguredCorrectly)
{
    auto slider = widget->d->iconSizeSlider;
    EXPECT_NE(slider, nullptr);
    EXPECT_EQ(slider->minimum(), 0);
    EXPECT_GT(slider->maximum(), 0);
}

TEST_F(ViewOptionsWidgetTest, InitializeUi_GridDensitySlider_ConfiguredCorrectly)
{
    auto slider = widget->d->gridDensitySlider;
    EXPECT_NE(slider, nullptr);
    EXPECT_EQ(slider->minimum(), 0);
    EXPECT_GT(slider->maximum(), 0);
}

TEST_F(ViewOptionsWidgetTest, InitializeUi_ListHeightSlider_ConfiguredCorrectly)
{
    auto slider = widget->d->listHeightSlider;
    EXPECT_NE(slider, nullptr);
    EXPECT_EQ(slider->minimum(), 0);
    EXPECT_GT(slider->maximum(), 0);
}

TEST_F(ViewOptionsWidgetTest, InitializeUi_DisplayPreviewCheckBox_ConfiguredCorrectly)
{
    EXPECT_NE(widget->d->displayPreviewCheckBox, nullptr);
}

TEST_F(ViewOptionsWidgetTest, SetUrl_ValidUrl_UpdatesSlidersCorrectly)
{
    QUrl testUrl("file:///home/test");
    widget->d->setUrl(testUrl);
    EXPECT_EQ(widget->d->fileUrl, testUrl);
}

TEST_F(ViewOptionsWidgetTest, SetUrl_LoadsIconSizeLevel_FromStateValue)
{
    int expectedValue = 3;
    stub.set_lamda(&TitleBarHelper::getFileViewStateValue, [expectedValue](const QUrl &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "iconSizeLevel")
            return QVariant(expectedValue);
        return QVariant(1);
    });

    QUrl testUrl("file:///home/test");
    widget->d->setUrl(testUrl);
    EXPECT_EQ(widget->d->iconSizeSlider->value(), expectedValue);
}

TEST_F(ViewOptionsWidgetTest, SetUrl_LoadsGridDensityLevel_FromStateValue)
{
    int expectedValue = 2;
    stub.set_lamda(&TitleBarHelper::getFileViewStateValue, [expectedValue](const QUrl &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "gridDensityLevel")
            return QVariant(expectedValue);
        return QVariant(1);
    });

    QUrl testUrl("file:///home/test");
    widget->d->setUrl(testUrl);
    EXPECT_EQ(widget->d->gridDensitySlider->value(), expectedValue);
}

TEST_F(ViewOptionsWidgetTest, SetUrl_LoadsListHeightLevel_FromStateValue)
{
    int expectedValue = 2;
    stub.set_lamda(&TitleBarHelper::getFileViewStateValue, [expectedValue](const QUrl &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "listHeightLevel")
            return QVariant(expectedValue);
        return QVariant(1);
    });

    QUrl testUrl("file:///home/test");
    widget->d->setUrl(testUrl);
    EXPECT_EQ(widget->d->listHeightSlider->value(), expectedValue);
}

TEST_F(ViewOptionsWidgetTest, SwitchMode_IconMode_ShowsCorrectFrames)
{
    EXPECT_NO_THROW(widget->d->switchMode(ViewMode::kIconMode));
}

TEST_F(ViewOptionsWidgetTest, SwitchMode_ListMode_ShowsCorrectFrames)
{
    EXPECT_NO_THROW(widget->d->switchMode(ViewMode::kListMode));
}

TEST_F(ViewOptionsWidgetTest, SwitchMode_TreeMode_ShowsCorrectFrames)
{
    EXPECT_NO_THROW(widget->d->switchMode(ViewMode::kTreeMode));
}

TEST_F(ViewOptionsWidgetTest, IconSizeSlider_ValueChanged_SavesState)
{
    bool stateSaved = false;
    QVariant savedValue;
    QString savedKey;

    stub.set_lamda(&TitleBarHelper::setFileViewStateValue, [&](const QUrl &, const QString &key, const QVariant &value) {
        __DBG_STUB_INVOKE__
        stateSaved = true;
        savedKey = key;
        savedValue = value;
    });

    QUrl testUrl("file:///home/test");
    widget->d->fileUrl = testUrl;
    widget->d->iconSizeSlider->setValue(3);

    EXPECT_TRUE(stateSaved);
    EXPECT_EQ(savedKey, "iconSizeLevel");
    EXPECT_EQ(savedValue.toInt(), 3);
}

TEST_F(ViewOptionsWidgetTest, GridDensitySlider_ValueChanged_SavesState)
{
    bool stateSaved = false;
    QVariant savedValue;
    QString savedKey;

    stub.set_lamda(&TitleBarHelper::setFileViewStateValue, [&](const QUrl &, const QString &key, const QVariant &value) {
        __DBG_STUB_INVOKE__
        stateSaved = true;
        savedKey = key;
        savedValue = value;
    });

    QUrl testUrl("file:///home/test");
    widget->d->fileUrl = testUrl;
    widget->d->gridDensitySlider->setValue(2);

    EXPECT_TRUE(stateSaved);
    EXPECT_EQ(savedKey, "gridDensityLevel");
    EXPECT_EQ(savedValue.toInt(), 2);
}

TEST_F(ViewOptionsWidgetTest, ListHeightSlider_ValueChanged_SavesState)
{
    bool stateSaved = false;
    QVariant savedValue;
    QString savedKey;

    stub.set_lamda(&TitleBarHelper::setFileViewStateValue, [&](const QUrl &, const QString &key, const QVariant &value) {
        __DBG_STUB_INVOKE__
        stateSaved = true;
        savedKey = key;
        savedValue = value;
    });

    QUrl testUrl("file:///home/test");
    widget->d->fileUrl = testUrl;
    widget->d->listHeightSlider->setValue(2);

    EXPECT_TRUE(stateSaved);
    EXPECT_EQ(savedKey, "listHeightLevel");
    EXPECT_EQ(savedValue.toInt(), 2);
}

TEST_F(ViewOptionsWidgetTest, DisplayPreviewCheckBox_StateChanged_EmitsSignal)
{
    QSignalSpy spy(widget, &ViewOptionsWidget::displayPreviewVisibleChanged);

    widget->d->displayPreviewCheckBox->setChecked(true);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.at(0).at(0).toBool());
}

TEST_F(ViewOptionsWidgetTest, DisplayPreviewCheckBox_StateChanged_SavesConfig)
{
    bool configSaved = false;
    bool savedValue = false;

    stub.set_lamda(static_cast<void (DConfigManager::*)(const QString &, const QString &, const QVariant &)>(&DConfigManager::setValue),
                   [&](DConfigManager *, const QString &, const QString &, const QVariant &value) {
                       __DBG_STUB_INVOKE__
                       configSaved = true;
                       savedValue = value.toBool();
                   });

    widget->d->displayPreviewCheckBox->setChecked(true);

    EXPECT_TRUE(configSaved);
    EXPECT_TRUE(savedValue);
}

TEST_F(ViewOptionsWidgetTest, IconSizeSlider_LeftIconClicked_DecreasesValue)
{
    widget->d->iconSizeSlider->setValue(2);
    int initialValue = widget->d->iconSizeSlider->value();

    emit widget->d->iconSizeSlider->iconClicked(DSlider::LeftIcon, false);

    EXPECT_EQ(widget->d->iconSizeSlider->value(), initialValue - 1);
}

TEST_F(ViewOptionsWidgetTest, IconSizeSlider_RightIconClicked_IncreasesValue)
{
    widget->d->iconSizeSlider->setValue(2);
    int initialValue = widget->d->iconSizeSlider->value();

    emit widget->d->iconSizeSlider->iconClicked(DSlider::RightIcon, false);

    EXPECT_EQ(widget->d->iconSizeSlider->value(), initialValue + 1);
}

TEST_F(ViewOptionsWidgetTest, IconSizeSlider_LeftIconClickedAtMinimum_DoesNotDecrease)
{
    widget->d->iconSizeSlider->setValue(widget->d->iconSizeSlider->minimum());
    int initialValue = widget->d->iconSizeSlider->value();

    emit widget->d->iconSizeSlider->iconClicked(DSlider::LeftIcon, false);

    EXPECT_EQ(widget->d->iconSizeSlider->value(), initialValue);
}

TEST_F(ViewOptionsWidgetTest, IconSizeSlider_RightIconClickedAtMaximum_DoesNotIncrease)
{
    widget->d->iconSizeSlider->setValue(widget->d->iconSizeSlider->maximum());
    int initialValue = widget->d->iconSizeSlider->value();

    emit widget->d->iconSizeSlider->iconClicked(DSlider::RightIcon, false);

    EXPECT_EQ(widget->d->iconSizeSlider->value(), initialValue);
}

TEST_F(ViewOptionsWidgetTest, GridDensitySlider_LeftIconClicked_DecreasesValue)
{
    widget->d->gridDensitySlider->setValue(2);
    int initialValue = widget->d->gridDensitySlider->value();

    emit widget->d->gridDensitySlider->iconClicked(DSlider::LeftIcon, false);

    EXPECT_EQ(widget->d->gridDensitySlider->value(), initialValue - 1);
}

TEST_F(ViewOptionsWidgetTest, GridDensitySlider_RightIconClicked_IncreasesValue)
{
    widget->d->gridDensitySlider->setValue(2);
    int initialValue = widget->d->gridDensitySlider->value();

    emit widget->d->gridDensitySlider->iconClicked(DSlider::RightIcon, false);

    EXPECT_EQ(widget->d->gridDensitySlider->value(), initialValue + 1);
}

TEST_F(ViewOptionsWidgetTest, ListHeightSlider_LeftIconClicked_DecreasesValue)
{
    widget->d->listHeightSlider->setValue(2);
    int initialValue = widget->d->listHeightSlider->value();

    emit widget->d->listHeightSlider->iconClicked(DSlider::LeftIcon, false);

    EXPECT_EQ(widget->d->listHeightSlider->value(), initialValue - 1);
}

TEST_F(ViewOptionsWidgetTest, HideEvent_Called_EmitsHiddenSignal)
{
    QSignalSpy spy(widget, &ViewOptionsWidget::hidden);

    QHideEvent event;
    widget->hideEvent(&event);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ViewOptionsWidgetTest, Exec_ValidPosition_ShowsWidget)
{
    stub.set_lamda(static_cast<QScreen *(*)(const QPoint &)>(&QApplication::screenAt), [](const QPoint &) {
        __DBG_STUB_INVOKE__
        return QGuiApplication::primaryScreen();
    });

    // Stub the exec loop to avoid blocking
    stub.set_lamda(static_cast<int (QEventLoop::*)(QEventLoop::ProcessEventsFlags)>(&QEventLoop::exec),
                   [](QEventLoop *, QEventLoop::ProcessEventsFlags) {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    QPoint pos(100, 100);
    EXPECT_NO_THROW(widget->exec(pos, ViewMode::kIconMode, QUrl("file:///home/test")));
}

TEST_F(ViewOptionsWidgetTest, Exec_PositionNearRightBoundary_AdjustsPosition)
{
    QRect screenRect(0, 0, 1920, 1080);

    stub.set_lamda(static_cast<QScreen *(*)(const QPoint &)>(&QApplication::screenAt), [](const QPoint &) {
        __DBG_STUB_INVOKE__
        return QGuiApplication::primaryScreen();
    });

    stub.set_lamda(&QScreen::availableGeometry, [screenRect](const QScreen *) {
        __DBG_STUB_INVOKE__
        return screenRect;
    });

    stub.set_lamda(static_cast<int (QEventLoop::*)(QEventLoop::ProcessEventsFlags)>(&QEventLoop::exec),
                   [](QEventLoop *, QEventLoop::ProcessEventsFlags) {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    QPoint pos(1900, 100);   // Near right boundary
    widget->exec(pos, ViewMode::kIconMode, QUrl("file:///home/test"));

    EXPECT_LE(widget->pos().x() + widget->width(), screenRect.right());
}

TEST_F(ViewOptionsWidgetTest, Exec_PositionNearBottomBoundary_AdjustsPosition)
{
    QRect screenRect(0, 0, 1920, 1080);

    stub.set_lamda(static_cast<QScreen *(*)(const QPoint &)>(&QApplication::screenAt), [](const QPoint &) {
        __DBG_STUB_INVOKE__
        return QGuiApplication::primaryScreen();
    });

    stub.set_lamda(&QScreen::availableGeometry, [screenRect](const QScreen *) {
        __DBG_STUB_INVOKE__
        return screenRect;
    });

    stub.set_lamda(static_cast<int (QEventLoop::*)(QEventLoop::ProcessEventsFlags)>(&QEventLoop::exec),
                   [](QEventLoop *, QEventLoop::ProcessEventsFlags) {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    QPoint pos(100, 1070);   // Near bottom boundary
    widget->exec(pos, ViewMode::kIconMode, QUrl("file:///home/test"));

    EXPECT_LE(widget->pos().y() + widget->height(), screenRect.bottom());
}

TEST_F(ViewOptionsWidgetTest, Exec_NoScreenAtPosition_HandlesGracefully)
{
    stub.set_lamda(static_cast<QScreen *(*)(const QPoint &)>(&QApplication::screenAt), [](const QPoint &) {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(static_cast<int (QEventLoop::*)(QEventLoop::ProcessEventsFlags)>(&QEventLoop::exec),
                   [](QEventLoop *, QEventLoop::ProcessEventsFlags) {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    QPoint pos(100, 100);
    // Should not crash
    EXPECT_NO_THROW(widget->exec(pos, ViewMode::kIconMode, QUrl("file:///home/test")));
}

TEST_F(ViewOptionsWidgetTest, ShowSliderTips_ValidPosition_ShowsTooltip)
{
    QVariantList valList;
    valList << "Small"
            << "Medium"
            << "Large";

    stub.set_lamda(static_cast<void (*)(const QPoint &, const QString &, QWidget *, const QRect &, int)>(&QToolTip::showText),
                   [](const QPoint &, const QString &, QWidget *, const QRect &, int) {
                       __DBG_STUB_INVOKE__
                   });

    EXPECT_NO_THROW(widget->d->showSliderTips(widget->d->iconSizeSlider, 1, valList));
}

TEST_F(ViewOptionsWidgetTest, ShowSliderTips_InvalidPosition_HandlesGracefully)
{
    QVariantList valList;
    valList << "Small"
            << "Medium";

    EXPECT_NO_THROW(widget->d->showSliderTips(widget->d->iconSizeSlider, 5, valList));
}

TEST_F(ViewOptionsWidgetTest, ShowSliderTips_EmptyList_HandlesGracefully)
{
    QVariantList valList;

    EXPECT_NO_THROW(widget->d->showSliderTips(widget->d->iconSizeSlider, 0, valList));
}

TEST_F(ViewOptionsWidgetTest, ShowSliderTips_SingleItemList_HandlesGracefully)
{
    QVariantList valList;
    valList << "Only one";

    EXPECT_NO_THROW(widget->d->showSliderTips(widget->d->iconSizeSlider, 0, valList));
}

TEST_F(ViewOptionsWidgetTest, GetStringListByIntList_ValidList_ConvertsCorrectly)
{
    QList<int> intList { 10, 20, 30, 40 };

    QList<QString> result = widget->d->getStringListByIntList(intList);

    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "10");
    EXPECT_EQ(result[1], "20");
    EXPECT_EQ(result[2], "30");
    EXPECT_EQ(result[3], "40");
}

TEST_F(ViewOptionsWidgetTest, GetStringListByIntList_EmptyList_ReturnsEmpty)
{
    QList<int> intList;

    QList<QString> result = widget->d->getStringListByIntList(intList);

    EXPECT_TRUE(result.isEmpty());
}
