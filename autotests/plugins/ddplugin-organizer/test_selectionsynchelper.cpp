// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "mode/selectionsynchelper.h"
#include "models/itemselectionmodel.h"

#include <QItemSelectionModel>
#include <QAbstractItemModel>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_SelectionSyncHelper : public testing::Test
{
protected:
    void SetUp() override
    {
        helper = new SelectionSyncHelper();
    }

    void TearDown() override
    {
        delete helper;
        helper = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    SelectionSyncHelper *helper = nullptr;
};

TEST_F(UT_SelectionSyncHelper, Constructor_CreatesHelper)
{
    EXPECT_NE(helper, nullptr);
    EXPECT_FALSE(helper->enabled);
    EXPECT_EQ(helper->shell, nullptr);
    EXPECT_EQ(helper->inner, nullptr);
    EXPECT_EQ(helper->external, nullptr);
}

TEST_F(UT_SelectionSyncHelper, SetInnerModel_SetsModel)
{
    ItemSelectionModel model;
    helper->setInnerModel(&model);
    EXPECT_EQ(helper->inner, &model);
}

TEST_F(UT_SelectionSyncHelper, SetExternalModel_SetsModel)
{
    QItemSelectionModel model(nullptr);
    helper->setExternalModel(&model);
    EXPECT_EQ(helper->external, &model);
}

TEST_F(UT_SelectionSyncHelper, SetShell_SetsShell)
{
    // Create a mock CanvasSelectionShell
    CanvasSelectionShell shell;
    helper->setShell(&shell);
    EXPECT_EQ(helper->shell, &shell);
}

TEST_F(UT_SelectionSyncHelper, SetEnabled_SetsEnabledState)
{
    helper->setEnabled(true);
    EXPECT_TRUE(helper->enabled);
    
    helper->setEnabled(false);
    EXPECT_FALSE(helper->enabled);
}

TEST_F(UT_SelectionSyncHelper, ClearExteralSelection_DoesNotCrash)
{
    helper->clearExteralSelection();
    SUCCEED();
}

TEST_F(UT_SelectionSyncHelper, ClearInnerSelection_DoesNotCrash)
{
    helper->clearInnerSelection();
    SUCCEED();
}

TEST_F(UT_SelectionSyncHelper, InnerModelDestroyed_DoesNotCrash)
{
    helper->innerModelDestroyed();
    SUCCEED();
}

TEST_F(UT_SelectionSyncHelper, ExternalModelDestroyed_DoesNotCrash)
{
    helper->externalModelDestroyed();
    SUCCEED();
}
