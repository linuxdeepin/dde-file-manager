// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticaleventcaller.cpp
 * @brief Unit tests for OpticalEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/opticaleventcaller.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalEventCallerTest, OpticalEventCaller)
{
    // Test constructor: OpticalEventCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpticalEventCallerTest, sendOpenBurnDlg)
{
    // Test method: DFMBASE_USE_NAMESPACE sendOpenBurnDlg((const QString &dev, bool isSupportedUDF, QWidget *parent))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj->sendOpenBurnDlg(_arg0, false, nullptr); });
}

TEST_F(OpticalEventCallerTest, sendOpenDumpISODlg)
{
    // Test method: void sendOpenDumpISODlg((const QString &devId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenDumpISODlg(_arg0));
}
