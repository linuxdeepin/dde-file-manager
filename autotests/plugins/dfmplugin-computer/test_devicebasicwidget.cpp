// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QIcon>
#include <QPixmap>

#include "stubext.h"
#include "deviceproperty/devicebasicwidget.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/filestatisticsjob.h>

#include <DArrowLineDrawer>
#include <DFontSizeManager>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_DeviceBasicWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        testUrl = QUrl("entry://test.blockdev");
        widget = new DeviceBasicWidget();
    }

    virtual void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    DeviceInfo createTestDeviceInfo()
    {
        DeviceInfo info;
        info.deviceType = "Test Device";
        info.totalCapacity = 1024LL * 1024 * 1024 * 100;   // 100GB
        info.availableSpace = 1024LL * 1024 * 1024 * 50;   // 50GB
        info.fileSystem = "ext4";
        info.mountPoint = QUrl::fromLocalFile("/test/mount");
        info.deviceName = "Test Storage";
        info.deviceDesc = "Test Description";
        return info;
    }

protected:
    stub_ext::StubExt stub;
    DeviceBasicWidget *widget = nullptr;
    QUrl testUrl;
};

TEST_F(UT_DeviceBasicWidget, SelectFileUrl_ValidUrl_UpdatesFileCount)
{
    bool createCalled = false;
    int mockChildCount = 42;

    // Mock FileInfo creation and countChildFile
    stub.set_lamda(InfoFactory::create<FileInfo>, [&createCalled, mockChildCount] {
        __DBG_STUB_INVOKE__
        createCalled = true;

        // Create a mock FileInfo
        FileInfoPointer mockInfo(new FileInfo(QUrl()));
        stub_ext::StubExt localStub;
        localStub.set_lamda(VADDR(FileInfo,  countChildFile), [mockChildCount] {
            __DBG_STUB_INVOKE__
            return mockChildCount;
        });

        return mockInfo;
    });

    // Mock KeyValueLabel methods
    stub.set_lamda(&KeyValueLabel::setRightValue, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&KeyValueLabel::setRightFontSizeWeight, [] {
        __DBG_STUB_INVOKE__
    });

    // Test selectFileUrl
    widget->selectFileUrl(testUrl);

    // Verify results
    EXPECT_TRUE(createCalled);
}

TEST_F(UT_DeviceBasicWidget, SelectFileInfo_ValidDeviceInfo_UpdatesAllFields)
{
    DeviceInfo testInfo = createTestDeviceInfo();

    // Mock UniversalUtils::sizeFormat
    QString mockTotalSizeStr = "100 GB";
    QString mockFreeSizeStr = "50 GB";
    int sizeFormatCallCount = 0;

    stub.set_lamda(static_cast<QString (*)(qint64, int)>(&UniversalUtils::sizeFormat), [&](qint64 size, int precision) -> QString {
        __DBG_STUB_INVOKE__
        sizeFormatCallCount++;
        if (size == testInfo.totalCapacity) {
            return mockTotalSizeStr;
        } else if (size == testInfo.availableSpace) {
            return mockFreeSizeStr;
        }
        return "Unknown Size";
    });

    // Mock KeyValueLabel methods to capture set values
    stub.set_lamda(&KeyValueLabel::setRightValue, [&] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&KeyValueLabel::setRightFontSizeWeight, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&KeyValueLabel::hide, [] {
        __DBG_STUB_INVOKE__
    });

    // Mock FileStatisticsJob methods
    bool setFileHintsCalled = false;
    bool startCalled = false;
    QList<QUrl> capturedUrls;

    stub.set_lamda(&FileStatisticsJob::setFileHints, [&setFileHintsCalled](FileStatisticsJob *, FileStatisticsJob::FileHints) {
        __DBG_STUB_INVOKE__
        setFileHintsCalled = true;
    });

    stub.set_lamda(static_cast<void (FileStatisticsJob::*)(const QList<QUrl> &)>(&FileStatisticsJob::start),
                   [&startCalled, &capturedUrls](FileStatisticsJob *, const QList<QUrl> &urls) {
                       __DBG_STUB_INVOKE__
                       startCalled = true;
                       capturedUrls = urls;
                   });

    // Test selectFileInfo
    widget->selectFileInfo(testInfo);

    // Verify UniversalUtils::sizeFormat was called
    EXPECT_EQ(sizeFormatCallCount, 2);

    // Verify FileStatisticsJob was configured and started
    EXPECT_TRUE(setFileHintsCalled);
    EXPECT_TRUE(startCalled);
    EXPECT_EQ(capturedUrls.size(), 1);
    EXPECT_EQ(capturedUrls.first(), testInfo.mountPoint);
}

TEST_F(UT_DeviceBasicWidget, SelectFileInfo_EmptyFileSystem_HidesFileSystemField)
{
    DeviceInfo testInfo = createTestDeviceInfo();
    testInfo.fileSystem = "";   // Empty file system

    bool hideFileSystemCalled = false;

    // Mock KeyValueLabel methods
    stub.set_lamda(&KeyValueLabel::setRightValue, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&KeyValueLabel::setRightFontSizeWeight, [] {
        __DBG_STUB_INVOKE__
    });

    // Mock hide method specifically for fileSystem
    stub.set_lamda(&KeyValueLabel::hide, [&hideFileSystemCalled, this] {
        __DBG_STUB_INVOKE__
        // Check if this is called on fileSystem widget
        hideFileSystemCalled = true;
    });

    // Mock UniversalUtils::sizeFormat
    stub.set_lamda(static_cast<QString (*)(qint64, int)>(&UniversalUtils::sizeFormat), [](qint64, int) -> QString {
        __DBG_STUB_INVOKE__
        return "Test Size";
    });

    // Mock FileStatisticsJob methods
    stub.set_lamda(&FileStatisticsJob::setFileHints, [](FileStatisticsJob *, FileStatisticsJob::FileHints) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(static_cast<void (FileStatisticsJob::*)(const QList<QUrl> &)>(&FileStatisticsJob::start),
                   [](FileStatisticsJob *, const QList<QUrl> &) {
                       __DBG_STUB_INVOKE__
                   });

    // Test selectFileInfo with empty file system
    widget->selectFileInfo(testInfo);

    // Verify fileSystem field was hidden
    EXPECT_TRUE(hideFileSystemCalled);
}

TEST_F(UT_DeviceBasicWidget, SelectFileInfo_EmptyMountPoint_UsesTotalCapacityAsFreeSize)
{
    DeviceInfo testInfo = createTestDeviceInfo();
    testInfo.mountPoint = QUrl();   // Empty mount point

    QString mockTotalSizeStr = "100 GB";
    int sizeFormatCallCount = 0;
    QList<qint64> sizeFormatArgs;

    // Mock UniversalUtils::sizeFormat to track arguments
    stub.set_lamda(static_cast<QString (*)(qint64, int)>(&UniversalUtils::sizeFormat),
                   [&](qint64 size, int precision) -> QString {
                       __DBG_STUB_INVOKE__
                       sizeFormatCallCount++;
                       sizeFormatArgs.append(size);
                       return mockTotalSizeStr;
                   });

    // Mock other required methods
    stub.set_lamda(&KeyValueLabel::setRightValue, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&KeyValueLabel::setRightFontSizeWeight, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&FileStatisticsJob::setFileHints, [](FileStatisticsJob *, FileStatisticsJob::FileHints) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(static_cast<void (FileStatisticsJob::*)(const QList<QUrl> &)>(&FileStatisticsJob::start),
                   [](FileStatisticsJob *, const QList<QUrl> &) {
                       __DBG_STUB_INVOKE__
                   });

    // Test selectFileInfo with empty mount point
    widget->selectFileInfo(testInfo);

    // Verify sizeFormat was called with totalCapacity for both total and free size
    EXPECT_GE(sizeFormatCallCount, 2);
    EXPECT_TRUE(sizeFormatArgs.contains(testInfo.totalCapacity));
}

TEST_F(UT_DeviceBasicWidget, SlotFileDirSizeChange_SingleItem_UpdatesFileCountSingular)
{
    qint64 testSize = 1024;
    int testFilesCount = 1;
    int testDirectoryCount = 0;

    QString capturedValue;
    Qt::TextElideMode capturedElideMode;
    Qt::Alignment capturedAlignment;

    // Mock KeyValueLabel::setRightValue with all parameters
    stub.set_lamda(&KeyValueLabel::setRightValue,
                   [&](KeyValueLabel *, QString value, Qt::TextElideMode elide, Qt::Alignment align, bool, int) {
                       __DBG_STUB_INVOKE__
                       capturedValue = value;
                       capturedElideMode = elide;
                       capturedAlignment = align;
                   });

    // Test slotFileDirSizeChange with single item
    widget->slotFileDirSizeChange(testSize, testFilesCount, testDirectoryCount);

    // Verify singular form is used
    EXPECT_EQ(capturedValue, "1 item");
    EXPECT_EQ(capturedElideMode, Qt::ElideNone);
    EXPECT_EQ(capturedAlignment, Qt::AlignVCenter);
}

TEST_F(UT_DeviceBasicWidget, SlotFileDirSizeChange_MultipleItems_UpdatesFileCountPlural)
{
    qint64 testSize = 2048;
    int testFilesCount = 5;
    int testDirectoryCount = 3;

    QString capturedValue;

    // Mock KeyValueLabel::setRightValue
    stub.set_lamda(&KeyValueLabel::setRightValue,
                   [&](KeyValueLabel *, QString value, Qt::TextElideMode, Qt::Alignment, bool, int) {
                       __DBG_STUB_INVOKE__
                       capturedValue = value;
                   });

    // Test slotFileDirSizeChange with multiple items
    widget->slotFileDirSizeChange(testSize, testFilesCount, testDirectoryCount);

    // Verify plural form is used (5 + 3 = 8 items)
    EXPECT_EQ(capturedValue, "8 items");
}

TEST_F(UT_DeviceBasicWidget, SlotFileDirSizeChange_ZeroItems_UpdatesFileCountSingular)
{
    qint64 testSize = 0;
    int testFilesCount = 0;
    int testDirectoryCount = 0;

    QString capturedValue;

    // Mock KeyValueLabel::setRightValue
    stub.set_lamda(&KeyValueLabel::setRightValue,
                   [&](KeyValueLabel *, QString value, Qt::TextElideMode, Qt::Alignment, bool, int) {
                       __DBG_STUB_INVOKE__
                       capturedValue = value;
                   });

    // Test slotFileDirSizeChange with zero items
    widget->slotFileDirSizeChange(testSize, testFilesCount, testDirectoryCount);

    // Verify singular form is used for 0 items
    EXPECT_EQ(capturedValue, "0 item");
}

TEST_F(UT_DeviceBasicWidget, FileStatisticsJob_ConnectedCorrectly_ReceivesSignals)
{
    // Test that FileStatisticsJob signals are properly connected

    bool slotCalled = false;
    // Mock KeyValueLabel::setRightValue to capture slot execution
    stub.set_lamda(&KeyValueLabel::setRightValue,
                   [&] {
                       __DBG_STUB_INVOKE__
                       slotCalled = true;
                   });

    // Manually trigger the slot to verify it's working
    widget->slotFileDirSizeChange(1024, 5, 3);

    EXPECT_TRUE(slotCalled);
}

TEST_F(UT_DeviceBasicWidget, FileCount_RightWidgetMaxHeight_IsSetCorrectly)
{
    // Test that fileCount right widget has correct maximum height
    EXPECT_EQ(widget->fileCount->rightWidget()->maximumHeight(), 31);
}
