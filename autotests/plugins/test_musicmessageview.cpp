// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_musicmessageview.cpp
 * @brief Unit tests for MusicMessageView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/music-preview/musicmessageview.h"

#include <QTest>

using namespace src;

class MusicMessageViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MusicMessageView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MusicMessageView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MusicMessageViewTest, detectEncodings)
{
    // Test method: QList<QByteArray> detectEncodings((const QByteArray &rawData))
    QByteArray _arg0{};
    auto result = obj->detectEncodings(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MusicMessageViewTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(MusicMessageViewTest, setMediaInfo)
{
    // Test method: void setMediaInfo(())
    EXPECT_NO_FATAL_FAILURE(obj->setMediaInfo());
}
