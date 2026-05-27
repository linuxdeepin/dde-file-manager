// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QKeyEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QApplication>
#include <QRect>

#include "stubext.h"
#include "deviceproperty/devicepropertydialog.h"
#include "deviceproperty/devicebasicwidget.h"
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/elidetextlayout.h>
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>

#include <DDrawer>
#include <denhancedwidget.h>
#include <DArrowLineDrawer>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <DDialog>
#include <DLabel>
#include <DFontSizeManager>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_DevicePropertyDialog : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        testUrl = QUrl("entry://test.blockdev");
        dialog = new DevicePropertyDialog();
        dialog->setSelectDeviceInfo(createTestDeviceInfo());
    }

    virtual void TearDown() override
    {
        delete dialog;
        dialog = nullptr;
        stub.clear();
    }

    DeviceInfo createTestDeviceInfo()
    {
        DeviceInfo info;
        info.deviceType = "Test Storage Device";
        info.totalCapacity = 1024LL * 1024 * 1024 * 100;   // 100GB
        info.availableSpace = 1024LL * 1024 * 1024 * 50;   // 50GB
        info.fileSystem = "ext4";
        info.mountPoint = QUrl::fromLocalFile("/test/mount");
        info.deviceName = "Test USB Drive";
        info.deviceDesc = "USB 3.0 Storage";
        info.deviceUrl = testUrl;
        info.icon = QIcon(":/test/icon.png");
        return info;
    }

protected:
    stub_ext::StubExt stub;
    DevicePropertyDialog *dialog = nullptr;
    QUrl testUrl;
};

TEST_F(UT_DevicePropertyDialog, ContentHeight_WithExtendedControls_CalculatesCorrectHeight)
{
    // Mock widget height methods
    const int mockIconHeight = 128;
    const int mockBasicInfoHeight = 50;
    const int mockProgressBarHeight = 8;
    const int mockNameFrameHeight = 30;

    stub.set_lamda(&QWidget::height, [=](QWidget *widget) -> int {
        __DBG_STUB_INVOKE__
        if (widget == dialog->deviceIcon)
            return mockIconHeight;
        else if (widget == dialog->basicInfo)
            return mockBasicInfoHeight;
        else if (widget == dialog->devicesProgressBar)
            return mockProgressBarHeight;
        else if (widget == dialog->deviceNameFrame)
            return mockNameFrameHeight;
        return 10;   // Default height for other widgets
    });

    stub.set_lamda(&QWidget::contentsMargins, [](QWidget *) -> QMargins {
        __DBG_STUB_INVOKE__
        return QMargins(10, 10, 10, 10);
    });

    // Calculate expected height
    int expectedHeight = 50 + mockIconHeight + mockNameFrameHeight + mockBasicInfoHeight + mockProgressBarHeight + 10 + 10 + 10 + 40;   // margins + spacing

    // Test contentHeight calculation
    int actualHeight = dialog->contentHeight();
    EXPECT_GT(actualHeight, 0);
}

TEST_F(UT_DevicePropertyDialog, SetSelectDeviceInfo_ValidDeviceInfo_UpdatesAllUIElements)
{
    DeviceInfo testInfo = createTestDeviceInfo();

    bool setPixmapCalled = false;
    bool setFileNameCalled = false;
    bool selectFileInfoCalled = false;
    bool setLeftValueCalled = false;
    bool setProgressBarCalled = false;
    bool addExtendedControlCalled = false;

    // Mock DLabel::setPixmap
    stub.set_lamda(&DLabel::setPixmap, [&setPixmapCalled] {
        __DBG_STUB_INVOKE__
        setPixmapCalled = true;
    });

    // Mock setFileName
    stub.set_lamda(&DevicePropertyDialog::setFileName, [&setFileNameCalled](DevicePropertyDialog *, const QString &) {
        __DBG_STUB_INVOKE__
        setFileNameCalled = true;
    });

    // Mock DeviceBasicWidget::selectFileInfo
    stub.set_lamda(&DeviceBasicWidget::selectFileInfo, [&selectFileInfoCalled](DeviceBasicWidget *, const DeviceInfo &) {
        __DBG_STUB_INVOKE__
        selectFileInfoCalled = true;
    });

    // Mock KeyValueLabel::setLeftValue
    stub.set_lamda(&KeyValueLabel::setLeftValue, [&setLeftValueCalled] {
        __DBG_STUB_INVOKE__
        setLeftValueCalled = true;
    });

    // Mock setProgressBar
    stub.set_lamda(&DevicePropertyDialog::setProgressBar, [&setProgressBarCalled](DevicePropertyDialog *, qint64, qint64, bool) {
        __DBG_STUB_INVOKE__
        setProgressBarCalled = true;
    });

    // Mock addExtendedControl
    stub.set_lamda(&DevicePropertyDialog::addExtendedControl, [&addExtendedControlCalled](DevicePropertyDialog *, QWidget *) {
        __DBG_STUB_INVOKE__
        addExtendedControlCalled = true;
    });

    // Test setSelectDeviceInfo
    dialog->setSelectDeviceInfo(testInfo);

    // Verify all methods were called
    EXPECT_TRUE(setPixmapCalled);
    EXPECT_TRUE(setFileNameCalled);
    EXPECT_TRUE(selectFileInfoCalled);
    EXPECT_TRUE(setLeftValueCalled);
    EXPECT_TRUE(setProgressBarCalled);
    EXPECT_TRUE(addExtendedControlCalled);

    // Verify currentFileUrl was set
    EXPECT_EQ(dialog->currentFileUrl, testInfo.deviceUrl);
}

TEST_F(UT_DevicePropertyDialog, SetProgressBar_MountedDevice_SetsCorrectProgress)
{
    qint64 totalSize = 1024LL * 1024 * 1024 * 100;   // 100GB
    qint64 freeSize = 1024LL * 1024 * 1024 * 50;   // 50GB
    bool mounted = true;

    QString capturedTotalStr, capturedUsedStr, capturedRightValue;
    int capturedProgressValue = -1;
    bool setRightValueCalled = false;

    // Mock UniversalUtils::sizeFormat
    stub.set_lamda(static_cast<QString (*)(qint64, int)>(&UniversalUtils::sizeFormat),
                   [&](qint64 size, int precision) -> QString {
                       __DBG_STUB_INVOKE__
                       if (size == totalSize) {
                           capturedTotalStr = "100 GB";
                           return capturedTotalStr;
                       } else if (size == totalSize - freeSize) {
                           capturedUsedStr = "50 GB";
                           return capturedUsedStr;
                       }
                       return "Unknown";
                   });

    // Mock progress bar setValue
    stub.set_lamda(&DColoredProgressBar::setValue, [&capturedProgressValue](QProgressBar *&, int value) {
        __DBG_STUB_INVOKE__
        capturedProgressValue = value;
    });

    // Mock KeyValueLabel::setRightValue
    stub.set_lamda(&KeyValueLabel::setRightValue, [&](KeyValueLabel *, QString value, Qt::TextElideMode, Qt::Alignment, bool, int) {
        __DBG_STUB_INVOKE__
        capturedRightValue = value;
        setRightValueCalled = true;
    });

    // Mock other required methods
    stub.set_lamda(&KeyValueLabel::setRightFontSizeWeight, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&DGuiApplicationHelper::themeType, [] {
        __DBG_STUB_INVOKE__
        return DGuiApplicationHelper::LightType;
    });

    // Test setProgressBar
    dialog->setProgressBar(totalSize, freeSize, mounted);

    // Verify progress value calculation (50% used)
    EXPECT_EQ(capturedProgressValue, 5000);   // 50% of 10000

    // Verify right value format for mounted device
    EXPECT_TRUE(setRightValueCalled);
    EXPECT_TRUE(capturedRightValue.contains("/"));
}

TEST_F(UT_DevicePropertyDialog, SetProgressBar_UnmountedDevice_ShowsTotalSizeOnly)
{
    qint64 totalSize = 1024LL * 1024 * 1024 * 100;
    qint64 freeSize = 1024LL * 1024 * 1024 * 50;
    bool mounted = false;

    QString capturedRightValue;
    int capturedProgressValue = -1;

    // Mock UniversalUtils::sizeFormat
    stub.set_lamda(static_cast<QString (*)(qint64, int)>(&UniversalUtils::sizeFormat),
                   [](qint64 size, int precision) -> QString {
                       __DBG_STUB_INVOKE__
                       return "100 GB";
                   });

    // Mock progress bar setValue
    stub.set_lamda(&DColoredProgressBar::setValue, [&capturedProgressValue](QProgressBar *&, int value) {
        __DBG_STUB_INVOKE__
        capturedProgressValue = value;
    });

    // Mock KeyValueLabel::setRightValue
    stub.set_lamda(&KeyValueLabel::setRightValue, [&](KeyValueLabel *, QString value, Qt::TextElideMode, Qt::Alignment, bool, int) {
        __DBG_STUB_INVOKE__
        capturedRightValue = value;
    });

    // Mock other required methods
    stub.set_lamda(&KeyValueLabel::setRightFontSizeWeight, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&DGuiApplicationHelper::themeType, [] {
        __DBG_STUB_INVOKE__
        return DGuiApplicationHelper::DarkType;
    });

    // Test setProgressBar for unmounted device
    dialog->setProgressBar(totalSize, freeSize, mounted);

    // Verify progress value is 0 for unmounted device
    EXPECT_EQ(capturedProgressValue, 0);

    // Verify right value shows only total size (no slash)
    EXPECT_FALSE(capturedRightValue.contains("/"));
    EXPECT_EQ(capturedRightValue, "100 GB");
}

TEST_F(UT_DevicePropertyDialog, SetFileName_LongFileName_CreatesMultipleLabels)
{
    QString longFileName = "This is a very long file name that should be elided and split into multiple lines for proper display";

    bool deleteCalled = false;
    int createLabelCount = 0;
    QStringList createdLabelTexts;

    // Mock delete of existing frame
    if (dialog->deviceNameFrame) {
        delete dialog->deviceNameFrame;
        dialog->deviceNameFrame = nullptr;
    }

    // Mock ElideTextLayout::layout
    stub.set_lamda(&ElideTextLayout::layout, [&](ElideTextLayout *, const QRectF &, Qt::TextElideMode, QPainter *, const QBrush &, QStringList *labelTexts) {
        __DBG_STUB_INVOKE__
        if (labelTexts) {
            labelTexts->append("This is a very long file name that should be");
            labelTexts->append("elided and split into multiple lines for proper");
            labelTexts->append("display");
        }
        return QList<QRectF>();
    });

    stub.set_lamda(&DLabel::setAlignment, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&DLabel::fontInfo, [] {
        __DBG_STUB_INVOKE__
        QFont font;
        return QFontInfo(font);
    });

    stub.set_lamda(&QFontInfo::pixelSize, [](QFontInfo *) -> int {
        __DBG_STUB_INVOKE__
        return 14;
    });

    stub.set_lamda(&DLabel::fontMetrics, [] {
        __DBG_STUB_INVOKE__
        QFont font;
        return QFontMetrics(font);
    });

    typedef int (QFontMetrics::*Func)(const QString &, int) const;
    stub.set_lamda(static_cast<Func>(&QFontMetrics::horizontalAdvance),
                   [] {
                       __DBG_STUB_INVOKE__
                       return 150;   // Mock width less than 190 (200-10)
                   });

    stub.set_lamda(&DLabel::setFixedWidth, [] {
        __DBG_STUB_INVOKE__
    });

    // Test setFileName
    dialog->setFileName(longFileName);

    // Verify a new frame was created
    EXPECT_NE(dialog->deviceNameFrame, nullptr);
}

TEST_F(UT_DevicePropertyDialog, AddExtendedControl_ValidWidget_AddsWidgetToScrollArea)
{
    QWidget *testWidget = new QWidget();

    bool insertExtendedControlCalled = false;
    int capturedIndex = -1;
    QWidget *capturedWidget = nullptr;

    // Mock layout count
    stub.set_lamda(VADDR(QVBoxLayout, count), [] {
        __DBG_STUB_INVOKE__
        return 5;   // Mock existing widgets count
    });

    // Mock insertExtendedControl
    stub.set_lamda(&DevicePropertyDialog::insertExtendedControl, [&](DevicePropertyDialog *, int index, QWidget *widget) {
        __DBG_STUB_INVOKE__
        insertExtendedControlCalled = true;
        capturedIndex = index;
        capturedWidget = widget;
    });

    // Test addExtendedControl
    dialog->addExtendedControl(testWidget);

    // Verify insertExtendedControl was called with correct parameters
    EXPECT_TRUE(insertExtendedControlCalled);
    EXPECT_EQ(capturedIndex, 5);
    EXPECT_EQ(capturedWidget, testWidget);

    delete testWidget;
}

TEST_F(UT_DevicePropertyDialog, InsertExtendedControl_ValidWidgetAndIndex_InsertsCorrectly)
{
    QWidget *testWidget = new QWidget();
    int testIndex = 2;

    bool insertWidgetCalled = false;
    bool setFixedWidthCalled = false;

    // Mock QVBoxLayout methods
    stub.set_lamda(&QVBoxLayout::insertWidget, [&](QBoxLayout *&, int index, QWidget *widget, int stretch, Qt::Alignment) {
        __DBG_STUB_INVOKE__
        insertWidgetCalled = true;
        EXPECT_EQ(index, testIndex);
        EXPECT_EQ(widget, testWidget);
    });

    stub.set_lamda(&QVBoxLayout::contentsMargins, [] {
        __DBG_STUB_INVOKE__
        return QMargins(10, 10, 10, 10);
    });

    // Mock widget methods
    stub.set_lamda(&QWidget::contentsRect, [](QWidget *) -> QRect {
        __DBG_STUB_INVOKE__
        return QRect(0, 0, 350, 500);
    });

    stub.set_lamda(&QWidget::setFixedWidth, [&](QWidget *widget, int width) {
        __DBG_STUB_INVOKE__
        if (widget == testWidget) {
            setFixedWidthCalled = true;
            EXPECT_EQ(width, 330);   // 350 - 10 - 10
        }
    });

    // Test insertExtendedControl
    dialog->insertExtendedControl(testIndex, testWidget);

    // Verify operations were performed
    EXPECT_TRUE(insertWidgetCalled);
    EXPECT_TRUE(setFixedWidthCalled);

    delete testWidget;
}

TEST_F(UT_DevicePropertyDialog, HandleHeight_ValidHeight_UpdatesDialogGeometry)
{
    int testHeight = 100;

    QRect originalGeometry(100, 100, 350, 400);
    QRect capturedGeometry;
    bool setGeometryCalled = false;

    // Mock geometry methods
    stub.set_lamda(&DevicePropertyDialog::geometry, [&originalGeometry] () -> QRect & {
        __DBG_STUB_INVOKE__
        return originalGeometry;
    });

    stub.set_lamda(static_cast<void (QWidget::*)(const QRect &)>(&QWidget::setGeometry),
                   [&](QWidget *, const QRect &rect) {
                       __DBG_STUB_INVOKE__
                       capturedGeometry = rect;
                       setGeometryCalled = true;
                   });

    // Mock contentHeight
    stub.set_lamda(&DevicePropertyDialog::contentHeight, [testHeight](DevicePropertyDialog *) -> int {
        __DBG_STUB_INVOKE__
        return testHeight;
    });

    // Test handleHeight
    dialog->handleHeight(testHeight);

    // Verify geometry was updated
    EXPECT_TRUE(setGeometryCalled);
    EXPECT_EQ(capturedGeometry.width(), originalGeometry.width());
    EXPECT_EQ(capturedGeometry.height(), testHeight + 20);   // contentHeight + kArrowExpandSpacing * 2
}

TEST_F(UT_DevicePropertyDialog, ShowEvent_ValidEvent_SetsCorrectGeometry)
{
    QShowEvent testEvent;

    bool parentShowEventCalled = false;
    bool setGeometryCalled = false;
    int mockContentHeight = 200;
    QRect originalGeometry(100, 100, 350, 400);

    // Mock parent showEvent
    stub.set_lamda(VADDR(DDialog, showEvent), [&parentShowEventCalled] {
        __DBG_STUB_INVOKE__
        parentShowEventCalled = true;
    });

    // Mock geometry methods
    stub.set_lamda(&DevicePropertyDialog::geometry, [&originalGeometry]() -> QRect & {
        __DBG_STUB_INVOKE__
        return originalGeometry;
    });

    stub.set_lamda(static_cast<void (QWidget::*)(const QRect &)>(&QWidget::setGeometry),
                   [&setGeometryCalled](QWidget *, const QRect &) {
                       __DBG_STUB_INVOKE__
                       setGeometryCalled = true;
                   });

    // Mock contentHeight
    stub.set_lamda(&DevicePropertyDialog::contentHeight, [mockContentHeight](DevicePropertyDialog *) -> int {
        __DBG_STUB_INVOKE__
        return mockContentHeight;
    });

    // Test showEvent
    dialog->showEvent(&testEvent);

    // Verify parent showEvent was called and geometry was set
    EXPECT_TRUE(parentShowEventCalled);
    EXPECT_TRUE(setGeometryCalled);
}

TEST_F(UT_DevicePropertyDialog, CloseEvent_ValidEvent_EmitsClosedSignal)
{
    QCloseEvent testEvent;

    bool closedSignalEmitted = false;
    bool parentCloseEventCalled = false;

    // Connect to closed signal
    QSignalSpy closedSpy(dialog, SIGNAL(closed(QUrl)));

    // Mock parent closeEvent
    stub.set_lamda(VADDR(DDialog, closeEvent), [&parentCloseEventCalled](DDialog *, QCloseEvent *) {
        __DBG_STUB_INVOKE__
        parentCloseEventCalled = true;
    });

    // Set a test URL
    dialog->currentFileUrl = testUrl;

    // Test closeEvent
    dialog->closeEvent(&testEvent);

    // Verify signal was emitted and parent method was called
    EXPECT_EQ(closedSpy.count(), 1);
    EXPECT_EQ(closedSpy.at(0).at(0).toUrl(), testUrl);
    EXPECT_TRUE(parentCloseEventCalled);
}

TEST_F(UT_DevicePropertyDialog, KeyPressEvent_EscapeKey_ClosesDialog)
{
    bool closeCalled = false;
    bool parentKeyPressEventCalled = false;

    // Mock close method
    stub.set_lamda(&QWidget::close, [&closeCalled](QWidget *) -> bool {
        __DBG_STUB_INVOKE__
        closeCalled = true;
        return true;
    });

    // Mock parent keyPressEvent
    stub.set_lamda(VADDR(DDialog, keyPressEvent), [&parentKeyPressEventCalled](DDialog *, QKeyEvent *) {
        __DBG_STUB_INVOKE__
        parentKeyPressEventCalled = true;
    });

    // Create escape key event
    QKeyEvent escapeEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);

    // Test keyPressEvent with Escape key
    dialog->keyPressEvent(&escapeEvent);

    // Verify close was called and parent method was called
    EXPECT_TRUE(closeCalled);
    EXPECT_TRUE(parentKeyPressEventCalled);
}

TEST_F(UT_DevicePropertyDialog, KeyPressEvent_OtherKey_DoesNotClose)
{
    bool closeCalled = false;
    bool parentKeyPressEventCalled = false;

    // Mock close method
    stub.set_lamda(&QWidget::close, [&closeCalled](QWidget *) -> bool {
        __DBG_STUB_INVOKE__
        closeCalled = true;
        return true;
    });

    // Mock parent keyPressEvent
    stub.set_lamda(VADDR(DDialog, keyPressEvent), [&parentKeyPressEventCalled](DDialog *, QKeyEvent *) {
        __DBG_STUB_INVOKE__
        parentKeyPressEventCalled = true;
    });

    // Create non-escape key event
    QKeyEvent otherEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);

    // Test keyPressEvent with other key
    dialog->keyPressEvent(&otherEvent);

    // Verify close was not called but parent method was called
    EXPECT_FALSE(closeCalled);
    EXPECT_TRUE(parentKeyPressEventCalled);
}

// DFMRoundBackground Tests
class UT_DFMRoundBackground : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        parentWidget = new QWidget();
        background = new DFMRoundBackground(parentWidget, 8);
    }

    virtual void TearDown() override
    {
        delete background;   // This will also remove event filter
        delete parentWidget;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    QWidget *parentWidget = nullptr;
    DFMRoundBackground *background = nullptr;
};

TEST_F(UT_DFMRoundBackground, Construction_ValidWidget_InstallsEventFilter)
{
    // Test that background object was created
    EXPECT_NE(background, nullptr);

    // Test that radius property is set
    QVariant radiusProperty = background->property("radius");
    EXPECT_TRUE(radiusProperty.isValid());
    EXPECT_EQ(radiusProperty.toInt(), 8);

    // Test that parent is correct
    EXPECT_EQ(background->parent(), parentWidget);
}

TEST_F(UT_DFMRoundBackground, Destruction_ValidBackground_RemovesEventFilter)
{
    bool removeEventFilterCalled = false;

    // Mock removeEventFilter
    stub.set_lamda(&QObject::removeEventFilter, [&removeEventFilterCalled](QObject *, QObject *) {
        __DBG_STUB_INVOKE__
        removeEventFilterCalled = true;
    });

    // Delete background object
    delete background;
    background = nullptr;

    // Verify removeEventFilter was called
    EXPECT_TRUE(removeEventFilterCalled);
}

TEST_F(UT_DFMRoundBackground, EventFilter_PaintEvent_DrawsRoundedBackground)
{
    bool paintingOccurred = false;
    QColor capturedColor;

    stub.set_lamda(&QPainter::setRenderHint, [](QPainter *, QPainter::RenderHint, bool) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QPainter::fillPath, [&](QPainter *, const QPainterPath &, const QBrush &brush) {
        __DBG_STUB_INVOKE__
        paintingOccurred = true;
        capturedColor = brush.color();
    });

    // Mock QGuiApplication::palette
    stub.set_lamda(&QGuiApplication::palette, []() -> QPalette {
        __DBG_STUB_INVOKE__
        QPalette palette;
        palette.setColor(QPalette::Base, QColor(255, 255, 255));
        return palette;
    });

    // Mock widget size
    stub.set_lamda(&QWidget::size, [](QWidget *) -> QSize {
        __DBG_STUB_INVOKE__
        return QSize(200, 100);
    });

    // Create paint event
    QPaintEvent paintEvent(QRect(0, 0, 200, 100));

    // Test eventFilter with paint event
    bool result = background->eventFilter(parentWidget, &paintEvent);

    // Verify painting occurred and event was handled
    EXPECT_TRUE(result);
    EXPECT_TRUE(paintingOccurred);
    EXPECT_EQ(capturedColor, QColor(255, 255, 255));
}

TEST_F(UT_DFMRoundBackground, EventFilter_NonPaintEvent_ReturnsParentResult)
{
    // Create non-paint event
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);

    // Test eventFilter with non-paint event
    bool result = background->eventFilter(parentWidget, &keyEvent);

    // Verify event was not handled by the filter
    EXPECT_FALSE(result);
}

TEST_F(UT_DFMRoundBackground, EventFilter_WrongWatchedObject_ReturnsParentResult)
{
    QWidget *otherWidget = new QWidget();
    QPaintEvent paintEvent(QRect(0, 0, 200, 100));

    // Test eventFilter with different watched object
    bool result = background->eventFilter(otherWidget, &paintEvent);

    // Verify event was not handled
    EXPECT_FALSE(result);

    delete otherWidget;
}
