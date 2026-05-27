// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "broker/fileinfomodelbroker.h"
#include "model/fileinfomodel.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QUrl>
#include <QModelIndex>

using namespace ddplugin_canvas;

class UT_FileInfoModelBroker : public testing::Test
{
protected:
    void SetUp() override {
        mockModel = new FileInfoModel();
        broker = new FileInfoModelBroker(mockModel);
        
        // Skip complex DPF framework stubbing to avoid template resolution issues
    }

    void TearDown() override {
        delete broker;
        delete mockModel;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    FileInfoModel *mockModel = nullptr;
    FileInfoModelBroker *broker = nullptr;
};

TEST_F(UT_FileInfoModelBroker, Constructor_CreateWithModel_InitializesCorrectly)
{
    EXPECT_NE(broker, nullptr);
    EXPECT_EQ(broker->parent(), nullptr);
}

TEST_F(UT_FileInfoModelBroker, init_InitializeBroker_ReturnsTrue)
{
    bool result = broker->init();
    EXPECT_TRUE(result);
}

TEST_F(UT_FileInfoModelBroker, rootUrl_GetRootUrl_CallsModel)
{
    QUrl expectedUrl("file:///home/test");
    
    stub.set_lamda(ADDR(FileInfoModel, rootUrl), [&expectedUrl](FileInfoModel*) -> QUrl {
        __DBG_STUB_INVOKE__
        return expectedUrl;
    });
    
    QUrl result = broker->rootUrl();
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(UT_FileInfoModelBroker, rootIndex_GetRootIndex_CallsModel)
{
    QModelIndex expectedIndex;
    
    stub.set_lamda(ADDR(FileInfoModel, rootIndex), [&expectedIndex](FileInfoModel*) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return expectedIndex;
    });
    
    QModelIndex result = broker->rootIndex();
    EXPECT_EQ(result, expectedIndex);
}

TEST_F(UT_FileInfoModelBroker, urlIndex_GetIndexForUrl_CallsModel)
{
    QUrl testUrl("file:///home/test/file.txt");
    QModelIndex expectedIndex;
    
    using IndexUrlOverload = QModelIndex (FileInfoModel::*)(const QUrl&, int) const;
    stub.set_lamda(static_cast<IndexUrlOverload>(&FileInfoModel::index), 
                   [&expectedIndex](FileInfoModel*, const QUrl&, int) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return expectedIndex;
    });
    
    QModelIndex result = broker->urlIndex(testUrl);
    EXPECT_EQ(result, expectedIndex);
}

TEST_F(UT_FileInfoModelBroker, fileInfo_GetFileInfo_CallsModel)
{
    QModelIndex testIndex;
    FileInfoPointer expectedInfo = nullptr;
    
    stub.set_lamda(ADDR(FileInfoModel, fileInfo), [&expectedInfo](FileInfoModel*, const QModelIndex&) -> FileInfoPointer {
        __DBG_STUB_INVOKE__
        return expectedInfo;
    });
    
    FileInfoPointer result = broker->fileInfo(testIndex);
    EXPECT_EQ(result, expectedInfo);
}

TEST_F(UT_FileInfoModelBroker, indexUrl_GetUrlForIndex_CallsModel)
{
    QModelIndex testIndex;
    QUrl expectedUrl("file:///home/test/file.txt");
    
    stub.set_lamda(ADDR(FileInfoModel, fileUrl), [&expectedUrl](FileInfoModel*, const QModelIndex&) -> QUrl {
        __DBG_STUB_INVOKE__
        return expectedUrl;
    });
    
    QUrl result = broker->indexUrl(testIndex);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(UT_FileInfoModelBroker, files_GetAllFiles_CallsModel)
{
    QList<QUrl> expectedFiles;
    expectedFiles << QUrl("file:///home/test/file1.txt") << QUrl("file:///home/test/file2.txt");
    
    stub.set_lamda(ADDR(FileInfoModel, files), [&expectedFiles](FileInfoModel*) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return expectedFiles;
    });
    
    QList<QUrl> result = broker->files();
    EXPECT_EQ(result, expectedFiles);
}

TEST_F(UT_FileInfoModelBroker, refresh_RefreshModel_CallsModel)
{
    QModelIndex testParent;
    bool methodCalled = false;
    
    stub.set_lamda(ADDR(FileInfoModel, refresh), [&methodCalled](FileInfoModel*, const QModelIndex&) {
        __DBG_STUB_INVOKE__
        methodCalled = true;
    });
    
    broker->refresh(testParent);
    EXPECT_TRUE(methodCalled);
}

TEST_F(UT_FileInfoModelBroker, modelState_GetModelState_CallsModel)
{
    int expectedState = 42;
    
    stub.set_lamda(ADDR(FileInfoModel, modelState), [&expectedState](FileInfoModel*) -> int {
        __DBG_STUB_INVOKE__
        return expectedState;
    });
    
    int result = broker->modelState();
    EXPECT_EQ(result, expectedState);
}

TEST_F(UT_FileInfoModelBroker, updateFile_UpdateFile_CallsModel)
{
    QUrl testUrl("file:///home/test/file.txt");
    bool methodCalled = false;
    
    stub.set_lamda(ADDR(FileInfoModel, updateFile), [&methodCalled](FileInfoModel*, const QUrl&) {
        __DBG_STUB_INVOKE__
        methodCalled = true;
    });
    
    broker->updateFile(testUrl);
    EXPECT_TRUE(methodCalled);
}

TEST_F(UT_FileInfoModelBroker, onDataReplaced_DataReplacedSignal_ExecutesWithoutCrash)
{
    QUrl oldUrl("file:///home/test/old.txt");
    QUrl newUrl("file:///home/test/new.txt");
    
    // Test that onDataReplaced executes without crashing
    EXPECT_NO_THROW({
        broker->onDataReplaced(oldUrl, newUrl);
    });
}
