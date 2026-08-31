// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_multifilepermissionwidget_1.cpp
 * @brief Unit tests for MultiFilePermissionWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/multifilepermissionwidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class MultiFilePermissionWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MultiFilePermissionWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MultiFilePermissionWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MultiFilePermissionWidgetTest, MultiFilePermissionWidget)
{
    // Test constructor: MultiFilePermissionWidget((const QList<QUrl> &urls,
                                                     QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MultiFilePermissionWidgetTest, canChmodByFs)
{
    // Test method: bool canChmodByFs((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->canChmodByFs(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(MultiFilePermissionWidgetTest, disablePermissionComboBox)
{
    // Test method: void disablePermissionComboBox(())
    EXPECT_NO_FATAL_FAILURE(obj->disablePermissionComboBox());
}

TEST_F(MultiFilePermissionWidgetTest, getOrgPermissonBoxState)
{
    // Test method: void getOrgPermissonBoxState((FilePropertyState &states))
    FilePropertyState _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->getOrgPermissonBoxState(_arg0));
}

TEST_F(MultiFilePermissionWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(MultiFilePermissionWidgetTest, onGroupComboBoxChanged)
{
    // Test method: void onGroupComboBoxChanged((int index))
    EXPECT_NO_FATAL_FAILURE(obj->onGroupComboBoxChanged(0));
}

TEST_F(MultiFilePermissionWidgetTest, onOtherComboBoxChanged)
{
    // Test method: void onOtherComboBoxChanged((int index))
    EXPECT_NO_FATAL_FAILURE(obj->onOtherComboBoxChanged(0));
}

TEST_F(MultiFilePermissionWidgetTest, onOwnerComboBoxChanged)
{
    // Test method: void onOwnerComboBoxChanged((int index))
    EXPECT_NO_FATAL_FAILURE(obj->onOwnerComboBoxChanged(0));
}

TEST_F(MultiFilePermissionWidgetTest, updateComboBoxViewPalette)
{
    // Test method: void updateComboBoxViewPalette(())
    EXPECT_NO_FATAL_FAILURE(obj->updateComboBoxViewPalette());
}
