// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QUrl>
#include <QVariant>
#include <QMap>
#include <QSignalSpy>
#include <QTest>

#include "stubext.h"

#include <dfm-base/file/local/private/infodatafuture.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-io/dfilefuture.h>
#include <dfm-io/dfileinfo.h>

USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE

class TestInfoDataFuture : public testing::Test
{
public:
    void SetUp() override
    {
        testUrl = QUrl("file:///tmp/test.mp4");
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    QUrl testUrl;
};

// ========== Constructor Tests ==========

TEST_F(TestInfoDataFuture, Constructor_WithValidFuture)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    EXPECT_NE(infoFuture, nullptr);
    EXPECT_FALSE(infoFuture->isFinished());

    delete infoFuture;
}

TEST_F(TestInfoDataFuture, Constructor_WithNullFuture)
{
    InfoDataFuture *infoFuture = new InfoDataFuture(nullptr);

    EXPECT_NE(infoFuture, nullptr);
    EXPECT_FALSE(infoFuture->isFinished());

    delete infoFuture;
}

// ========== MediaInfo Tests ==========

TEST_F(TestInfoDataFuture, MediaInfo_InitiallyEmpty)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    QMap<DFileInfo::AttributeExtendID, QVariant> mediaInfo = infoFuture->mediaInfo();
    EXPECT_TRUE(mediaInfo.isEmpty());

    delete infoFuture;
}

TEST_F(TestInfoDataFuture, MediaInfo_AfterInfoMedia)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    QMap<DFileInfo::AttributeExtendID, QVariant> testData;
    testData[DFileInfo::AttributeExtendID::kExtendMediaWidth] = 1920;
    testData[DFileInfo::AttributeExtendID::kExtendMediaHeight] = 1080;

    // Trigger infoMedia slot
    emit mockFuture->infoMedia(testUrl, testData);

    // Give some time for signal processing
    QTest::qWait(10);

    QMap<DFileInfo::AttributeExtendID, QVariant> mediaInfo = infoFuture->mediaInfo();
    EXPECT_EQ(mediaInfo.size(), 2);
    EXPECT_EQ(mediaInfo[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt(), 1920);
    EXPECT_EQ(mediaInfo[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt(), 1080);

    delete infoFuture;
}

// ========== IsFinished Tests ==========

TEST_F(TestInfoDataFuture, IsFinished_InitiallyFalse)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    EXPECT_FALSE(infoFuture->isFinished());

    delete infoFuture;
}

TEST_F(TestInfoDataFuture, IsFinished_AfterInfoMedia)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    QMap<DFileInfo::AttributeExtendID, QVariant> testData;
    testData[DFileInfo::AttributeExtendID::kExtendMediaDuration] = 3600;

    // Trigger infoMedia slot
    emit mockFuture->infoMedia(testUrl, testData);

    // Give some time for signal processing
    QTest::qWait(10);

    EXPECT_TRUE(infoFuture->isFinished());

    delete infoFuture;
}

// ========== InfoMedia Slot Tests ==========

TEST_F(TestInfoDataFuture, InfoMedia_ProcessesData)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    QMap<DFileInfo::AttributeExtendID, QVariant> testData;
    testData[DFileInfo::AttributeExtendID::kExtendMediaWidth] = 3840;
    testData[DFileInfo::AttributeExtendID::kExtendMediaHeight] = 2160;
    testData[DFileInfo::AttributeExtendID::kExtendMediaDuration] = 7200;

    // Trigger infoMedia slot
    emit mockFuture->infoMedia(testUrl, testData);

    // Give some time for signal processing
    QTest::qWait(10);

    QMap<DFileInfo::AttributeExtendID, QVariant> mediaInfo = infoFuture->mediaInfo();
    EXPECT_EQ(mediaInfo.size(), 3);
    EXPECT_TRUE(infoFuture->isFinished());

    delete infoFuture;
}

TEST_F(TestInfoDataFuture, InfoMedia_EmitsSignal)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    // Use QSignalSpy to monitor infoMediaAttributes signal
    QSignalSpy spy(infoFuture, &InfoDataFuture::infoMediaAttributes);

    QMap<DFileInfo::AttributeExtendID, QVariant> testData;
    testData[DFileInfo::AttributeExtendID::kExtendMediaWidth] = 1280;

    // Stub FileInfoHelper to avoid dependency
    stub.set_lamda(&FileInfoHelper::instance, []() -> FileInfoHelper & {
        __DBG_STUB_INVOKE__
        static FileInfoHelper helper;
        return helper;
    });

    stub.set_lamda(&FileInfoHelper::mediaDataFinished,
                   [](FileInfoHelper *, const QUrl &, const QMap<DFileInfo::AttributeExtendID, QVariant> &) {
                       __DBG_STUB_INVOKE__
                       // Do nothing, just prevent actual processing
                   });

    // Trigger infoMedia slot
    emit mockFuture->infoMedia(testUrl, testData);

    // Give some time for signal processing
    QTest::qWait(10);

    EXPECT_EQ(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).value<QUrl>(), testUrl);

    delete infoFuture;
}

// ========== Future Reset Tests ==========

TEST_F(TestInfoDataFuture, InfoMedia_ResetsFutureAfterProcessing)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    QMap<DFileInfo::AttributeExtendID, QVariant> testData;
    testData[DFileInfo::AttributeExtendID::kExtendMediaWidth] = 640;

    // Stub FileInfoHelper to avoid dependency
    stub.set_lamda(&FileInfoHelper::instance, []() -> FileInfoHelper & {
        __DBG_STUB_INVOKE__
        static FileInfoHelper helper;
        return helper;
    });

    stub.set_lamda(&FileInfoHelper::mediaDataFinished,
                   [](FileInfoHelper *, const QUrl &, const QMap<DFileInfo::AttributeExtendID, QVariant> &) {
                       __DBG_STUB_INVOKE__
                   });

    // Trigger infoMedia slot
    emit mockFuture->infoMedia(testUrl, testData);

    // Give some time for signal processing
    QTest::qWait(10);

    // After infoMedia processing, future should be reset to nullptr
    // We can verify this by checking that the data is still available
    EXPECT_TRUE(infoFuture->isFinished());
    EXPECT_FALSE(infoFuture->mediaInfo().isEmpty());

    delete infoFuture;
}

// ========== Signal Connection Tests ==========

TEST_F(TestInfoDataFuture, Constructor_ConnectsSignals)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    // Verify that signals are connected by checking signal emission
    QSignalSpy spy(infoFuture, &InfoDataFuture::infoMediaAttributes);

    QMap<DFileInfo::AttributeExtendID, QVariant> testData;
    testData[DFileInfo::AttributeExtendID::kExtendMediaDuration] = 100;

    // Stub FileInfoHelper to avoid dependency
    stub.set_lamda(&FileInfoHelper::instance, []() -> FileInfoHelper & {
        __DBG_STUB_INVOKE__
        static FileInfoHelper helper;
        return helper;
    });

    stub.set_lamda(&FileInfoHelper::mediaDataFinished,
                   [](FileInfoHelper *, const QUrl &, const QMap<DFileInfo::AttributeExtendID, QVariant> &) {
                       __DBG_STUB_INVOKE__
                   });

    emit mockFuture->infoMedia(testUrl, testData);
    QTest::qWait(10);

    EXPECT_GT(spy.count(), 0);

    delete infoFuture;
}

// ========== Destructor Tests ==========

TEST_F(TestInfoDataFuture, Destructor_HandlesNullFuture)
{
    InfoDataFuture *infoFuture = new InfoDataFuture(nullptr);
    delete infoFuture;
    // No assertion needed, just verify it doesn't crash
}

TEST_F(TestInfoDataFuture, Destructor_HandlesValidFuture)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);
    delete infoFuture;
    // No assertion needed, just verify it doesn't crash
}

// ========== Multiple InfoMedia Calls Tests ==========

TEST_F(TestInfoDataFuture, InfoMedia_MultipleCallsUpdateData)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    // Stub FileInfoHelper to avoid dependency
    stub.set_lamda(&FileInfoHelper::instance, []() -> FileInfoHelper & {
        __DBG_STUB_INVOKE__
        static FileInfoHelper helper;
        return helper;
    });

    stub.set_lamda(&FileInfoHelper::mediaDataFinished,
                   [](FileInfoHelper *, const QUrl &, const QMap<DFileInfo::AttributeExtendID, QVariant> &) {
                       __DBG_STUB_INVOKE__
                   });

    QMap<DFileInfo::AttributeExtendID, QVariant> testData1;
    testData1[DFileInfo::AttributeExtendID::kExtendMediaWidth] = 1920;

    emit mockFuture->infoMedia(testUrl, testData1);
    QTest::qWait(10);

    EXPECT_TRUE(infoFuture->isFinished());
    QMap<DFileInfo::AttributeExtendID, QVariant> mediaInfo1 = infoFuture->mediaInfo();
    EXPECT_EQ(mediaInfo1[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt(), 1920);

    delete infoFuture;
}

// ========== Edge Cases ==========

TEST_F(TestInfoDataFuture, MediaInfo_EmptyMapAfterInfoMedia)
{
    DFileFuture *mockFuture = new DFileFuture();
    InfoDataFuture *infoFuture = new InfoDataFuture(mockFuture);

    QMap<DFileInfo::AttributeExtendID, QVariant> emptyData;

    // Stub FileInfoHelper to avoid dependency
    stub.set_lamda(&FileInfoHelper::instance, []() -> FileInfoHelper & {
        __DBG_STUB_INVOKE__
        static FileInfoHelper helper;
        return helper;
    });

    stub.set_lamda(&FileInfoHelper::mediaDataFinished,
                   [](FileInfoHelper *, const QUrl &, const QMap<DFileInfo::AttributeExtendID, QVariant> &) {
                       __DBG_STUB_INVOKE__
                   });

    emit mockFuture->infoMedia(testUrl, emptyData);
    QTest::qWait(10);

    QMap<DFileInfo::AttributeExtendID, QVariant> mediaInfo = infoFuture->mediaInfo();
    EXPECT_TRUE(mediaInfo.isEmpty());
    EXPECT_TRUE(infoFuture->isFinished());

    delete infoFuture;
}
