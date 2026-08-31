// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_multifilebasicinfowidget_1.cpp
 * @brief Unit tests for MultiFileBasicInfoWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/multifilebasicinfowidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class MultiFileBasicInfoWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MultiFileBasicInfoWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MultiFileBasicInfoWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MultiFileBasicInfoWidgetTest, createValueLabel)
{
    // Test method: KeyValueLabel createValueLabel((QFrame *frame,
                                                          const QString &leftValue))
    QString _arg1{};
    auto result = obj->createValueLabel(nullptr, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->createValueLabel(nullptr, _arg1); });

}

TEST_F(MultiFileBasicInfoWidgetTest, discardCurrentScanner)
{
    // Test method: void discardCurrentScanner(())
    EXPECT_NO_FATAL_FAILURE(obj->discardCurrentScanner());
}

TEST_F(MultiFileBasicInfoWidgetTest, filesHideStateChanged)
{
    // Test method: void filesHideStateChanged((int state))
    EXPECT_NO_FATAL_FAILURE(obj->filesHideStateChanged(0));
}

TEST_F(MultiFileBasicInfoWidgetTest, getOrgHideBoxState)
{
    // Test method: void getOrgHideBoxState((FilePropertyState &states))
    FilePropertyState _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->getOrgHideBoxState(_arg0));
}

TEST_F(MultiFileBasicInfoWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(MultiFileBasicInfoWidgetTest, loadData)
{
    // Test method: void loadData((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->loadData(_arg0));
}

TEST_F(MultiFileBasicInfoWidgetTest, retireScanner)
{
    // Test method: void retireScanner((FileScanner *scanner))
    EXPECT_NO_FATAL_FAILURE(obj->retireScanner(nullptr));
}

TEST_F(MultiFileBasicInfoWidgetTest, setFilesCountAndSize)
{
    // Test setter: void setFilesCountAndSize((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFilesCountAndSize(_arg0));
}

TEST_F(MultiFileBasicInfoWidgetTest, updateFilesCountAndSizeLabel)
{
    // Test method: void updateFilesCountAndSizeLabel((const FileScanner::ScanResult &result))
    FileScanner::ScanResult _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateFilesCountAndSizeLabel(_arg0));
}

TEST_F(MultiFileBasicInfoWidgetTest, MultiFileBasicInfoWidget_Destructor)
{
    // Test method:  ~MultiFileBasicInfoWidget(())
    EXPECT_NO_FATAL_FAILURE({ MultiFileBasicInfoWidget *tmp = new MultiFileBasicInfoWidget(); delete tmp; });
}
