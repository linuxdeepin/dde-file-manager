// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalfilehelper.cpp
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

TEST_F(OpticalFileHelperTest, moveToTrash)
{
    // Test method: bool moveToTrash((const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags))
    auto result = obj->moveToTrash(0, QList<QUrl>(), DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(OpticalFileHelperTest, writeUrlsToClipboard)
{
    // Test method: bool writeUrlsToClipboard((const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action, const QList<QUrl> urls))
    auto result = obj->writeUrlsToClipboard(0, DFMBASE_NAMESPACE::ClipBoard::ClipboardAction(), QList<QUrl>());
    EXPECT_FALSE(result);

}
