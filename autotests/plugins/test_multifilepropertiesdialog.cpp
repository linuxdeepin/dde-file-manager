// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_multifilepropertiesdialog.cpp
 * @brief Unit tests for MultiFilePropertiesDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/multifilepropertiesdialog.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class MultiFilePropertiesDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MultiFilePropertiesDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MultiFilePropertiesDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MultiFilePropertiesDialogTest, changeFilesGroupState)
{
    // Test method: bool changeFilesGroupState((int index, QString &strChange))
    QString _arg1{};
    auto result = obj->changeFilesGroupState(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(MultiFilePropertiesDialogTest, changeFilesHideState)
{
    // Test method: bool changeFilesHideState((int state, QString &strChange))
    QString _arg1{};
    auto result = obj->changeFilesHideState(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(MultiFilePropertiesDialogTest, changeFilesOtherState)
{
    // Test method: bool changeFilesOtherState((int index, QString &strChange))
    QString _arg1{};
    auto result = obj->changeFilesOtherState(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(MultiFilePropertiesDialogTest, changeFilesOwnerState)
{
    // Test method: bool changeFilesOwnerState((int index, QString &strChange))
    QString _arg1{};
    auto result = obj->changeFilesOwnerState(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(MultiFilePropertiesDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(MultiFilePropertiesDialogTest, saveBtnClicked)
{
    // Test method: void saveBtnClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->saveBtnClicked());
}
