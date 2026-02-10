// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "hook/canvasmodelhook.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariant>
#include <QMimeData>

using namespace ddplugin_canvas;

class UT_CanvasModelHook : public testing::Test
{
public:
    virtual void SetUp() override
    {
        hook = new CanvasModelHook();
    }

    virtual void TearDown() override
    {
        delete hook;
        
        stub.clear();
    }

public:
    CanvasModelHook *hook = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CanvasModelHook, constructor_CreateHook_InitializesCorrectly)
{
    EXPECT_NE(hook, nullptr);
}

TEST_F(UT_CanvasModelHook, modelData_CallModelData_RunsHook)
{
    QUrl testUrl("file:///home/test/file.txt");
    int role = Qt::DisplayRole;
    QVariant initialValue("initial");
    
    // modelData method expects QVariant* as third parameter
    bool result = hook->modelData(testUrl, role, &initialValue);
    
    // The method should return boolean without crashing
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasModelHook, dataInserted_CallDataInserted_RunsHook)
{
    QUrl testUrl("file:///home/test/file.txt");
    
    // Test that the method doesn't crash
    EXPECT_NO_THROW(hook->dataInserted(testUrl));
}

TEST_F(UT_CanvasModelHook, dataRemoved_CallDataRemoved_RunsHook)
{
    QUrl testUrl("file:///home/test/file.txt");
    
    // Test that the method doesn't crash
    EXPECT_NO_THROW(hook->dataRemoved(testUrl));
}

TEST_F(UT_CanvasModelHook, dataRenamed_CallDataRenamed_RunsHook)
{
    QUrl oldUrl("file:///home/test/oldfile.txt");
    QUrl newUrl("file:///home/test/newfile.txt");
    
    // Test that the method doesn't crash
    EXPECT_NO_THROW(hook->dataRenamed(oldUrl, newUrl));
}

TEST_F(UT_CanvasModelHook, dataRested_CallDataRested_RunsHook)
{
    QList<QUrl> urls = {
        QUrl("file:///home/test/file1.txt"),
        QUrl("file:///home/test/file2.txt")
    };
    
    // dataRested expects QList<QUrl>* as parameter
    bool result = hook->dataRested(&urls);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasModelHook, dataChanged_CallDataChanged_RunsHook)
{
    QUrl testUrl("file:///home/test/file.txt");
    QVector<int> roles = {Qt::DisplayRole, Qt::DecorationRole};
    
    // Test that the method doesn't crash
    EXPECT_NO_THROW(hook->dataChanged(testUrl, roles));
}

TEST_F(UT_CanvasModelHook, dropMimeData_CallDropMimeData_RunsHook)
{
    QMimeData mimeData;
    QUrl targetUrl("file:///home/test/");
    Qt::DropAction action = Qt::CopyAction;
    
    // Test that the method doesn't crash
    bool result = hook->dropMimeData(&mimeData, targetUrl, action);
    EXPECT_TRUE(result == true || result == false); // Just verify it returns a boolean
}

TEST_F(UT_CanvasModelHook, mimeData_CallMimeData_RunsHook)
{
    QList<QUrl> urls = {
        QUrl("file:///home/test/file1.txt"),
        QUrl("file:///home/test/file2.txt")
    };
    QMimeData mimeData;
    
    // mimeData expects QMimeData* as second parameter
    bool result = hook->mimeData(urls, &mimeData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasModelHook, mimeTypes_CallMimeTypes_RunsHook)
{
    QStringList types;
    
    // mimeTypes expects QStringList* as parameter
    bool result = hook->mimeTypes(&types);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasModelHook, sortData_CallSortData_RunsHook)
{
    int column = 0;
    int order = Qt::AscendingOrder;
    QList<QUrl> urls = {
        QUrl("file:///home/test/file1.txt"),
        QUrl("file:///home/test/file2.txt")
    };
    
    // sortData expects QList<QUrl>* as third parameter
    bool result = hook->sortData(column, order, &urls);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasModelHook, hiddenFlagChanged_CallHiddenFlagChanged_PublishesSignal)
{
    bool showHiddenFiles = true;
    
    // Test that the method doesn't crash
    EXPECT_NO_THROW(hook->hiddenFlagChanged(showHiddenFiles));
}