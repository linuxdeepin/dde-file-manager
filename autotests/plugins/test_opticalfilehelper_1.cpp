// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalfilehelper_1.cpp
 * @brief Unit tests for OpticalFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/opticalfilehelper.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalFileHelperTest, OpticalFileHelper)
{
    // Test constructor: OpticalFileHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpticalFileHelperTest, copyFile)
{
    // Test method: bool copyFile((const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags))
    auto result = obj->copyFile(0, QList<QUrl>(), QUrl(), DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(OpticalFileHelperTest, cutFile)
{
    // Test method: bool cutFile((const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags))
    auto result = obj->cutFile(0, QList<QUrl>(), QUrl(), DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(OpticalFileHelperTest, instance)
{
    // Test getter: DPOPTICAL_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(OpticalFileHelperTest, openFileInPlugin)
{
    // Test method: bool openFileInPlugin((quint64 winId, QList<QUrl> urls))
    auto result = obj->openFileInPlugin(0, QList<QUrl>());
    EXPECT_FALSE(result);

}

TEST_F(OpticalFileHelperTest, openFileInTerminal)
{
    // Test method: bool openFileInTerminal((const quint64 windowId, const QList<QUrl> urls))
    auto result = obj->openFileInTerminal(0, QList<QUrl>());
    EXPECT_FALSE(result);

}

TEST_F(OpticalFileHelperTest, pasteFilesHandle)
{
    // Test method: void pasteFilesHandle((const QList<QUrl> sources, const QUrl target, bool isCopy))
    EXPECT_NO_FATAL_FAILURE(obj->pasteFilesHandle(QList<QUrl>(), QUrl(), false));
}
