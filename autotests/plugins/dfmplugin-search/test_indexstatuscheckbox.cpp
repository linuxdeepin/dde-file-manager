// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QString>

#include "utils/indexstatuscheckbox.h"

using namespace dfmplugin_search;

class IndexStatusCheckBoxTest : public testing::Test
{
protected:
    void SetUp() override
    {
        box = new IndexStatusCheckBox();
    }

    void TearDown() override
    {
        delete box;
    }

    IndexStatusCheckBox *box = nullptr;
};

// --- construction and default status ---

TEST_F(IndexStatusCheckBoxTest, DefaultStatus_IsInactive)
{
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

// --- setDisplayText / setChecked / isChecked ---

TEST_F(IndexStatusCheckBoxTest, SetChecked_True_ReturnsTrue)
{
    box->setChecked(true);
    EXPECT_TRUE(box->isChecked());
}

TEST_F(IndexStatusCheckBoxTest, SetChecked_False_ReturnsFalse)
{
    box->setChecked(true);
    box->setChecked(false);
    EXPECT_FALSE(box->isChecked());
}

TEST_F(IndexStatusCheckBoxTest, SetChecked_EmitsCheckStateChanged)
{
    QSignalSpy spy(box, &IndexStatusCheckBox::checkStateChanged);
    box->setChecked(true);
    EXPECT_GE(spy.count(), 1);
}

// --- setInactiveText ---

TEST_F(IndexStatusCheckBoxTest, SetInactiveText_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(box->setInactiveText("inactive text"));
}

// --- setIndexingTexts ---

TEST_F(IndexStatusCheckBoxTest, SetIndexingTexts_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(box->setIndexingTexts("initial", "files", "items"));
}

// --- setCompletedText ---

TEST_F(IndexStatusCheckBoxTest, SetCompletedText_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(box->setCompletedText("completed", "link", "update"));
}

TEST_F(IndexStatusCheckBoxTest, SetCompletedText_DefaultHref)
{
    EXPECT_NO_FATAL_FAILURE(box->setCompletedText("completed", "link"));
}

// --- setFailedText ---

TEST_F(IndexStatusCheckBoxTest, SetFailedText_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(box->setFailedText("failed", "retry", "update"));
}

TEST_F(IndexStatusCheckBoxTest, SetFailedText_DefaultHref)
{
    EXPECT_NO_FATAL_FAILURE(box->setFailedText("failed", "retry"));
}

// --- setWaitingText ---

TEST_F(IndexStatusCheckBoxTest, SetWaitingText_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(box->setWaitingText("waiting", "continue", "update"));
}

TEST_F(IndexStatusCheckBoxTest, SetWaitingText_DefaultHref)
{
    EXPECT_NO_FATAL_FAILURE(box->setWaitingText("waiting", "continue"));
}

// --- setStatus / status ---

TEST_F(IndexStatusCheckBoxTest, SetStatus_Indexing_UpdatesStatus)
{
    box->setStatus(IndexStatusCheckBox::Status::Indexing);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Indexing);
}

TEST_F(IndexStatusCheckBoxTest, SetStatus_Completed_UpdatesStatus)
{
    box->setStatus(IndexStatusCheckBox::Status::Completed);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Completed);
}

TEST_F(IndexStatusCheckBoxTest, SetStatus_Failed_UpdatesStatus)
{
    box->setStatus(IndexStatusCheckBox::Status::Failed);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Failed);
}

TEST_F(IndexStatusCheckBoxTest, SetStatus_WaitingPower_UpdatesStatus)
{
    box->setStatus(IndexStatusCheckBox::Status::WaitingPower);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::WaitingPower);
}

TEST_F(IndexStatusCheckBoxTest, SetStatus_WaitingPowerSave_UpdatesStatus)
{
    box->setStatus(IndexStatusCheckBox::Status::WaitingPowerSave);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::WaitingPowerSave);
}

TEST_F(IndexStatusCheckBoxTest, SetStatus_WaitingIdle_UpdatesStatus)
{
    box->setStatus(IndexStatusCheckBox::Status::WaitingIdle);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::WaitingIdle);
}

TEST_F(IndexStatusCheckBoxTest, SetStatus_WaitingUpgrade_UpdatesStatus)
{
    box->setStatus(IndexStatusCheckBox::Status::WaitingUpgrade);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::WaitingUpgrade);
}

TEST_F(IndexStatusCheckBoxTest, SetStatus_Inactive_UpdatesStatus)
{
    box->setStatus(IndexStatusCheckBox::Status::Indexing);
    box->setStatus(IndexStatusCheckBox::Status::Inactive);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(IndexStatusCheckBoxTest, SetStatus_AllStates_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(box->setStatus(IndexStatusCheckBox::Status::Indexing));
    EXPECT_NO_FATAL_FAILURE(box->setStatus(IndexStatusCheckBox::Status::Completed));
    EXPECT_NO_FATAL_FAILURE(box->setStatus(IndexStatusCheckBox::Status::Failed));
    EXPECT_NO_FATAL_FAILURE(box->setStatus(IndexStatusCheckBox::Status::Inactive));
    EXPECT_NO_FATAL_FAILURE(box->setStatus(IndexStatusCheckBox::Status::WaitingPower));
    EXPECT_NO_FATAL_FAILURE(box->setStatus(IndexStatusCheckBox::Status::WaitingPowerSave));
    EXPECT_NO_FATAL_FAILURE(box->setStatus(IndexStatusCheckBox::Status::WaitingIdle));
    EXPECT_NO_FATAL_FAILURE(box->setStatus(IndexStatusCheckBox::Status::WaitingUpgrade));
}

// --- updateIndexingProgress ---

TEST_F(IndexStatusCheckBoxTest, UpdateIndexingProgress_NoCrash)
{
    box->setStatus(IndexStatusCheckBox::Status::Indexing);
    EXPECT_NO_FATAL_FAILURE(box->updateIndexingProgress(100, 200));
}

TEST_F(IndexStatusCheckBoxTest, UpdateIndexingProgress_ZeroTotal_NoCrash)
{
    box->setStatus(IndexStatusCheckBox::Status::Indexing);
    EXPECT_NO_FATAL_FAILURE(box->updateIndexingProgress(0, 0));
}

TEST_F(IndexStatusCheckBoxTest, UpdateIndexingProgress_Complete_NoCrash)
{
    box->setStatus(IndexStatusCheckBox::Status::Indexing);
    EXPECT_NO_FATAL_FAILURE(box->updateIndexingProgress(200, 200));
}
