// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/model/modelhookinterface.h"

#include <QUrl>
#include <QMimeData>
#include <QVariant>
#include <QStringList>
#include <QVector>

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

class UT_ModelHookInterface : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Create instance of ModelHookInterface for testing
        hookInterface = new ModelHookInterface();
    }

    virtual void TearDown() override
    {
        delete hookInterface;
        hookInterface = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    ModelHookInterface *hookInterface = nullptr;
};

TEST_F(UT_ModelHookInterface, Constructor_CreateInterface_ObjectCreated)
{
    // Test constructor behavior
    EXPECT_NE(hookInterface, nullptr);
}

TEST_F(UT_ModelHookInterface, Destructor_DeleteInterface_ObjectDestroyed)
{
    // Test destructor behavior - should not crash
    ModelHookInterface *tempInterface = new ModelHookInterface();
    EXPECT_NO_THROW(delete tempInterface);
}

TEST_F(UT_ModelHookInterface, modelData_DefaultImplementation_ReturnsFalse)
{
    // Test default modelData implementation
    QUrl testUrl("file:///tmp/test.txt");
    int testRole = Qt::DisplayRole;
    QVariant out;
    
    bool result = hookInterface->modelData(testUrl, testRole, &out);
    EXPECT_FALSE(result);
    
    // Test with different role
    result = hookInterface->modelData(testUrl, Qt::DecorationRole, &out);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->modelData(testUrl, testRole, &out, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_ModelHookInterface, dataInserted_DefaultImplementation_ReturnsFalse)
{
    // Test default dataInserted implementation
    QUrl testUrl("file:///tmp/newfile.txt");
    
    bool result = hookInterface->dataInserted(testUrl);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->dataInserted(testUrl, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_ModelHookInterface, dataRemoved_DefaultImplementation_ReturnsFalse)
{
    // Test default dataRemoved implementation
    QUrl testUrl("file:///tmp/deletedfile.txt");
    
    bool result = hookInterface->dataRemoved(testUrl);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->dataRemoved(testUrl, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_ModelHookInterface, dataRenamed_DefaultImplementation_ReturnsFalse)
{
    // Test default dataRenamed implementation
    QUrl oldUrl("file:///tmp/oldname.txt");
    QUrl newUrl("file:///tmp/newname.txt");
    
    bool result = hookInterface->dataRenamed(oldUrl, newUrl);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->dataRenamed(oldUrl, newUrl, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_ModelHookInterface, dataRested_DefaultImplementation_ReturnsFalse)
{
    // Test default dataRested implementation
    QList<QUrl> testUrls = {
        QUrl("file:///tmp/file1.txt"),
        QUrl("file:///tmp/file2.txt")
    };
    
    bool result = hookInterface->dataRested(&testUrls);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->dataRested(&testUrls, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_ModelHookInterface, dataChanged_DefaultImplementation_ReturnsFalse)
{
    // Test default dataChanged implementation
    QUrl testUrl("file:///tmp/changedfile.txt");
    QVector<int> testRoles = {Qt::DisplayRole, Qt::DecorationRole};
    
    bool result = hookInterface->dataChanged(testUrl, testRoles);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->dataChanged(testUrl, testRoles, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_ModelHookInterface, dropMimeData_DefaultImplementation_ReturnsFalse)
{
    // Test default dropMimeData implementation
    QMimeData *testData = new QMimeData();
    testData->setText("test data");
    QUrl testDir("file:///tmp/");
    Qt::DropAction testAction = Qt::CopyAction;
    
    bool result = hookInterface->dropMimeData(testData, testDir, testAction);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->dropMimeData(testData, testDir, testAction, nullptr);
    EXPECT_FALSE(result);
    
    // Test with different action
    result = hookInterface->dropMimeData(testData, testDir, Qt::MoveAction);
    EXPECT_FALSE(result);
    
    delete testData;
}

TEST_F(UT_ModelHookInterface, mimeData_DefaultImplementation_ReturnsFalse)
{
    // Test default mimeData implementation
    QList<QUrl> testUrls = {
        QUrl("file:///tmp/file1.txt"),
        QUrl("file:///tmp/file2.txt")
    };
    QMimeData *out = new QMimeData();
    
    bool result = hookInterface->mimeData(testUrls, out);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->mimeData(testUrls, out, nullptr);
    EXPECT_FALSE(result);
    
    delete out;
}

TEST_F(UT_ModelHookInterface, mimeTypes_DefaultImplementation_ReturnsFalse)
{
    // Test default mimeTypes implementation
    QStringList types;
    
    bool result = hookInterface->mimeTypes(&types);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->mimeTypes(&types, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_ModelHookInterface, sortData_DefaultImplementation_ReturnsFalse)
{
    // Test default sortData implementation
    int testRole = Qt::DisplayRole;
    int testOrder = Qt::AscendingOrder;
    QList<QUrl> testFiles = {
        QUrl("file:///tmp/file1.txt"),
        QUrl("file:///tmp/file2.txt"),
        QUrl("file:///tmp/file3.txt")
    };
    
    bool result = hookInterface->sortData(testRole, testOrder, &testFiles);
    EXPECT_FALSE(result);
    
    // Test with different order
    result = hookInterface->sortData(testRole, Qt::DescendingOrder, &testFiles);
    EXPECT_FALSE(result);
    
    // Test with extData
    result = hookInterface->sortData(testRole, testOrder, &testFiles, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_ModelHookInterface, hiddenFlagChanged_DefaultImplementation_DoesNotCrash)
{
    // Test default hiddenFlagChanged implementation - should not crash
    EXPECT_NO_THROW(hookInterface->hiddenFlagChanged(true));
    EXPECT_NO_THROW(hookInterface->hiddenFlagChanged(false));
}

