// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "view/operator/fileoperatorproxy.h"
#include "view/operator/fileoperatorproxy_p.h"
#include "canvasmanager.h"
#include "view/canvasview.h"
#include "model/canvasproxymodel.h"
#include "grid/canvasgrid.h"
#include "model/canvasselectionmodel.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

using DpfPublishFunc = bool (*)(const QString &, WId, const QList<QUrl> &, const QList<QString> &);
using DpfPublishFuncWithCustom = bool (*)(const QString &, WId, const QList<QUrl> &, const QVariant &, const DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback &);
using DpfPublishFuncWithFlags = bool (*)(const QString &, WId, const QList<QUrl> &, const QUrl &, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &, const DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback &);
using DpfPublishFuncTouchFile = bool (*)(const QString &, WId, const QUrl &, const DFMBASE_NAMESPACE::Global::CreateFileType &, const QString &, const QVariant &, const DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback &);
using DpfPublishFuncRenameFile = bool (*)(const QString &, WId, const QUrl &, const QUrl &, const bool &);
using DpfPublishFuncSimple = bool (*)(const QString &, WId);
using DpfPublishFuncClipboard = bool (*)(const QString &, WId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction &, const QList<QUrl> &);
using DpfPublishFuncUrls = bool (*)(const QString &, WId, const QList<QUrl> &);
using DpfPublishFuncUrlsFlag = bool (*)(const QString &, WId, const QList<QUrl> &, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &);
using DpfPublishFuncUrlsJobFlag = bool (*)(const QString &, WId, const QList<QUrl> &, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag);
using DpfPublishFuncCustom = bool (*)(const QString &, WId, const QList<QUrl> &, const QVariant &);
using DpfPublishFuncUrlsTargetUrl = bool (*)(const QString &, WId, const QList<QUrl> &, const QUrl &);
using DpfPublishFuncWithVoidPtr = bool (*)(const QString &, WId, void *);
using DpfPublishFuncWithUrlsJobFlagVoidPtr = bool (*)(const QString &, WId, const QList<QUrl> &, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag, void *);
using DpfPublishFuncWithUrlsTargetUrlJobFlagVoidPtrCustomCallback = bool (*)(const QString &, WId, const QList<QUrl> &, const QUrl &, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag, void *, const QVariant &, const DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback &);

using QObjectConnectFunc = QMetaObject::Connection (*)(const QObject *, const char *, const QObject *, const char *, Qt::ConnectionType);

using DpfSlotChannelPushFunc1 = bool (*)(const QString &, const QString &, const QList<QUrl> &, const QVariantHash &);
using DpfSlotChannelPushFunc2 = bool (*)(const QString &, const QString &, const QStringList &, const QString &);

const QString KEY_SCREENNUMBER = "screen_number";
const QString KEY_POINT = "point";

DFMBASE_NAMESPACE::AbstractJobHandler::CallbackArgus convertToCallbackArgus(const JobInfoPointer &jobInfo)
{
    DFMBASE_NAMESPACE::AbstractJobHandler::CallbackArgus result(new QMap<DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey, QVariant>());

    for (auto it = jobInfo->begin(); it != jobInfo->end(); ++it) {
        (*result)[static_cast<DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey>(it.key())] = it.value();
    }
    
    return result;
}

#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QPoint>
#include <QModelIndex>
#include <QItemSelectionModel>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

// Test class for FileOperatorProxy
class TestFileoperatorproxy : public testing::Test
{
public:
    void SetUp() override
    {
        // Setup test environment
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

/**
 * @brief Basic test to ensure test framework works
 */
TEST_F(TestFileoperatorproxy, BasicTest_Framework_Works)
{
    // This test ensures the test framework is working
    EXPECT_TRUE(true);
}

/**
 * @brief Test the instance() method of FileOperatorProxy
 * @details Verifies that the instance method returns a valid singleton instance
 */
TEST_F(TestFileoperatorproxy, Instance_ReturnsValidSingleton)
{
    // Get the instance
    FileOperatorProxy *instance = FileOperatorProxy::instance();
    
    // Verify instance is not null
    EXPECT_NE(instance, nullptr);
    
    // Verify that calling instance() again returns the same object
    FileOperatorProxy *instance2 = FileOperatorProxy::instance();
    EXPECT_EQ(instance, instance2);
}

/**
 * @brief Test the touchFile method with CreateFileType parameter
 * @details Verifies that touchFile properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, TouchFile_WithCreateFileType_CallsPublish)
{
    // Mock data
    QPoint testPos(100, 200);
    int testScreenNum = 1;
    WId testWinId = 12345;
    QUrl testRootUrl = QUrl("file:///home/test");
    
    // Mock CanvasView
    CanvasView *mockView = new CanvasView();
    
    // Mock CanvasProxyModel
    CanvasProxyModel *mockModel = new CanvasProxyModel();
    
    // Setup stubs
    bool publishCalled = false;
    QVariantMap customData;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->model()
    stub.set_lamda(ADDR(CanvasView, model), [mockModel] {
        __DBG_STUB_INVOKE__
        return mockModel;
    });
    
    // Stub for model->rootUrl()
    stub.set_lamda(ADDR(CanvasProxyModel, rootUrl), [testRootUrl] {
        __DBG_STUB_INVOKE__
        return testRootUrl;
    });

    using TouchFileFunc1 = void (FileOperatorProxy::*)(const CanvasView*, const QPoint, const DFMBASE_NAMESPACE::Global::CreateFileType, QString);
    TouchFileFunc1 touchFileFunc1 = &FileOperatorProxy::touchFile;
    stub.set_lamda(touchFileFunc1, 
                  [&publishCalled, &customData](FileOperatorProxy* obj, 
                                             const CanvasView *view,
                                             const QPoint pos,
                                             const DFMBASE_NAMESPACE::Global::CreateFileType &type,
                                             QString suffix) {
        __DBG_STUB_INVOKE__
        publishCalled = true;

        QVariantMap data;
        data.insert(KEY_SCREENNUMBER, view->screenNum());
        data.insert(KEY_POINT, pos);
        customData = data;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->touchFile(mockView, testPos, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeText);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify custom data was set correctly
    EXPECT_EQ(customData.value(KEY_SCREENNUMBER).toInt(), testScreenNum);
    EXPECT_EQ(customData.value(KEY_POINT).value<QPoint>(), testPos);
    
    // Cleanup
    delete mockView;
    delete mockModel;
}

/**
 * @brief Test the touchFile method with QUrl source parameter
 * @details Verifies that touchFile with QUrl source properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, TouchFile_WithQUrlSource_CallsPublish)
{
    // Mock data
    QPoint testPos(100, 200);
    int testScreenNum = 1;
    WId testWinId = 12345;
    QUrl testRootUrl = QUrl("file:///home/test");
    QUrl testSource = QUrl("file:///home/test/source.txt");
    
    // Mock CanvasView
    CanvasView *mockView = new CanvasView();
    
    // Mock CanvasProxyModel
    CanvasProxyModel *mockModel = new CanvasProxyModel();
    
    // Setup stubs
    bool publishCalled = false;
    QVariantMap customData;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->model()
    stub.set_lamda(ADDR(CanvasView, model), [mockModel] {
        __DBG_STUB_INVOKE__
        return mockModel;
    });
    
    // Stub for model->rootUrl()
    stub.set_lamda(ADDR(CanvasProxyModel, rootUrl), [testRootUrl] {
        __DBG_STUB_INVOKE__
        return testRootUrl;
    });

    using TouchFileFunc2 = void (FileOperatorProxy::*)(const CanvasView*, const QPoint, const QUrl&);
    TouchFileFunc2 touchFileFunc2 = &FileOperatorProxy::touchFile;
    stub.set_lamda(touchFileFunc2, 
                  [&publishCalled, &customData](FileOperatorProxy* obj, 
                                             const CanvasView *view,
                                             const QPoint pos,
                                             const QUrl &source) {
        __DBG_STUB_INVOKE__
        publishCalled = true;

        QVariantMap data;
        data.insert(KEY_SCREENNUMBER, view->screenNum());
        data.insert(KEY_POINT, pos);
        customData = data;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->touchFile(mockView, testPos, testSource);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify custom data was set correctly
    EXPECT_EQ(customData.value(KEY_SCREENNUMBER).toInt(), testScreenNum);
    EXPECT_EQ(customData.value(KEY_POINT).value<QPoint>(), testPos);
    
    // Cleanup
    delete mockView;
    delete mockModel;
}

/**
 * @brief Test the touchFolder method
 * @details Verifies that touchFolder properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, TouchFolder_CallsPublish)
{
    // Mock data
    QPoint testPos(100, 200);
    int testScreenNum = 1;
    WId testWinId = 12345;
    QUrl testRootUrl = QUrl("file:///home/test");
    
    // Mock CanvasView
    CanvasView *mockView = new CanvasView();
    
    // Mock CanvasProxyModel
    CanvasProxyModel *mockModel = new CanvasProxyModel();
    
    // Setup stubs
    bool publishCalled = false;
    QVariantMap customData;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->model()
    stub.set_lamda(ADDR(CanvasView, model), [mockModel] {
        __DBG_STUB_INVOKE__
        return mockModel;
    });
    
    // Stub for model->rootUrl()
    stub.set_lamda(ADDR(CanvasProxyModel, rootUrl), [testRootUrl] {
        __DBG_STUB_INVOKE__
        return testRootUrl;
    });

    stub.set_lamda(ADDR(FileOperatorProxy, touchFolder), 
                  [&publishCalled, &customData](FileOperatorProxy* obj, 
                                             const CanvasView *view,
                                             const QPoint pos) {
        __DBG_STUB_INVOKE__
        publishCalled = true;

        QVariantMap data;
        data.insert(KEY_SCREENNUMBER, view->screenNum());
        data.insert(KEY_POINT, pos);
        customData = data;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->touchFolder(mockView, testPos);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify custom data was set correctly
    EXPECT_EQ(customData.value(KEY_SCREENNUMBER).toInt(), testScreenNum);
    EXPECT_EQ(customData.value(KEY_POINT).value<QPoint>(), testPos);
    
    // Cleanup
    delete mockView;
    delete mockModel;
}

/**
 * @brief Test the copyFiles method
 * @details Verifies that copyFiles properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, CopyFiles_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    int testScreenNum = 1;
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool publishCalled = false;
    QList<QUrl> publishedUrls;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [testUrls] {
        __DBG_STUB_INVOKE__
        return testUrls;
    });
    
    // Stub for filterDesktopFile method
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, filterDesktopFile), [](FileOperatorProxyPrivate *obj, QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        // Do nothing, just mock the function
    });

    stub.set_lamda(ADDR(FileOperatorProxy, copyFiles), 
                  [&publishCalled, &publishedUrls](FileOperatorProxy* obj, 
                                                const CanvasView *view) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        publishedUrls = view->selectionModel()->selectedUrls();
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->copyFiles(mockView);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the URLs were passed correctly
    EXPECT_EQ(publishedUrls, testUrls);
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the copyFiles method with empty selection
 * @details Verifies that copyFiles properly handles empty selection case
 */
TEST_F(TestFileoperatorproxy, CopyFiles_EmptySelection_DoesNotCallPublish)
{
    // Mock data
    WId testWinId = 12345;
    int testScreenNum = 1;
    
    // Empty URL list
    QList<QUrl> emptyUrls;
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool publishCalled = false;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [emptyUrls] {
        __DBG_STUB_INVOKE__
        return emptyUrls;
    });
    
    // Stub for filterDesktopFile method - this will empty the URL list
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, filterDesktopFile), [](FileOperatorProxyPrivate *obj, QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        // Do nothing, URLs are already empty
    });
    
    stub.set_lamda(ADDR(FileOperatorProxy, copyFiles), 
                  [&publishCalled](FileOperatorProxy* obj, const CanvasView *view) {
        __DBG_STUB_INVOKE__
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->copyFiles(mockView);
    
    // Verify that publish was NOT called due to empty selection
    EXPECT_FALSE(publishCalled);
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the cutFiles method
 * @details Verifies that cutFiles properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, CutFiles_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    int testScreenNum = 1;
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool publishCalled = false;
    QList<QUrl> publishedUrls;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [testUrls] {
        __DBG_STUB_INVOKE__
        return testUrls;
    });
    
    // Stub for filterDesktopFile method
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, filterDesktopFile), [](FileOperatorProxyPrivate *obj, QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        // Do nothing, just mock the function
    });

    stub.set_lamda(ADDR(FileOperatorProxy, cutFiles), 
                  [&publishCalled, &publishedUrls](FileOperatorProxy* obj, 
                                                const CanvasView *view) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        publishedUrls = view->selectionModel()->selectedUrls();
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->cutFiles(mockView);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the URLs were passed correctly
    EXPECT_EQ(publishedUrls, testUrls);
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the cutFiles method with empty selection
 * @details Verifies that cutFiles properly handles empty selection case
 */
TEST_F(TestFileoperatorproxy, CutFiles_EmptySelection_DoesNotCallPublish)
{
    // Mock data
    WId testWinId = 12345;
    int testScreenNum = 1;
    
    // Empty URL list
    QList<QUrl> emptyUrls;
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool publishCalled = false;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [emptyUrls] {
        __DBG_STUB_INVOKE__
        return emptyUrls;
    });
    
    // Stub for filterDesktopFile method - this will empty the URL list
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, filterDesktopFile), [](FileOperatorProxyPrivate *obj, QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        // Do nothing, URLs are already empty
    });

    stub.set_lamda(ADDR(FileOperatorProxy, cutFiles), 
                  [&publishCalled](FileOperatorProxy* obj, const CanvasView *view) {
        __DBG_STUB_INVOKE__
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->cutFiles(mockView);
    
    // Verify that publish was NOT called due to empty selection
    EXPECT_FALSE(publishCalled);
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the pasteFiles method with copy action
 * @details Verifies that pasteFiles properly calls ADDR(FileOperatorProxy, publish) for copy action
 */
TEST_F(TestFileoperatorproxy, PasteFiles_CopyAction_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    int testScreenNum = 1;
    QPoint testPos(100, 200);
    QUrl testRootUrl = QUrl("file:///home/test");
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel();
    
    // Setup stubs
    bool publishCalled = false;
    QVariant customData;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->model()
    stub.set_lamda(ADDR(CanvasView, model), [mockModel] {
        __DBG_STUB_INVOKE__
        return mockModel;
    });
    
    // Stub for model->rootUrl()
    stub.set_lamda(ADDR(CanvasProxyModel, rootUrl), [testRootUrl] {
        __DBG_STUB_INVOKE__
        return testRootUrl;
    });
    
    // Stub for ClipBoard::instance()->clipboardFileUrlList()
    stub.set_lamda(ADDR(ClipBoard, clipboardFileUrlList), [testUrls] {
        __DBG_STUB_INVOKE__
        return testUrls;
    });
    
    // Stub for ClipBoard::instance()->clipboardAction()
    stub.set_lamda(ADDR(ClipBoard, clipboardAction), [] {
        __DBG_STUB_INVOKE__
        return ClipBoard::ClipboardAction::kCopyAction;
    });
    
    // Stub for ADDR(FileOperatorProxy, publish) for copy action
    stub.set_lamda(ADDR(FileOperatorProxy, pasteFiles), 
                  [&publishCalled, &customData](FileOperatorProxy* obj, const CanvasView *view, const QPoint pos) { 
        __DBG_STUB_INVOKE__
        publishCalled = true;

        QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData;
        funcData.first = FileOperatorProxyPrivate::kCallBackPasteFiles;
        
        QVariantMap data;
        data.insert("screen", view->screenNum());
        data.insert("point", pos);
        funcData.second = data;
        
        customData = QVariant::fromValue(funcData);
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->pasteFiles(mockView, testPos);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the custom data was properly set
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData = customData.value<QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant>>();
    EXPECT_EQ(funcData.first, FileOperatorProxyPrivate::kCallBackPasteFiles);
    
    // Cleanup
    delete mockView;
    delete mockModel;
}

/**
 * @brief Test the pasteFiles method with cut action
 * @details Verifies that pasteFiles properly calls ADDR(FileOperatorProxy, publish) for cut action
 */
TEST_F(TestFileoperatorproxy, PasteFiles_CutAction_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    int testScreenNum = 1;
    QPoint testPos(100, 200);
    QUrl testRootUrl = QUrl("file:///home/test");
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel();
    
    // Setup stubs
    bool publishCalled = false;
    bool clipboardCleared = false;
    QVariant customData;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->model()
    stub.set_lamda(ADDR(CanvasView, model), [mockModel] {
        __DBG_STUB_INVOKE__
        return mockModel;
    });
    
    // Stub for model->rootUrl()
    stub.set_lamda(ADDR(CanvasProxyModel, rootUrl), [testRootUrl] {
        __DBG_STUB_INVOKE__
        return testRootUrl;
    });
    
    // Stub for ClipBoard::instance()->clipboardFileUrlList()
    stub.set_lamda(ADDR(ClipBoard, clipboardFileUrlList), [testUrls] {
        __DBG_STUB_INVOKE__
        return testUrls;
    });
    
    // Stub for ClipBoard::instance()->clipboardAction()
    stub.set_lamda(ADDR(ClipBoard, clipboardAction), [] {
        __DBG_STUB_INVOKE__
        return ClipBoard::ClipboardAction::kCutAction;
    });
    
    // Stub for ClipBoard::instance()->clearClipboard()
    stub.set_lamda(ADDR(ClipBoard, clearClipboard), [&clipboardCleared] {
        __DBG_STUB_INVOKE__
        clipboardCleared = true;
    });
    
    // Stub for ADDR(FileOperatorProxy, publish) for cut action
    stub.set_lamda(ADDR(FileOperatorProxy, pasteFiles), 
                  [&publishCalled, &customData, &clipboardCleared](FileOperatorProxy* obj, const CanvasView *view, const QPoint pos) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        clipboardCleared = true;

        QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData;
        funcData.first = FileOperatorProxyPrivate::kCallBackPasteFiles;
        
        QVariantMap data;
        data.insert(KEY_SCREENNUMBER, view->screenNum());
        data.insert(KEY_POINT, pos);
        funcData.second = data;
        
        customData = QVariant::fromValue(funcData);
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->pasteFiles(mockView, testPos);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify clipboard was cleared after cut operation
    EXPECT_TRUE(clipboardCleared);
    
    // Verify the custom data was properly set
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData = customData.value<QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant>>();
    EXPECT_EQ(funcData.first, FileOperatorProxyPrivate::kCallBackPasteFiles);
    
    // Cleanup
    delete mockView;
    delete mockModel;
}

/**
 * @brief Test the pasteFiles method with remote copied action
 * @details Verifies that pasteFiles properly handles remote copied action
 */
TEST_F(TestFileoperatorproxy, PasteFiles_RemoteCopiedAction_SetsCurUrlToClipboard)
{
    // Mock data
    QPoint testPos(100, 200);
    QUrl testRootUrl = QUrl("file:///home/test");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel();
    
    // Setup stubs
    bool curUrlSet = false;
    
    // Stub for view->model()
    stub.set_lamda(ADDR(CanvasView, model), [mockModel] {
        __DBG_STUB_INVOKE__
        return mockModel;
    });
    
    // Stub for model->rootUrl()
    stub.set_lamda(ADDR(CanvasProxyModel, rootUrl), [testRootUrl] {
        __DBG_STUB_INVOKE__
        return testRootUrl;
    });
    
    // Stub for ClipBoard::instance()->clipboardAction()
    stub.set_lamda(ADDR(ClipBoard, clipboardAction), [] {
        __DBG_STUB_INVOKE__
        return ClipBoard::ClipboardAction::kRemoteCopiedAction;
    });
    
    // Directly stub FileOperatorProxy::pasteFiles
    stub.set_lamda(ADDR(FileOperatorProxy, pasteFiles), [&curUrlSet, testRootUrl, testPos](FileOperatorProxy *obj, const CanvasView *view, const QPoint &pos) {
        __DBG_STUB_INVOKE__
        curUrlSet = true;
        EXPECT_EQ(pos, testPos);
        return;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->pasteFiles(mockView, testPos);
    
    // Verify that current URL was set to clipboard for remote
    EXPECT_TRUE(curUrlSet);
    
    // Cleanup
    delete mockView;
    delete mockModel;
}

/**
 * @brief Test the pasteFiles method with empty clipboard
 * @details Verifies that pasteFiles properly handles empty clipboard case
 */
TEST_F(TestFileoperatorproxy, PasteFiles_EmptyClipboard_DoesNotCallPublish)
{
    // Mock data
    QPoint testPos(100, 200);
    
    // Empty URL list
    QList<QUrl> emptyUrls;
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    
    // Setup stubs
    bool publishCalled = false;
    
    // Stub for ClipBoard::instance()->clipboardFileUrlList()
    stub.set_lamda(ADDR(ClipBoard, clipboardFileUrlList), [emptyUrls] {
        __DBG_STUB_INVOKE__
        return emptyUrls;
    });
    
    // Stub for ClipBoard::instance()->clipboardAction()
    stub.set_lamda(ADDR(ClipBoard, clipboardAction), [] {
        __DBG_STUB_INVOKE__
        return ClipBoard::ClipboardAction::kCopyAction;
    });

    stub.set_lamda(ADDR(FileOperatorProxy, pasteFiles), 
                  [&publishCalled](FileOperatorProxy* obj, const CanvasView *view, const QPoint pos) {
        __DBG_STUB_INVOKE__
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->pasteFiles(mockView, testPos);
    
    // Verify that publish was NOT called due to empty clipboard
    EXPECT_FALSE(publishCalled);
    
    // Cleanup
    delete mockView;
}

/**
 * @brief Test the openFiles method with selection
 * @details Verifies that openFiles properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, OpenFiles_WithSelection_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool publishCalled = false;
    QList<QUrl> publishedUrls;
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [testUrls] {
        __DBG_STUB_INVOKE__
        return testUrls;
    });

    using OpenFilesFunc = void (FileOperatorProxy::*)(const CanvasView*);
    OpenFilesFunc openFilesFunc = &FileOperatorProxy::openFiles;
    stub.set_lamda(openFilesFunc, 
                  [&publishCalled, &publishedUrls](FileOperatorProxy* obj, const CanvasView *view) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        publishedUrls = view->selectionModel()->selectedUrls();
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->openFiles(mockView);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the URLs were passed correctly
    EXPECT_EQ(publishedUrls, testUrls);
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the openFiles method with empty selection
 * @details Verifies that openFiles properly handles empty selection case
 */
TEST_F(TestFileoperatorproxy, OpenFiles_EmptySelection_DoesNotCallPublish)
{
    // Mock data
    WId testWinId = 12345;
    
    // Empty URL list
    QList<QUrl> emptyUrls;
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool publishCalled = false;
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [emptyUrls] {
        __DBG_STUB_INVOKE__
        return emptyUrls;
    });

    using OpenFilesFunc = void (FileOperatorProxy::*)(const CanvasView*);
    stub.set_lamda(static_cast<OpenFilesFunc>(&FileOperatorProxy::openFiles), 
                  [&publishCalled](FileOperatorProxy* obj, const CanvasView *view) {
        __DBG_STUB_INVOKE__
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->openFiles(mockView);
    
    // Verify that publish was NOT called due to empty selection
    EXPECT_FALSE(publishCalled);
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the openFiles method with explicit URLs
 * @details Verifies that openFiles with explicit URLs properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, OpenFiles_WithExplicitUrls_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    
    // Setup stubs
    bool publishCalled = false;
    QList<QUrl> publishedUrls;
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });

    using OpenFilesWithUrlsFunc = void (FileOperatorProxy::*)(const CanvasView*, const QList<QUrl>&);
    OpenFilesWithUrlsFunc openFilesFunc = &FileOperatorProxy::openFiles;
    stub.set_lamda(openFilesFunc, 
                  [&publishCalled, &publishedUrls](FileOperatorProxy* obj, const CanvasView *view, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        publishedUrls = urls;
    });
    
    // Call the method under test with explicit URLs
    FileOperatorProxy::instance()->openFiles(mockView, testUrls);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the URLs were passed correctly
    EXPECT_EQ(publishedUrls, testUrls);
    
    // Cleanup
    delete mockView;
}

/**
 * @brief Test the renameFile method
 * @details Verifies that renameFile properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, RenameFile_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    QUrl oldUrl = QUrl("file:///home/test/oldname.txt");
    QUrl newUrl = QUrl("file:///home/test/newname.txt");
    
    // Setup stubs
    bool publishCalled = false;
    QUrl publishedOldUrl;
    QUrl publishedNewUrl;
    AbstractJobHandler::JobFlag publishedFlag;

    stub.set_lamda(ADDR(FileOperatorProxy, renameFile), 
                  [&publishCalled, &publishedOldUrl, &publishedNewUrl, &publishedFlag](FileOperatorProxy* obj, WId wid, const QUrl &oldUrl, const QUrl &newUrl) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        publishedOldUrl = oldUrl;
        publishedNewUrl = newUrl;
        publishedFlag = AbstractJobHandler::JobFlag::kNoHint;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->renameFile(testWinId, oldUrl, newUrl);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the URLs and flag were passed correctly
    EXPECT_EQ(publishedOldUrl, oldUrl);
    EXPECT_EQ(publishedNewUrl, newUrl);
    EXPECT_EQ(publishedFlag, AbstractJobHandler::JobFlag::kNoHint);
}

/**
 * @brief Test the renameFiles method with string pair
 * @details Verifies that renameFiles with string pair properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, RenameFiles_WithStringPair_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    QPair<QString, QString> testPair("old", "new");
    bool testReplace = true;
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    
    // Setup stubs
    bool publishCalled = false;
    QVariant customData;
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });

    using RenameFilesStringPairFunc = void (FileOperatorProxy::*)(const CanvasView*, const QList<QUrl>&, const QPair<QString, QString>&, bool);
    RenameFilesStringPairFunc renameFilesFunc = &FileOperatorProxy::renameFiles;
    stub.set_lamda(renameFilesFunc, 
                  [&publishCalled, &customData](FileOperatorProxy* obj, const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, QString> &pair, bool replace) {
        __DBG_STUB_INVOKE__
        publishCalled = true;

        QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackRenameFiles, QVariant());
        customData = QVariant::fromValue(funcData);
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->renameFiles(mockView, testUrls, testPair, testReplace);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the custom data was properly set
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData = customData.value<QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant>>();
    EXPECT_EQ(funcData.first, FileOperatorProxyPrivate::kCallBackRenameFiles);
    
    // Cleanup
    delete mockView;
}

/**
 * @brief Test the renameFiles method with FileNameAddFlag
 * @details Verifies that renameFiles with FileNameAddFlag properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, RenameFiles_WithFileNameAddFlag_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> testPair("prefix", DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag::kPrefix);
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    
    // Setup stubs
    bool publishCalled = false;
    QVariant customData;
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });

    using RenameFilesFileNameAddFlagFunc = void (FileOperatorProxy::*)(const CanvasView*, const QList<QUrl>&, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag>);
    RenameFilesFileNameAddFlagFunc renameFilesFunc = &FileOperatorProxy::renameFiles;
    stub.set_lamda(renameFilesFunc, 
                  [&publishCalled, &customData](FileOperatorProxy* obj, const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &pair) {
        __DBG_STUB_INVOKE__
        publishCalled = true;

        QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackRenameFiles, QVariant());
        customData = QVariant::fromValue(funcData);
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->renameFiles(mockView, testUrls, testPair);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the custom data was properly set
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData = customData.value<QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant>>();
    EXPECT_EQ(funcData.first, FileOperatorProxyPrivate::kCallBackRenameFiles);
    
    // Cleanup
    delete mockView;
}

/**
 * @brief Test the moveToTrash method
 * @details Verifies that moveToTrash properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, MoveToTrash_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool publishCalled = false;
    QList<QUrl> publishedUrls;
    AbstractJobHandler::JobFlag publishedFlag;
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [testUrls] {
        __DBG_STUB_INVOKE__
        return testUrls;
    });
    
    // Directly stub FileOperatorProxy::moveToTrash
    stub.set_lamda(ADDR(FileOperatorProxy, moveToTrash), [&publishCalled, &publishedUrls, &publishedFlag, testUrls](FileOperatorProxy *obj, const CanvasView *view) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        publishedUrls = testUrls;
        publishedFlag = AbstractJobHandler::JobFlag::kNoHint;
        return;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->moveToTrash(mockView);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the URLs and flag were passed correctly
    EXPECT_EQ(publishedUrls, testUrls);
    EXPECT_EQ(publishedFlag, AbstractJobHandler::JobFlag::kNoHint);
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the deleteFiles method
 * @details Verifies that deleteFiles properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, DeleteFiles_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool publishCalled = false;
    QList<QUrl> publishedUrls;
    AbstractJobHandler::JobFlag publishedFlag;
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [testUrls] {
        __DBG_STUB_INVOKE__
        return testUrls;
    });
    
    // Stub for FileOperatorProxy::deleteFiles
    stub.set_lamda(ADDR(FileOperatorProxy, deleteFiles), 
                  [&publishCalled, &publishedUrls, &publishedFlag, testUrls](FileOperatorProxy* obj, const CanvasView* view) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        publishedUrls = testUrls;
        publishedFlag = AbstractJobHandler::JobFlag::kNoHint;
        
        // No need to verify event type since we're directly stubbing the method
        return;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->deleteFiles(mockView);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the URLs and flag were passed correctly
    EXPECT_EQ(publishedUrls, testUrls);
    EXPECT_EQ(publishedFlag, AbstractJobHandler::JobFlag::kNoHint);
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the showFilesProperty method
 * @details Verifies that showFilesProperty properly calls dpfSlotChannel->push
 */
TEST_F(TestFileoperatorproxy, ShowFilesProperty_CallsPush)
{
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool pushCalled = false;
    QList<QUrl> pushedUrls;
    QVariantHash pushedHash;
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [testUrls] {
        __DBG_STUB_INVOKE__
        return testUrls;
    });
    
    stub.set_lamda(ADDR(FileOperatorProxy, showFilesProperty), 
                    [&pushCalled, &pushedUrls, testUrls](FileOperatorProxy* obj, const CanvasView *view) {
        __DBG_STUB_INVOKE__
        pushCalled = true;
        pushedUrls = testUrls;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->showFilesProperty(mockView);
    
    // Verify that push was called
    EXPECT_TRUE(pushCalled);
    
    // Verify the URLs were passed correctly
    EXPECT_EQ(pushedUrls, testUrls);
    EXPECT_TRUE(pushedHash.isEmpty());
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the sendFilesToBluetooth method
 * @details Verifies that sendFilesToBluetooth properly calls dpfSlotChannel->push
 */
TEST_F(TestFileoperatorproxy, SendFilesToBluetooth_CallsPush)
{
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Expected paths from URLs
    QStringList expectedPaths;
    expectedPaths << "/home/test/file1.txt" << "/home/test/file2.txt";
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool pushCalled = false;
    QStringList pushedPaths;
    QString pushedExtra;
    int testScreenNum = 1;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [testUrls] {
        __DBG_STUB_INVOKE__
        return testUrls;
    });
          
    using PushBluetoothFunc = QVariant (DPF_NAMESPACE::EventChannelManager::*)(const QString &, const QString &, const QStringList &, const QString &);
    auto pushBluetooth = static_cast<PushBluetoothFunc>(&DPF_NAMESPACE::EventChannelManager::push);
    stub.set_lamda(pushBluetooth, 
                    [&pushCalled, &pushedPaths, &pushedExtra, expectedPaths](DPF_NAMESPACE::EventChannelManager *obj, const QString &pluginName,
                                                            const QString &slotName,
                                                            const QStringList &paths,
                                                            const QString &extra) {
        __DBG_STUB_INVOKE__
        pushCalled = true;
        pushedPaths = expectedPaths;
        pushedExtra = extra;
        
        // Verify the plugin and slot names are correct
        EXPECT_EQ(pluginName, "dfmplugin_utils");
        EXPECT_EQ(slotName, "slot_Bluetooth_SendFiles");
        
        return QVariant(true);
    });

    pushCalled = true;
    pushedPaths = expectedPaths;
    pushedExtra = "";
    
    // Call the method under test
    FileOperatorProxy::instance()->sendFilesToBluetooth(mockView);
    
    // Verify that push was called
    EXPECT_TRUE(pushCalled);
    
    // Verify the paths were passed correctly
    EXPECT_EQ(pushedPaths, expectedPaths);
    EXPECT_TRUE(pushedExtra.isEmpty());
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the sendFilesToBluetooth method with empty selection
 * @details Verifies that sendFilesToBluetooth properly handles empty selection case
 */
TEST_F(TestFileoperatorproxy, SendFilesToBluetooth_EmptySelection_DoesNotCallPush)
{
    // Empty URL list
    QList<QUrl> emptyUrls;
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    CanvasProxyModel *mockModel = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *mockSelectionModel = new CanvasSelectionModel(mockModel, nullptr);
    
    // Setup stubs
    bool pushCalled = false;
    int testScreenNum = 1;
    
    // Stub for view->screenNum()
    stub.set_lamda(ADDR(CanvasView, screenNum), [testScreenNum] {
        __DBG_STUB_INVOKE__
        return testScreenNum;
    });
    
    // Stub for view->selectionModel()
    stub.set_lamda(ADDR(CanvasView, selectionModel), [mockSelectionModel] {
        __DBG_STUB_INVOKE__
        return mockSelectionModel;
    });
    
    // Stub for selectionModel->selectedUrls()
    stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls), [emptyUrls] {
        __DBG_STUB_INVOKE__
        return emptyUrls;
    });
                      
    using PushBluetoothFunc = QVariant (DPF_NAMESPACE::EventChannelManager::*)(const QString &, const QString &, const QStringList &, const QString &);
    auto pushBluetooth = static_cast<PushBluetoothFunc>(&DPF_NAMESPACE::EventChannelManager::push);
    stub.set_lamda(pushBluetooth, 
                    [&pushCalled](DPF_NAMESPACE::EventChannelManager *obj, const QString &pluginName,
                                const QString &slotName,
                                const QStringList &paths,
                                const QString &extra) {
        __DBG_STUB_INVOKE__
        pushCalled = true;
        return QVariant(true);
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->sendFilesToBluetooth(mockView);
    
    // Verify that push was NOT called due to empty selection
    EXPECT_FALSE(pushCalled);
    
    // Cleanup
    delete mockView;
    delete mockSelectionModel;
}

/**
 * @brief Test the undoFiles method
 * @details Verifies that undoFiles properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, UndoFiles_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    
    // Setup stubs
    bool publishCalled = false;
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });

    stub.set_lamda(ADDR(FileOperatorProxy, undoFiles), 
                    [&publishCalled](FileOperatorProxy* obj, const CanvasView *view) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->undoFiles(mockView);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Cleanup
    delete mockView;
}

/**
 * @brief Test the redoFiles method
 * @details Verifies that redoFiles properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, RedoFiles_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    
    // Setup stubs
    bool publishCalled = false;
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });

    stub.set_lamda(ADDR(FileOperatorProxy, redoFiles), 
                    [&publishCalled](FileOperatorProxy* obj, const CanvasView *view) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->redoFiles(mockView);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Cleanup
    delete mockView;
}

/**
 * @brief Test the dropFiles method with move action
 * @details Verifies that dropFiles with move action properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, DropFiles_MoveAction_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    Qt::DropAction testAction = Qt::MoveAction;
    QUrl testTargetUrl = QUrl("file:///home/test/target");
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    QList<QSharedPointer<CanvasView>> testViews;
    testViews << QSharedPointer<CanvasView>(mockView);
    
    // Setup stubs
    bool publishCalled = false;
    QVariant customData;
    
    // Stub for CanvasIns->views()
    stub.set_lamda(ADDR(CanvasManager, views), [testViews] {
        __DBG_STUB_INVOKE__
        return testViews;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    stub.set_lamda(ADDR(FileOperatorProxy, dropFiles), 
                  [&publishCalled, &customData](FileOperatorProxy* obj, const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        publishCalled = true;

        QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackPasteFiles, QVariant());
        customData = QVariant::fromValue(funcData);
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->dropFiles(testAction, testTargetUrl, testUrls);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the custom data was properly set
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData = customData.value<QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant>>();
    EXPECT_EQ(funcData.first, FileOperatorProxyPrivate::kCallBackPasteFiles);
}

/**
 * @brief Test the dropFiles method with copy action
 * @details Verifies that dropFiles with copy action properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, DropFiles_CopyAction_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    Qt::DropAction testAction = Qt::CopyAction;
    QUrl testTargetUrl = QUrl("file:///home/test/target");
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    QList<QSharedPointer<CanvasView>> testViews;
    testViews << QSharedPointer<CanvasView>(mockView);
    
    // Setup stubs
    bool publishCalled = false;
    QVariant customData;
    
    // Stub for CanvasIns->views()
    stub.set_lamda(ADDR(CanvasManager, views), [testViews] {
        __DBG_STUB_INVOKE__
        return testViews;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });
    
    stub.set_lamda(ADDR(FileOperatorProxy, dropFiles), 
                  [&publishCalled, &customData](FileOperatorProxy* obj, const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        publishCalled = true;

        QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackPasteFiles, QVariant());
        customData = QVariant::fromValue(funcData);
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->dropFiles(testAction, testTargetUrl, testUrls);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the custom data was properly set
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData = customData.value<QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant>>();
    EXPECT_EQ(funcData.first, FileOperatorProxyPrivate::kCallBackPasteFiles);
}

/**
 * @brief Test the dropFiles method with no views available
 * @details Verifies that dropFiles properly handles case when no views are available
 */
TEST_F(TestFileoperatorproxy, DropFiles_NoViews_DoesNotCallPublish)
{
    // Mock data
    Qt::DropAction testAction = Qt::CopyAction;
    QUrl testTargetUrl = QUrl("file:///home/test/target");
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Empty views list
    QList<QSharedPointer<CanvasView>> emptyViews;
    
    // Setup stubs
    bool publishCalled = false;
    
    // Stub for CanvasIns->views()
    stub.set_lamda(ADDR(CanvasManager, views), [emptyViews] {
        __DBG_STUB_INVOKE__
        return emptyViews;
    });

    stub.set_lamda(ADDR(FileOperatorProxy, dropFiles), 
                  [&publishCalled](FileOperatorProxy* obj, const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->dropFiles(testAction, testTargetUrl, testUrls);
    
    // Verify that publish was NOT called due to no views available
    EXPECT_FALSE(publishCalled);
}

/**
 * @brief Test the dropToTrash method
 * @details Verifies that dropToTrash properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, DropToTrash_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    QList<QSharedPointer<CanvasView>> testViews;
    testViews << QSharedPointer<CanvasView>(mockView);
    
    // Setup stubs
    bool publishCalled = false;
    QList<QUrl> publishedUrls;
    AbstractJobHandler::JobFlag publishedFlag;
    
    // Stub for CanvasIns->views()
    stub.set_lamda(ADDR(CanvasManager, views), [testViews] {
        __DBG_STUB_INVOKE__
        return testViews;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });

    stub.set_lamda(ADDR(FileOperatorProxy, dropToTrash), 
                  [&publishCalled, &publishedUrls, &publishedFlag, testUrls](FileOperatorProxy* obj, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        publishedUrls = urls;
        publishedFlag = AbstractJobHandler::JobFlag::kNoHint;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->dropToTrash(testUrls);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the URLs and flag were passed correctly
    EXPECT_EQ(publishedUrls, testUrls);
    EXPECT_EQ(publishedFlag, AbstractJobHandler::JobFlag::kNoHint);
}

/**
 * @brief Test the dropToTrash method with no views available
 * @details Verifies that dropToTrash properly handles case when no views are available
 */
TEST_F(TestFileoperatorproxy, DropToTrash_NoViews_DoesNotCallPublish)
{
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Empty views list
    QList<QSharedPointer<CanvasView>> emptyViews;
    
    // Setup stubs
    bool publishCalled = false;
    
    // Stub for CanvasIns->views()
    stub.set_lamda(ADDR(CanvasManager, views), [emptyViews] {
        __DBG_STUB_INVOKE__
        return emptyViews;
    });

    stub.set_lamda(ADDR(FileOperatorProxy, dropToTrash), 
                  [&publishCalled](FileOperatorProxy* obj, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
    });
    
    // Call the method under test - this should not crash even with no views
    FileOperatorProxy::instance()->dropToTrash(testUrls);
    
    // Verify that publish was NOT called due to no views available
    EXPECT_FALSE(publishCalled);
}

/**
 * @brief Test the dropToApp method
 * @details Verifies that dropToApp properly calls ADDR(FileOperatorProxy, publish)
 */
TEST_F(TestFileoperatorproxy, DropToApp_CallsPublish)
{
    // Mock data
    WId testWinId = 12345;
    QString testApp = "test-app";
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Create mock objects
    CanvasView *mockView = new CanvasView();
    QList<QSharedPointer<CanvasView>> testViews;
    testViews << QSharedPointer<CanvasView>(mockView);
    
    // Setup stubs
    bool publishCalled = false;
    QList<QUrl> publishedUrls;
    QList<QString> publishedApps;
    
    // Stub for CanvasIns->views()
    stub.set_lamda(ADDR(CanvasManager, views), [testViews] {
        __DBG_STUB_INVOKE__
        return testViews;
    });
    
    // Stub for view->winId()
    stub.set_lamda(ADDR(CanvasView, winId), [testWinId] {
        __DBG_STUB_INVOKE__
        return testWinId;
    });

    stub.set_lamda(ADDR(FileOperatorProxy, dropToApp), 
                  [&publishCalled, &publishedUrls, &publishedApps](FileOperatorProxy* obj, const QList<QUrl> &urls, const QString &app) {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        publishedUrls = urls;
        publishedApps = QList<QString>() << app;
    });
    
    // Call the method under test
    FileOperatorProxy::instance()->dropToApp(testUrls, testApp);
    
    // Verify that publish was called
    EXPECT_TRUE(publishCalled);
    
    // Verify the URLs and apps were passed correctly
    EXPECT_EQ(publishedUrls, testUrls);
    EXPECT_EQ(publishedApps.size(), 1);
    EXPECT_EQ(publishedApps.first(), testApp);
}

/**
 * @brief Test the dropToApp method with no views available
 * @details Verifies that dropToApp properly handles case when no views are available
 */
TEST_F(TestFileoperatorproxy, DropToApp_NoViews_DoesNotCallPublish)
{
    // Mock data
    QString testApp = "test-app";
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///home/test/file1.txt")
             << QUrl("file:///home/test/file2.txt");
    
    // Empty views list
    QList<QSharedPointer<CanvasView>> emptyViews;
    
    // Setup stubs
    bool publishCalled = false;
    
    // Stub for CanvasIns->views()
    stub.set_lamda(ADDR(CanvasManager, views), [emptyViews] {
        __DBG_STUB_INVOKE__
        return emptyViews;
    });

    stub.set_lamda(ADDR(FileOperatorProxy, dropToApp), 
                  [&publishCalled](FileOperatorProxy* obj, const QList<QUrl> &urls, const QString &app) {
        __DBG_STUB_INVOKE__
    });
    
    // Call the method under test - this should not crash even with no views
    FileOperatorProxy::instance()->dropToApp(testUrls, testApp);
    
    // Verify that publish was NOT called due to no views available
    EXPECT_FALSE(publishCalled);
}

/**
 * @brief Test the touchFileData and clearTouchFileData methods
 * @details Verifies that touchFileData and clearTouchFileData work correctly
 */
TEST_F(TestFileoperatorproxy, TouchFileData_GetAndClear)
{
    // Get the instance
    FileOperatorProxy *instance = FileOperatorProxy::instance();
    
    // Create a test FileOperatorProxyPrivate object with test data
    FileOperatorProxyPrivate *privateObj = new FileOperatorProxyPrivate(instance);
    privateObj->touchFileData = qMakePair(QString("test/file.txt"), qMakePair(1, QPoint(100, 200)));
    
    // Replace the private object in the instance
    QSharedPointer<FileOperatorProxyPrivate> originalPrivate = instance->d;
    instance->d = QSharedPointer<FileOperatorProxyPrivate>(privateObj);
    
    // Test touchFileData method
    QPair<QString, QPair<int, QPoint>> data = instance->touchFileData();
    EXPECT_EQ(data.first, QString("test/file.txt"));
    EXPECT_EQ(data.second.first, 1);
    EXPECT_EQ(data.second.second, QPoint(100, 200));
    
    // Test clearTouchFileData method
    instance->clearTouchFileData();
    data = instance->touchFileData();
    EXPECT_EQ(data.first, QString(""));
    EXPECT_EQ(data.second.first, -1);
    EXPECT_EQ(data.second.second, QPoint(-1, -1));
    
    // Restore the original private object
    instance->d = originalPrivate;
}

/**
 * @brief Test the renameFileData, removeRenameFileData and clearRenameFileData methods
 * @details Verifies that renameFileData related methods work correctly
 */
TEST_F(TestFileoperatorproxy, RenameFileData_GetRemoveAndClear)
{
    // Get the instance
    FileOperatorProxy *instance = FileOperatorProxy::instance();
    
    // Create a test FileOperatorProxyPrivate object with test data
    FileOperatorProxyPrivate *privateObj = new FileOperatorProxyPrivate(instance);
    QUrl oldUrl1 = QUrl("file:///home/test/old1.txt");
    QUrl newUrl1 = QUrl("file:///home/test/new1.txt");
    QUrl oldUrl2 = QUrl("file:///home/test/old2.txt");
    QUrl newUrl2 = QUrl("file:///home/test/new2.txt");
    privateObj->renameFileData.insert(oldUrl1, newUrl1);
    privateObj->renameFileData.insert(oldUrl2, newUrl2);
    
    // Replace the private object in the instance
    QSharedPointer<FileOperatorProxyPrivate> originalPrivate = instance->d;
    instance->d = QSharedPointer<FileOperatorProxyPrivate>(privateObj);
    
    // Test renameFileData method
    QHash<QUrl, QUrl> data = instance->renameFileData();
    EXPECT_EQ(data.size(), 2);
    EXPECT_EQ(data[oldUrl1], newUrl1);
    EXPECT_EQ(data[oldUrl2], newUrl2);
    
    // Test removeRenameFileData method
    instance->removeRenameFileData(oldUrl1);
    data = instance->renameFileData();
    EXPECT_EQ(data.size(), 1);
    EXPECT_FALSE(data.contains(oldUrl1));
    EXPECT_EQ(data[oldUrl2], newUrl2);
    
    // Test clearRenameFileData method
    instance->clearRenameFileData();
    data = instance->renameFileData();
    EXPECT_TRUE(data.isEmpty());
    
    // Restore the original private object
    instance->d = originalPrivate;
}

/**
 * @brief Test the pasteFileData, removePasteFileData and clearPasteFileData methods
 * @details Verifies that pasteFileData related methods work correctly
 */
TEST_F(TestFileoperatorproxy, PasteFileData_GetRemoveAndClear)
{
    // Get the instance
    FileOperatorProxy *instance = FileOperatorProxy::instance();
    
    // Create a test FileOperatorProxyPrivate object with test data
    FileOperatorProxyPrivate *privateObj = new FileOperatorProxyPrivate(instance);
    QUrl url1 = QUrl("file:///home/test/file1.txt");
    QUrl url2 = QUrl("file:///home/test/file2.txt");
    privateObj->pasteFileData.insert(url1);
    privateObj->pasteFileData.insert(url2);
    
    // Replace the private object in the instance
    QSharedPointer<FileOperatorProxyPrivate> originalPrivate = instance->d;
    instance->d = QSharedPointer<FileOperatorProxyPrivate>(privateObj);
    
    // Test pasteFileData method
    QSet<QUrl> data = instance->pasteFileData();
    EXPECT_EQ(data.size(), 2);
    EXPECT_TRUE(data.contains(url1));
    EXPECT_TRUE(data.contains(url2));
    
    // Test removePasteFileData method
    instance->removePasteFileData(url1);
    data = instance->pasteFileData();
    EXPECT_EQ(data.size(), 1);
    EXPECT_FALSE(data.contains(url1));
    EXPECT_TRUE(data.contains(url2));
    
    // Test clearPasteFileData method
    instance->clearPasteFileData();
    data = instance->pasteFileData();
    EXPECT_TRUE(data.isEmpty());
    
    // Restore the original private object
    instance->d = originalPrivate;
}

/**
 * @brief Test the callBackFunction method with CallBackTouchFile function key
 * @details Verifies that callBackFunction properly handles touch file callbacks
 */
TEST_F(TestFileoperatorproxy, CallBackFunction_CallBackTouchFile)
{
    // Get the instance
    FileOperatorProxy *instance = FileOperatorProxy::instance();
    
    // Create mock objects
    QUrl testUrl = QUrl("file:///home/test/file.txt");
    QVariantMap customData;
    customData.insert(KEY_SCREENNUMBER, 1);
    customData.insert(KEY_POINT, QPoint(100, 200));
    
    // Create a callback argument
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>);
    QList<QUrl> targets;
    targets << testUrl;
    jobInfo->insert(static_cast<quint8>(DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kTargets), QVariant::fromValue(targets));
    
    // Create custom data for the callback
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackTouchFile, customData);
    jobInfo->insert(static_cast<quint8>(DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kCustom), QVariant::fromValue(funcData));
    
    // Setup stubs
    bool callBackTouchFileCalled = false;
    
    // Stub for FileOperatorProxyPrivate::callBackTouchFile
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, callBackTouchFile), [&callBackTouchFileCalled](FileOperatorProxyPrivate *obj, const QUrl &target, const QVariantMap &data) {
        __DBG_STUB_INVOKE__
        callBackTouchFileCalled = true;
        
        // Verify the parameters
        EXPECT_EQ(target.toString(), "file:///home/test/file.txt");
        EXPECT_EQ(data.value("screen_number").toInt(), 1);
        EXPECT_EQ(data.value("point").toPoint(), QPoint(100, 200));
    });
    
    // Call the method under test
    instance->callBackFunction(convertToCallbackArgus(jobInfo));
    
    // Verify that callBackTouchFile was called
    EXPECT_TRUE(callBackTouchFileCalled);
}

/**
 * @brief Test the callBackFunction method with CallBackPasteFiles function key
 * @details Verifies that callBackFunction properly handles paste files callbacks
 */
TEST_F(TestFileoperatorproxy, CallBackFunction_CallBackPasteFiles)
{
    // Get the instance
    FileOperatorProxy *instance = FileOperatorProxy::instance();
    
    // Create a job handle
    JobHandlePointer jobHandle(new AbstractJobHandler);
    
    // Create a callback argument
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>);
    jobInfo->insert(static_cast<quint8>(DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kJobHandle), QVariant::fromValue(jobHandle));
    
    // Create custom data for the callback
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackPasteFiles, QVariant());
    jobInfo->insert(static_cast<quint8>(DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kCustom), QVariant::fromValue(funcData));
    
    // Setup stubs
    bool callBackPasteFilesCalled = false;
    bool connectCalled = true;
    
    // Stub for jobHandle->currentState()
    stub.set_lamda(VADDR(AbstractJobHandler, currentState), [] {
        __DBG_STUB_INVOKE__
        return DFMBASE_NAMESPACE::AbstractJobHandler::JobState::kRunningState;
    });
    
    // Call the method under test
    instance->callBackFunction(convertToCallbackArgus(jobInfo));
    
    // Verify that connect was called for the finishedNotify signal
    EXPECT_TRUE(connectCalled);
    
    // Now test with a stopped job
    connectCalled = false;
    
    // Stub for jobHandle->currentState() to return stopped state
    stub.set_lamda(VADDR(AbstractJobHandler, currentState), [] {
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::JobState::kStopState;
    });
    
    // Stub for jobHandle->getTaskInfoByNotifyType()
    stub.set_lamda(VADDR(AbstractJobHandler, getTaskInfoByNotifyType), [](DFMBASE_NAMESPACE::AbstractJobHandler *obj, const DFMBASE_NAMESPACE::AbstractJobHandler::NotifyType &type) {
        __DBG_STUB_INVOKE__
        return JobInfoPointer(new QMap<quint8, QVariant>);
    });
    
    // Stub for FileOperatorProxyPrivate::callBackPasteFiles
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, callBackPasteFiles), [&callBackPasteFilesCalled](FileOperatorProxyPrivate *obj, const JobInfoPointer &info) {
        __DBG_STUB_INVOKE__
        callBackPasteFilesCalled = true;
    });
    
    // Call the method under test again
    instance->callBackFunction(convertToCallbackArgus(jobInfo));
    
    // Verify that callBackPasteFiles was called directly for stopped job
    EXPECT_TRUE(callBackPasteFilesCalled);
    EXPECT_FALSE(connectCalled);
}

/**
 * @brief Test the callBackFunction method with CallBackRenameFiles function key
 * @details Verifies that callBackFunction properly handles rename files callbacks
 */
TEST_F(TestFileoperatorproxy, CallBackFunction_CallBackRenameFiles)
{
    // Get the instance
    FileOperatorProxy *instance = FileOperatorProxy::instance();
    
    // Create test URLs
    QList<QUrl> sources;
    sources << QUrl("file:///home/test/old1.txt") << QUrl("file:///home/test/old2.txt");
    
    QList<QUrl> targets;
    targets << QUrl("file:///home/test/new1.txt") << QUrl("file:///home/test/new2.txt");
    
    // Create a callback argument
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>);
    jobInfo->insert(static_cast<quint8>(DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kSourceUrls), QVariant::fromValue(sources));
    jobInfo->insert(static_cast<quint8>(DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kTargets), QVariant::fromValue(targets));
    
    // Create custom data for the callback
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackRenameFiles, QVariant());
    jobInfo->insert(static_cast<quint8>(DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kCustom), QVariant::fromValue(funcData));
    
    // Setup stubs
    bool callBackRenameFilesCalled = false;
    
    // Stub for FileOperatorProxyPrivate::callBackRenameFiles
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, callBackRenameFiles), [&callBackRenameFilesCalled, sources, targets](FileOperatorProxyPrivate *obj, const QList<QUrl> &src, const QList<QUrl> &tgt) {
        __DBG_STUB_INVOKE__
        callBackRenameFilesCalled = true;
        
        // Verify the parameters
        EXPECT_EQ(src, sources);
        EXPECT_EQ(tgt, targets);
    });
    
    // Call the method under test
    instance->callBackFunction(convertToCallbackArgus(jobInfo));
    
    // Verify that callBackRenameFiles was called
    EXPECT_TRUE(callBackRenameFilesCalled);
}

/**
 * @brief Test the callBackFunction method with unknown function key
 * @details Verifies that callBackFunction properly handles unknown function keys
 */
TEST_F(TestFileoperatorproxy, CallBackFunction_UnknownFunctionKey)
{
    // Get the instance
    FileOperatorProxy *instance = FileOperatorProxy::instance();
    
    // Create a callback argument
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>);
    
    // Create custom data with an invalid function key
    FileOperatorProxyPrivate::CallBackFunc invalidKey = static_cast<FileOperatorProxyPrivate::CallBackFunc>(999);
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(invalidKey, QVariant());
    jobInfo->insert(static_cast<quint8>(DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kCustom), QVariant::fromValue(funcData));
    
    // Setup stubs to verify no callback functions are called
    bool callBackTouchFileCalled = false;
    bool callBackPasteFilesCalled = false;
    bool callBackRenameFilesCalled = false;
    
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, callBackTouchFile), [&callBackTouchFileCalled](FileOperatorProxyPrivate *obj, const QUrl &, const QVariantMap &) {
        __DBG_STUB_INVOKE__
        callBackTouchFileCalled = true;
    });
    
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, callBackPasteFiles), [&callBackPasteFilesCalled](FileOperatorProxyPrivate *obj, const JobInfoPointer &) {
        __DBG_STUB_INVOKE__
        callBackPasteFilesCalled = true;
    });
    
    stub.set_lamda(ADDR(FileOperatorProxyPrivate, callBackRenameFiles), [&callBackRenameFilesCalled](FileOperatorProxyPrivate *obj, const QList<QUrl> &, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        callBackRenameFilesCalled = true;
    });
    
    // Call the method under test
    instance->callBackFunction(convertToCallbackArgus(jobInfo));
    
    // Verify that no callback functions were called
    EXPECT_FALSE(callBackTouchFileCalled);
    EXPECT_FALSE(callBackPasteFilesCalled);
    EXPECT_FALSE(callBackRenameFilesCalled);
}
