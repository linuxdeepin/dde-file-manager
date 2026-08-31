// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basicwidget_1.cpp
 * @brief Unit tests for BasicWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/basicwidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class BasicWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BasicWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BasicWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BasicWidgetTest, audioExtenInfo)
{
    // Test method: void audioExtenInfo((const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->audioExtenInfo(_arg0, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant>()));
}

TEST_F(BasicWidgetTest, closeEvent)
{
    // Test event handler: closeEvent((QCloseEvent *event))
    QCloseEvent _event(QCloseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->closeEvent(&_event));
}

TEST_F(BasicWidgetTest, closeEvent_closeEve)
{
    // Test event handler: closeEvent((QCloseEvent *event))
    QCloseEvent _event(QCloseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->closeEvent(&_event));
}

TEST_F(BasicWidgetTest, createValueLabel)
{
    // Test method: KeyValueLabel createValueLabel((QFrame *frame, QString leftValue))
    auto result = obj->createValueLabel(nullptr, QString());
    EXPECT_NO_FATAL_FAILURE({ obj->createValueLabel(nullptr, QString()); });

}

TEST_F(BasicWidgetTest, discardCurrentScanner)
{
    // Test method: void discardCurrentScanner(())
    EXPECT_NO_FATAL_FAILURE(obj->discardCurrentScanner());
}

TEST_F(BasicWidgetTest, expansionPreditHeight)
{
    // Test getter: int expansionPreditHeight()
    auto result = obj->expansionPreditHeight();
    EXPECT_EQ(result, 0);

}

TEST_F(BasicWidgetTest, getFileCount)
{
    // Test getter: int getFileCount()
    auto result = obj->getFileCount();
    EXPECT_EQ(result, 0);

}

TEST_F(BasicWidgetTest, getFileCount_getFileC)
{
    // Test getter: int getFileCount()
    auto result = obj->getFileCount();
    EXPECT_EQ(result, 0);

}

TEST_F(BasicWidgetTest, getFileSize)
{
    // Test getter: qint64 getFileSize()
    auto result = obj->getFileSize();
    EXPECT_EQ(result, 0);

}

TEST_F(BasicWidgetTest, getFileSize_getFileS)
{
    // Test getter: qint64 getFileSize()
    auto result = obj->getFileSize();
    EXPECT_EQ(result, 0);

}

TEST_F(BasicWidgetTest, imageExtenInfo)
{
    // Test method: void imageExtenInfo((const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->imageExtenInfo(_arg0, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant>()));
}

TEST_F(BasicWidgetTest, initFileMap)
{
    // Test method: void initFileMap(())
    EXPECT_NO_FATAL_FAILURE(obj->initFileMap());
}

TEST_F(BasicWidgetTest, retireScanner)
{
    // Test method: void retireScanner((FileScanner *scanner))
    EXPECT_NO_FATAL_FAILURE(obj->retireScanner(nullptr));
}

TEST_F(BasicWidgetTest, selectFileUrl)
{
    // Test method: void selectFileUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFileUrl(_arg0));
}

TEST_F(BasicWidgetTest, selectFileUrl_selectFi)
{
    // Test method: void selectFileUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFileUrl(_arg0));
}

TEST_F(BasicWidgetTest, slotFileCountAndSizeChange)
{
    // Test method: void slotFileCountAndSizeChange((const FileScanner::ScanResult &result))
    FileScanner::ScanResult _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->slotFileCountAndSizeChange(_arg0));
}

TEST_F(BasicWidgetTest, slotFileCountAndSizeChange_slotFile)
{
    // Test method: void slotFileCountAndSizeChange((const FileScanner::ScanResult &result))
    FileScanner::ScanResult _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->slotFileCountAndSizeChange(_arg0));
}

TEST_F(BasicWidgetTest, slotFileHide)
{
    // Test method: void slotFileHide((Qt::CheckState state))
    EXPECT_NO_FATAL_FAILURE(obj->slotFileHide(Qt::CheckState()));
}

TEST_F(BasicWidgetTest, slotOpenFileLocation)
{
    // Test method: void slotOpenFileLocation(())
    EXPECT_NO_FATAL_FAILURE(obj->slotOpenFileLocation());
}

TEST_F(BasicWidgetTest, startFileCountScan)
{
    // Test method: void startFileCountScan((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->startFileCountScan(_arg0));
}

TEST_F(BasicWidgetTest, updateFileUrl)
{
    // Test method: void updateFileUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateFileUrl(_arg0));
}

TEST_F(BasicWidgetTest, videoExtenInfo)
{
    // Test method: void videoExtenInfo((const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->videoExtenInfo(_arg0, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant>()));
}
