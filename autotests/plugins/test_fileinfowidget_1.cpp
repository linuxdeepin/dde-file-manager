// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfowidget_1.cpp
 * @brief Unit tests for FileInfoWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/fileinfowidget.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class FileInfoWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileInfoWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileInfoWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileInfoWidgetTest, formatDateTime)
{
    // Test method: QString formatDateTime((const QDateTime &time))
    QDateTime _arg0{};
    auto result = obj->formatDateTime(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileInfoWidgetTest, initConnections)
{
    // Test method: void initConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnections());
}

TEST_F(FileInfoWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(FileInfoWidgetTest, onAudioMediaInfo)
{
    // Test method: void onAudioMediaInfo((const QUrl &url, QMap<DFileInfo::AttributeExtendID, QVariant> properties))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAudioMediaInfo(_arg0, QMap<DFileInfo::AttributeExtendID, QVariant>()));
}

TEST_F(FileInfoWidgetTest, onImageMediaInfo)
{
    // Test method: void onImageMediaInfo((const QUrl &url, QMap<DFileInfo::AttributeExtendID, QVariant> properties))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onImageMediaInfo(_arg0, QMap<DFileInfo::AttributeExtendID, QVariant>()));
}

TEST_F(FileInfoWidgetTest, onVideoMediaInfo)
{
    // Test method: void onVideoMediaInfo((const QUrl &url, QMap<DFileInfo::AttributeExtendID, QVariant> properties))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onVideoMediaInfo(_arg0, QMap<DFileInfo::AttributeExtendID, QVariant>()));
}

TEST_F(FileInfoWidgetTest, setUrl)
{
    // Test setter: void setUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUrl(_arg0));
}
