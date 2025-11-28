// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "appendcompress/appendcompresseventreceiver.h"
#include "appendcompress/appendcompresshelper.h"

#include <dfm-framework/dpf.h>

#include <QMimeData>
#include <QUrl>
#include <QPoint>
#include <QVariantHash>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DPF_USE_NAMESPACE

class UT_AppendCompressEventReceiver : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        receiver = new AppendCompressEventReceiver();
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete receiver;
        receiver = nullptr;
    }

    stub_ext::StubExt stub;
    AppendCompressEventReceiver *receiver { nullptr };
};

/**
 * @brief Test initEventConnect method
 * Verifies that event connections are properly initialized
 */
TEST_F(UT_AppendCompressEventReceiver, initEventConnect_WorkspaceHooks_Success)
{
    __DBG_STUB_INVOKE__

    // Stub dpfHookSequence->follow to prevent actual hook registration
    bool workspaceFollowCalled = false;
    typedef bool (AppendCompressEventReceiver::*HookFunc)(const QList<QUrl> &, const QUrl &, Qt::DropAction *);
    typedef bool (EventSequenceManager::*HookType)(const QString &, const QString &, AppendCompressEventReceiver *, HookFunc);
    stub.set_lamda(static_cast<HookType>(&EventSequenceManager::follow),
                   [&workspaceFollowCalled] {
                       __DBG_STUB_INVOKE__
                       workspaceFollowCalled = true;
                       return true;
                   });

    stub.set_lamda(ADDR(DPF_NAMESPACE::Event, eventType),
                   [](DPF_NAMESPACE::Event *, const QString &, const QString &) -> int {
                       __DBG_STUB_INVOKE__
                       return DPF_NAMESPACE::EventTypeScope::kInValid;
                   });

    receiver->initEventConnect();

    EXPECT_TRUE(workspaceFollowCalled);
}

/**
 * @brief Test handleSetMouseStyle with valid parameters
 * Verifies that mouse style is set correctly
 */
TEST_F(UT_AppendCompressEventReceiver, handleSetMouseStyle_ValidParams_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");
    Qt::DropAction action = Qt::CopyAction;

    stub.set_lamda(ADDR(AppendCompressHelper, setMouseStyle),
                   [](const QUrl &, const QList<QUrl> &, Qt::DropAction *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleSetMouseStyle(fromUrls, toUrl, &action);

    EXPECT_TRUE(result);
}

/**
 * @brief Test handleSetMouseStyle returns false when helper fails
 */
TEST_F(UT_AppendCompressEventReceiver, handleSetMouseStyle_HelperFails_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/invalid.txt");
    Qt::DropAction action = Qt::CopyAction;

    stub.set_lamda(ADDR(AppendCompressHelper, setMouseStyle),
                   [](const QUrl &, const QList<QUrl> &, Qt::DropAction *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = receiver->handleSetMouseStyle(fromUrls, toUrl, &action);

    EXPECT_FALSE(result);
}

/**
 * @brief Test handleDragDropCompress with valid parameters
 * Verifies that drag-drop compression is handled correctly
 */
TEST_F(UT_AppendCompressEventReceiver, handleDragDropCompress_ValidParams_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt"),
                             QUrl::fromLocalFile("/tmp/file2.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(AppendCompressHelper, dragDropCompress),
                   [](const QUrl &, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleDragDropCompress(fromUrls, toUrl);

    EXPECT_TRUE(result);
}

/**
 * @brief Test handleDragDropCompress returns false when helper fails
 */
TEST_F(UT_AppendCompressEventReceiver, handleDragDropCompress_HelperFails_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/invalid.txt");

    stub.set_lamda(ADDR(AppendCompressHelper, dragDropCompress),
                   [](const QUrl &, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = receiver->handleDragDropCompress(fromUrls, toUrl);

    EXPECT_FALSE(result);
}

/**
 * @brief Test handleSetMouseStyleOnDesktop with valid extData
 */
TEST_F(UT_AppendCompressEventReceiver, handleSetMouseStyleOnDesktop_ValidExtData_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    Qt::DropAction action = Qt::CopyAction;
    QVariantHash extData;
    extData["hoverUrl"] = QUrl::fromLocalFile("/tmp/archive.zip");
    extData["dropAction"] = reinterpret_cast<qint64>(&action);

    stub.set_lamda(ADDR(AppendCompressHelper, setMouseStyle),
                   [](const QUrl &, const QList<QUrl> &, Qt::DropAction *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleSetMouseStyleOnDesktop(0, &mimeData, QPoint(0, 0), &extData);

    EXPECT_TRUE(result);
}

/**
 * @brief Test handleSetMouseStyleOnDesktop with null extData
 */
TEST_F(UT_AppendCompressEventReceiver, handleSetMouseStyleOnDesktop_NullExtData_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    bool result = receiver->handleSetMouseStyleOnDesktop(0, &mimeData, QPoint(0, 0), nullptr);

    EXPECT_FALSE(result);
}

/**
 * @brief Test handleSetMouseStyleOnDesktop with null dropAction pointer
 */
TEST_F(UT_AppendCompressEventReceiver, handleSetMouseStyleOnDesktop_NullDropAction_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    QVariantHash extData;
    extData["hoverUrl"] = QUrl::fromLocalFile("/tmp/archive.zip");
    extData["dropAction"] = 0;   // Null pointer

    bool result = receiver->handleSetMouseStyleOnDesktop(0, &mimeData, QPoint(0, 0), &extData);

    EXPECT_FALSE(result);
}

/**
 * @brief Test handleDragDropCompressOnDesktop with valid extData
 */
TEST_F(UT_AppendCompressEventReceiver, handleDragDropCompressOnDesktop_ValidExtData_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    QVariantHash extData;
    extData["dropUrl"] = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(AppendCompressHelper, dragDropCompress),
                   [](const QUrl &, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleDragDropCompressOnDesktop(0, &mimeData, QPoint(0, 0), &extData);

    EXPECT_TRUE(result);
}

/**
 * @brief Test handleDragDropCompressOnDesktop with null extData
 */
TEST_F(UT_AppendCompressEventReceiver, handleDragDropCompressOnDesktop_NullExtData_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    bool result = receiver->handleDragDropCompressOnDesktop(0, &mimeData, QPoint(0, 0), nullptr);

    EXPECT_FALSE(result);
}

/**
 * @brief Test handleSetMouseStyleOnOrganizer with valid extData
 */
TEST_F(UT_AppendCompressEventReceiver, handleSetMouseStyleOnOrganizer_ValidExtData_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    Qt::DropAction action = Qt::CopyAction;
    QVariantHash extData;
    extData["hoverUrl"] = QUrl::fromLocalFile("/tmp/archive.zip");
    extData["dropAction"] = reinterpret_cast<qint64>(&action);

    stub.set_lamda(ADDR(AppendCompressHelper, setMouseStyle),
                   [](const QUrl &, const QList<QUrl> &, Qt::DropAction *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleSetMouseStyleOnOrganizer("view-id", &mimeData, QPoint(0, 0), &extData);

    EXPECT_TRUE(result);
}

/**
 * @brief Test handleSetMouseStyleOnOrganizer with null extData
 */
TEST_F(UT_AppendCompressEventReceiver, handleSetMouseStyleOnOrganizer_NullExtData_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    bool result = receiver->handleSetMouseStyleOnOrganizer("view-id", &mimeData, QPoint(0, 0), nullptr);

    EXPECT_FALSE(result);
}

/**
 * @brief Test handleSetMouseStyleOnOrganizer with null dropAction pointer
 */
TEST_F(UT_AppendCompressEventReceiver, handleSetMouseStyleOnOrganizer_NullDropAction_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    QVariantHash extData;
    extData["hoverUrl"] = QUrl::fromLocalFile("/tmp/archive.zip");
    extData["dropAction"] = 0;   // Null pointer

    bool result = receiver->handleSetMouseStyleOnOrganizer("view-id", &mimeData, QPoint(0, 0), &extData);

    EXPECT_FALSE(result);
}

/**
 * @brief Test handleDragDropCompressOnOsrganizer with valid extData
 */
TEST_F(UT_AppendCompressEventReceiver, handleDragDropCompressOnOsrganizer_ValidExtData_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    QVariantHash extData;
    extData["dropUrl"] = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(AppendCompressHelper, dragDropCompress),
                   [](const QUrl &, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleDragDropCompressOnOsrganizer("view-id", &mimeData, QPoint(0, 0), &extData);

    EXPECT_TRUE(result);
}

/**
 * @brief Test handleDragDropCompressOnOsrganizer with null extData
 */
TEST_F(UT_AppendCompressEventReceiver, handleDragDropCompressOnOsrganizer_NullExtData_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/file1.txt") });

    bool result = receiver->handleDragDropCompressOnOsrganizer("view-id", &mimeData, QPoint(0, 0), nullptr);

    EXPECT_FALSE(result);
}

/**
 * @brief Test handleIsDrop with compressed file
 */
TEST_F(UT_AppendCompressEventReceiver, handleIsDrop_CompressedFile_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    QUrl url = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(AppendCompressHelper, isCompressedFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = receiver->handleIsDrop(url);

    EXPECT_TRUE(result);
}

/**
 * @brief Test handleIsDrop with non-compressed file
 */
TEST_F(UT_AppendCompressEventReceiver, handleIsDrop_NonCompressedFile_ReturnsFalse)
{
    __DBG_STUB_INVOKE__

    QUrl url = QUrl::fromLocalFile("/tmp/document.txt");

    stub.set_lamda(ADDR(AppendCompressHelper, isCompressedFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = receiver->handleIsDrop(url);

    EXPECT_FALSE(result);
}

/**
 * @brief Test constructor creates valid object
 */
TEST_F(UT_AppendCompressEventReceiver, Constructor_CreatesValidObject)
{
    __DBG_STUB_INVOKE__

    AppendCompressEventReceiver *testReceiver = new AppendCompressEventReceiver();

    EXPECT_NE(testReceiver, nullptr);

    delete testReceiver;
}
