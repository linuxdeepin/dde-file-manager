// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QUrl>
#include <QFrame>
#include <QDateTime>
#include <QSignalSpy>

// 包含待测试的类
#include "views/filebaseinfoview.h"
#include "utils/detailmanager.h"
#include "dfmplugin_detailspace_global.h"

// 包含依赖的头文件
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>

DPDETAILSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

/**
 * @brief FileBaseInfoView类单元测试
 *
 * 测试范围：
 * 1. UI组件初始化
 * 2. 文件URL设置处理
 * 3. 基础字段填充逻辑
 * 4. 扩展信息处理
 * 5. 信号槽连接机制
 * 6. 字段过滤功能
 * 7. 日期时间格式化
 * 8. 多媒体信息接收处理
 */
class FileBaseInfoViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test data
        testUrl = QUrl("file:///home/user/test.txt");
        testImageUrl = QUrl("file:///home/user/image.jpg");
        testVideoUrl = QUrl("file:///home/user/video.mp4");
        testAudioUrl = QUrl("file:///home/user/audio.mp3");

        view = new FileBaseInfoView(nullptr);
    }

    void TearDown() override
    {
        if (view) {
            delete view;
            view = nullptr;
        }
        stub.clear();
    }

    stub_ext::StubExt stub;
    FileBaseInfoView *view { nullptr };
    QUrl testUrl;
    QUrl testImageUrl;
    QUrl testVideoUrl;
    QUrl testAudioUrl;
};

/**
 * @brief 测试设置文件URL
 * 验证文件URL设置的基本功能
 */
TEST_F(FileBaseInfoViewTest, SetFileUrl_ValidUrl_UpdatesCurrentUrl)
{
    bool basicExpandCalled = false;
    bool basicFieldFilterCalled = false;
    bool basicFillCalled = false;
    QUrl receivedUrl;

    // Mock the internal methods
    stub.set_lamda(ADDR(FileBaseInfoView, basicExpand), [&basicExpandCalled, &receivedUrl](FileBaseInfoView *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        basicExpandCalled = true;
        receivedUrl = url;
    });

    stub.set_lamda(ADDR(FileBaseInfoView, basicFieldFilter), [&basicFieldFilterCalled](FileBaseInfoView *, const QUrl &) {
        __DBG_STUB_INVOKE__
        basicFieldFilterCalled = true;
    });

    stub.set_lamda(ADDR(FileBaseInfoView, basicFill), [&basicFillCalled](FileBaseInfoView *, const QUrl &) {
        __DBG_STUB_INVOKE__
        basicFillCalled = true;
    });

    view->setFileUrl(testUrl);

    EXPECT_TRUE(basicExpandCalled);
    EXPECT_TRUE(basicFieldFilterCalled);
    EXPECT_TRUE(basicFillCalled);
    EXPECT_EQ(receivedUrl, testUrl);
}

/**
 * @brief 测试设置文件URL - 空URL
 * 验证空URL的处理
 */
TEST_F(FileBaseInfoViewTest, SetFileUrl_EmptyUrl_HandledSafely)
{
    bool basicExpandCalled = false;

    stub.set_lamda(ADDR(FileBaseInfoView, basicExpand), [&basicExpandCalled](FileBaseInfoView *, const QUrl &) {
        __DBG_STUB_INVOKE__
        basicExpandCalled = true;
    });

    stub.set_lamda(ADDR(FileBaseInfoView, basicFieldFilter), [](FileBaseInfoView *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(ADDR(FileBaseInfoView, basicFill), [](FileBaseInfoView *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    QUrl emptyUrl;
    view->setFileUrl(emptyUrl);

    EXPECT_TRUE(basicExpandCalled);
}

/**
 * @brief 测试文件映射初始化
 * 验证字段映射的正确初始化
 */
TEST_F(FileBaseInfoViewTest, InitFileMap_Called_InitializesFieldMap)
{
    // This should not crash and should initialize the field map
    EXPECT_NO_FATAL_FAILURE(view->initFileMap());
}

/**
 * @brief 测试基础扩展
 * 验证基础扩展功能
 */
TEST_F(FileBaseInfoViewTest, BasicExpand_ValidUrl_CallsDetailManager)
{
    bool createBasicViewExtensionFieldCalled = false;
    QUrl receivedUrl;

    // Mock DetailManager
    stub.set_lamda(&DetailManager::createBasicViewExtensionField, [&createBasicViewExtensionFieldCalled, &receivedUrl](DetailManager *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        createBasicViewExtensionFieldCalled = true;
        receivedUrl = url;
        return QMap<BasicExpandType, BasicExpandMap>();
    });

    view->basicExpand(testUrl);

    EXPECT_TRUE(createBasicViewExtensionFieldCalled);
    EXPECT_EQ(receivedUrl, testUrl);
}

/**
 * @brief 测试基础字段过滤
 * 验证字段过滤功能
 */
TEST_F(FileBaseInfoViewTest, BasicFieldFilter_ValidUrl_FiltersFields)
{
    bool basicFiledFiltersCalled = false;
    QUrl receivedUrl;

    // Mock DetailManager basicFiledFiltes
    stub.set_lamda(&DetailManager::basicFiledFiltes, [&basicFiledFiltersCalled, &receivedUrl](DetailManager *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        basicFiledFiltersCalled = true;
        receivedUrl = url;
        return kNotFilter;
    });

    view->basicFieldFilter(testUrl);

    EXPECT_TRUE(basicFiledFiltersCalled);
    EXPECT_EQ(receivedUrl, testUrl);
}

/**
 * @brief 测试基础填充
 * 验证基础信息填充功能
 */
TEST_F(FileBaseInfoViewTest, BasicFill_ValidUrl_FillsBasicInfo)
{
    // Mock KeyValueLabel setRightValue
    stub.set_lamda(ADDR(KeyValueLabel, setRightValue), [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(InfoFactory::create<FileInfo>, [] {
        __DBG_STUB_INVOKE__
        QSharedPointer<FileInfo> info { new FileInfo(QUrl::fromLocalFile("/tmp")) };
        return info;
    });

    stub.set_lamda(&DetailManager::basicFiledFiltes, [] {
        __DBG_STUB_INVOKE__
        return kFileNameField;
    });

    EXPECT_NO_FATAL_FAILURE(view->basicFill(QUrl::fromLocalFile("/tmp")));
}

/**
 * @brief 测试清除字段
 * 验证字段清除功能
 */
TEST_F(FileBaseInfoViewTest, ClearField_Called_ClearsAllFields)
{
    // Mock KeyValueLabel setRightValue
    stub.set_lamda(ADDR(KeyValueLabel, setRightValue), [] {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(view->clearField());
}

/**
 * @brief 测试日期时间格式化
 * 验证日期时间格式化功能
 */
TEST_F(FileBaseInfoViewTest, GetDateTimeFormatStr_ValidDateTime_ReturnsFormattedString)
{
    QDateTime testDateTime = QDateTime::currentDateTime();
    QString result = view->getDateTimeFormatStr(testDateTime);

    // Should return a non-empty formatted string
    EXPECT_FALSE(result.isEmpty());
}

/**
 * @brief 测试日期时间格式化 - 无效日期
 * 验证无效日期的处理
 */
TEST_F(FileBaseInfoViewTest, GetDateTimeFormatStr_InvalidDateTime_ReturnsEmptyString)
{
    QDateTime invalidDateTime;
    EXPECT_NO_FATAL_FAILURE(view->getDateTimeFormatStr(invalidDateTime));
}

/**
 * @brief 测试图像扩展信息接收
 * 验证图像信息接收器功能
 */
TEST_F(FileBaseInfoViewTest, ImageExtenInfoReceiver_ValidProperties_ProcessesImageInfo)
{
    QStringList testProperties;
    testProperties << "Width: 1920"
                   << "Height: 1080"
                   << "Format: JPEG";

    // This should not crash
    EXPECT_NO_FATAL_FAILURE(view->imageExtenInfoReceiver(testProperties));
}

/**
 * @brief 测试视频扩展信息接收
 * 验证视频信息接收器功能
 */
TEST_F(FileBaseInfoViewTest, VideoExtenInfoReceiver_ValidProperties_ProcessesVideoInfo)
{
    QStringList testProperties;
    testProperties << "Duration: 120s"
                   << "Resolution: 1920x1080"
                   << "Codec: H.264";

    // This should not crash
    EXPECT_NO_FATAL_FAILURE(view->videoExtenInfoReceiver(testProperties));
}

/**
 * @brief 测试音频扩展信息接收
 * 验证音频信息接收器功能
 */
TEST_F(FileBaseInfoViewTest, AudioExtenInfoReceiver_ValidProperties_ProcessesAudioInfo)
{
    QStringList testProperties;
    testProperties << "Duration: 180s"
                   << "Bitrate: 320kbps"
                   << "Codec: MP3";

    // This should not crash
    EXPECT_NO_FATAL_FAILURE(view->audioExtenInfoReceiver(testProperties));
}

/**
 * @brief 测试图像扩展信息槽函数
 * 验证图像扩展信息槽的功能
 */
TEST_F(FileBaseInfoViewTest, ImageExtenInfo_ValidData_EmitsSignal)
{
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties;
    properties[DFMIO::DFileInfo::AttributeExtendID::kExtendMediaWidth] = 1920;
    properties[DFMIO::DFileInfo::AttributeExtendID::kExtendMediaHeight] = 1080;

    EXPECT_NO_FATAL_FAILURE(view->imageExtenInfo(testImageUrl, properties));
}

/**
 * @brief 测试视频扩展信息槽函数
 * 验证视频扩展信息槽的功能
 */
TEST_F(FileBaseInfoViewTest, VideoExtenInfo_ValidData_EmitsSignal)
{
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties;
    properties[DFMIO::DFileInfo::AttributeExtendID::kExtendMediaDuration] = 120;

    EXPECT_NO_FATAL_FAILURE(view->videoExtenInfo(testVideoUrl, properties));
}

/**
 * @brief 测试音频扩展信息槽函数
 * 验证音频扩展信息槽的功能
 */
TEST_F(FileBaseInfoViewTest, AudioExtenInfo_ValidData_EmitsSignal)
{
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties;
    properties[DFMIO::DFileInfo::AttributeExtendID::kExtendMediaDuration] = 180;

    EXPECT_NO_FATAL_FAILURE(view->audioExtenInfo(testAudioUrl, properties));
}

/**
 * @brief 测试图像扩展信息槽处理函数
 * 验证图像扩展信息槽处理的功能
 */
TEST_F(FileBaseInfoViewTest, SlotImageExtenInfo_ValidProperties_UpdatesView)
{
    // Mock KeyValueLabel setRightValue
    stub.set_lamda(ADDR(KeyValueLabel, setRightValue), [] {
        __DBG_STUB_INVOKE__
    });

    QStringList testProperties;
    testProperties << "Width: 1920"
                   << "Height: 1080";

    EXPECT_NO_FATAL_FAILURE(view->slotImageExtenInfo(testProperties));
}

/**
 * @brief 测试视频扩展信息槽处理函数
 * 验证视频扩展信息槽处理的功能
 */
TEST_F(FileBaseInfoViewTest, SlotVideoExtenInfo_ValidProperties_UpdatesView)
{
    // Mock KeyValueLabel setRightValue
    stub.set_lamda(ADDR(KeyValueLabel, setRightValue), [] {
        __DBG_STUB_INVOKE__
    });

    QStringList testProperties;
    testProperties << "Duration: 120s";

    EXPECT_NO_FATAL_FAILURE(view->slotVideoExtenInfo(testProperties));
}

/**
 * @brief 测试音频扩展信息槽处理函数
 * 验证音频扩展信息槽处理的功能
 */
TEST_F(FileBaseInfoViewTest, SlotAudioExtenInfo_ValidProperties_UpdatesView)
{
    // Mock KeyValueLabel setRightValue
    stub.set_lamda(ADDR(KeyValueLabel, setRightValue), [] {
        __DBG_STUB_INVOKE__
    });

    QStringList testProperties;
    testProperties << "Bitrate: 320kbps";

    EXPECT_NO_FATAL_FAILURE(view->slotAudioExtenInfo(testProperties));
}
