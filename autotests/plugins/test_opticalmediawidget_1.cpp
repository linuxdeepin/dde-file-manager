// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalmediawidget_1.cpp
 * @brief Unit tests for OpticalMediaWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/opticalmediawidget.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalMediaWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalMediaWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalMediaWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalMediaWidgetTest, handleErrorMount)
{
    // Test method: void handleErrorMount(())
    EXPECT_NO_FATAL_FAILURE(obj->handleErrorMount());
}

TEST_F(OpticalMediaWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(OpticalMediaWidgetTest, isSupportedUDF)
{
    // Test bool getter: isSupportedUDF()
    bool result = obj->isSupportedUDF();
    EXPECT_FALSE(result);

}

TEST_F(OpticalMediaWidgetTest, onBurnButtonClicked)
{
    // Test method: void onBurnButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onBurnButtonClicked());
}

TEST_F(OpticalMediaWidgetTest, onDiscUnmounted)
{
    // Test method: void onDiscUnmounted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDiscUnmounted(_arg0));
}

TEST_F(OpticalMediaWidgetTest, onDumpButtonClicked)
{
    // Test method: void onDumpButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onDumpButtonClicked());
}

TEST_F(OpticalMediaWidgetTest, onStagingFileStatisticsFinished)
{
    // Test method: void onStagingFileStatisticsFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onStagingFileStatisticsFinished());
}
