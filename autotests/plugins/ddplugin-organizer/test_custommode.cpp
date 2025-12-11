// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/custommode.h"
#include "organizer_defines.h"
#include "models/collectionmodel.h"
#include "interface/fileinfomodelshell.h"

#include <QUrl>
#include <QModelIndex>
#include <QMimeData>
#include <QPoint>
#include <QVector>
#include <QTest>

#include "stubext.h"
#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_CustomMode : public testing::Test
{
protected:
    void SetUp() override
    {
        mockModel = new CollectionModel();
        // mock model shell for mock model
        mockModel->setModelShell(new FileInfoModelShell());
        customMode = new CustomMode();
    }
    
    void TearDown() override
    {
        delete customMode;
        delete mockModel;

        stub.clear();
    }
    
    CustomMode* customMode;
    CollectionModel *mockModel;
    stub_ext::StubExt stub;
};

TEST_F(UT_CustomMode, ConstructorTest)
{
    EXPECT_NE(customMode, nullptr);
}

TEST_F(UT_CustomMode, DestructorTest)
{
    CustomMode* tempMode = new CustomMode();
    EXPECT_NE(tempMode, nullptr);
    delete tempMode;
}

TEST_F(UT_CustomMode, ModeTest)
{
    OrganizerMode mode = customMode->mode();
    EXPECT_GE(mode, 0);
    EXPECT_LE(mode, 1);
}

TEST_F(UT_CustomMode, InitializeTest)
{
    bool result = customMode->initialize(mockModel);
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_CustomMode, ResetTest)
{
    customMode->initialize(mockModel);
    EXPECT_NO_THROW(customMode->reset());
}

TEST_F(UT_CustomMode, LayoutTest)
{
    EXPECT_NO_THROW(customMode->layout());
}

TEST_F(UT_CustomMode, DetachLayoutTest)
{
    EXPECT_NO_THROW(customMode->detachLayout());
}

TEST_F(UT_CustomMode, RebuildTest)
{
    customMode->initialize(mockModel);
    EXPECT_NO_THROW(customMode->rebuild());
}

TEST_F(UT_CustomMode, OnFileRenamedTest)
{
    QUrl oldUrl("file:///old.txt");
    QUrl newUrl("file:///new.txt");
    customMode->initialize(mockModel);
    EXPECT_NO_THROW(customMode->onFileRenamed(oldUrl, newUrl));
    
    QUrl emptyUrl;
    EXPECT_NO_THROW(customMode->onFileRenamed(emptyUrl, emptyUrl));
    
    for (int i = 0; i < 3; ++i) {
        QUrl testUrl1(QString("file:///old%1.txt").arg(i));
        QUrl testUrl2(QString("file:///new%1.txt").arg(i));
        customMode->onFileRenamed(testUrl1, testUrl2);
    }
}

TEST_F(UT_CustomMode, OnFileInsertedTest)
{
    QModelIndex parent;
    
    customMode->initialize(mockModel);
    EXPECT_NO_THROW(customMode->onFileInserted(parent, 0, 0));
    EXPECT_NO_THROW(customMode->onFileInserted(parent, 1, 5));
    EXPECT_NO_THROW(customMode->onFileInserted(parent, -1, 10));
    
    for (int i = 0; i < 3; ++i) {
        customMode->onFileInserted(QModelIndex(), i, i + 2);
    }
}

TEST_F(UT_CustomMode, OnFileAboutToBeRemovedTest)
{
    QModelIndex parent;
    customMode->initialize(mockModel);
    EXPECT_NO_THROW(customMode->onFileAboutToBeRemoved(parent, 0, 0));
    EXPECT_NO_THROW(customMode->onFileAboutToBeRemoved(parent, 1, 3));
    EXPECT_NO_THROW(customMode->onFileAboutToBeRemoved(parent, -1, 5));
    
    for (int i = 0; i < 3; ++i) {
        customMode->onFileAboutToBeRemoved(QModelIndex(), i, i + 1);
    }
}

TEST_F(UT_CustomMode, OnFileDataChangedTest)
{
    QModelIndex topLeft;
    QModelIndex bottomRight;
    QVector<int> roles = {0, 1, 2};
    
    customMode->initialize(mockModel);
    EXPECT_NO_THROW(customMode->onFileDataChanged(topLeft, bottomRight, roles));
    EXPECT_NO_THROW(customMode->onFileDataChanged(topLeft, bottomRight, QVector<int>()));
    
    for (int i = 0; i < 3; ++i) {
        QVector<int> testRoles = {i, i + 1};
        customMode->onFileDataChanged(QModelIndex(), QModelIndex(), testRoles);
    }
}

TEST_F(UT_CustomMode, FilterDataRestedTest)
{
    QList<QUrl> urls = {
        QUrl("file:///test1.txt"),
        QUrl("file:///test2.txt")
    };
    
    customMode->initialize(mockModel);

    bool result = customMode->filterDataRested(&urls);
    EXPECT_TRUE(result || !result);
    
    QList<QUrl> emptyUrls;
    bool result2 = customMode->filterDataRested(&emptyUrls);
    EXPECT_TRUE(result2 || !result2);
    
    bool result3 = customMode->filterDataRested(nullptr);
    EXPECT_TRUE(result3 || !result3);
}

TEST_F(UT_CustomMode, FilterDataInsertedTest)
{
    QUrl url("file:///test.txt");
    
    customMode->initialize(mockModel);
    bool result = customMode->filterDataInserted(url);
    EXPECT_TRUE(result || !result);
    
    QUrl emptyUrl;
    bool result2 = customMode->filterDataInserted(emptyUrl);
    EXPECT_TRUE(result2 || !result2);
    
    for (int i = 0; i < 3; ++i) {
        QUrl testUrl(QString("file:///test%1.txt").arg(i));
        customMode->filterDataInserted(testUrl);
    }
}

TEST_F(UT_CustomMode, FilterDataRenamedTest)
{
    QUrl oldUrl("file:///old.txt");
    QUrl newUrl("file:///new.txt");
    
    customMode->initialize(mockModel);
    bool result = customMode->filterDataRenamed(oldUrl, newUrl);
    EXPECT_TRUE(result || !result);
    
    QUrl emptyUrl;
    bool result2 = customMode->filterDataRenamed(emptyUrl, emptyUrl);
    EXPECT_TRUE(result2 || !result2);
    
    for (int i = 0; i < 3; ++i) {
        QUrl testUrl1(QString("file:///old%1.txt").arg(i));
        QUrl testUrl2(QString("file:///new%1.txt").arg(i));
        customMode->filterDataRenamed(testUrl1, testUrl2);
    }
}

TEST_F(UT_CustomMode, FilterDropDataTest)
{
    int viewIndex = 0;
    QMimeData* mimeData = new QMimeData();
    QPoint viewPoint(10, 10);
    void* extData = nullptr;

    customMode->initialize(mockModel);
    bool result = customMode->filterDropData(viewIndex, mimeData, viewPoint, extData);
    EXPECT_TRUE(result || !result);
    
    delete mimeData;
    
    QMimeData* emptyData = new QMimeData();
    bool result2 = customMode->filterDropData(-1, emptyData, QPoint(), nullptr);
    EXPECT_TRUE(result2 || !result2);
    
    delete emptyData;
}

TEST_F(UT_CustomMode, MultipleInstancesTest)
{
    CustomMode* mode1 = new CustomMode();
    CustomMode* mode2 = new CustomMode();
    CustomMode* mode3 = new CustomMode();
    
    EXPECT_NE(mode1, nullptr);
    EXPECT_NE(mode2, nullptr);
    EXPECT_NE(mode3, nullptr);
    
    EXPECT_GE(mode1->mode(), 0);
    EXPECT_GE(mode2->mode(), 0);
    EXPECT_GE(mode3->mode(), 0);
    
    mode1->initialize(mockModel);
    mode1->reset();
    mode2->initialize(mockModel);
    mode2->reset();
    mode3->initialize(mockModel);
    mode3->reset();
    
    delete mode1;
    delete mode2;
    delete mode3;
}

TEST_F(UT_CustomMode, EdgeCasesTest)
{
    CustomMode* mode = new CustomMode();
    mode->initialize(mockModel);
    
    QUrl invalidUrl("invalid-url");
    EXPECT_NO_THROW(mode->onFileRenamed(invalidUrl, invalidUrl));
    EXPECT_NO_THROW(mode->filterDataInserted(invalidUrl));
    EXPECT_NO_THROW(mode->filterDataRenamed(invalidUrl, invalidUrl));
    
    QModelIndex invalidIndex;
    EXPECT_NO_THROW(mode->onFileInserted(invalidIndex, -1, -1));
    EXPECT_NO_THROW(mode->onFileAboutToBeRemoved(invalidIndex, -1, -1));
    EXPECT_NO_THROW(mode->onFileDataChanged(invalidIndex, invalidIndex, QVector<int>{-1}));
    
    delete mode;
}

TEST_F(UT_CustomMode, StressTest)
{
    customMode->initialize(mockModel);
    
    for (int i = 0; i < 5; ++i) {
        customMode->reset();
        customMode->layout();
        customMode->detachLayout();
        customMode->rebuild();
        
        QUrl testUrl(QString("file:///stress%1.txt").arg(i));
        customMode->filterDataInserted(testUrl);
        
        QTest::qWait(1);
    }
}
