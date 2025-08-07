// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "views/computerstatusbar.h"

#include <QString>
#include <QLabel>

using namespace dfmplugin_computer;

class UT_ComputerStatusBar : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        statusBar = new ComputerStatusBar(nullptr);
    }

    virtual void TearDown() override
    {
        delete statusBar;
        statusBar = nullptr;
    }

protected:
    ComputerStatusBar *statusBar = nullptr;
};

TEST_F(UT_ComputerStatusBar, ShowSingleSelectionMessage)
{
    EXPECT_NO_FATAL_FAILURE(statusBar->showSingleSelectionMessage());
}
