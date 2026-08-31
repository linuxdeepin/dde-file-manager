// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfseventhandler_1.cpp
 * @brief Unit tests for AvfsEventHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/avfseventhandler.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsEventHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsEventHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsEventHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsEventHandlerTest, AvfsEventHandler)
{
    // Test constructor: AvfsEventHandler(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(AvfsEventHandlerTest, hookEnterPressed)
{
    // Test method: bool hookEnterPressed((quint64 winId, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    auto result = obj->hookEnterPressed(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(AvfsEventHandlerTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}

TEST_F(AvfsEventHandlerTest, openArchivesAsDir)
{
    // Test method: void openArchivesAsDir((quint64 winId, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->openArchivesAsDir(0, _arg1));
}

TEST_F(AvfsEventHandlerTest, sepateTitlebarCrumb)
{
    // Test method: bool sepateTitlebarCrumb((const QUrl &url, QList<QVariantMap> *mapGroup))
    QUrl _arg0{};
    auto result = obj->sepateTitlebarCrumb(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AvfsEventHandlerTest, showProperty)
{
    // Test method: void showProperty((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showProperty(_arg0));
}
