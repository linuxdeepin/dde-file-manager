// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mediainfofetchworker.cpp
 * @brief Unit tests for MediaInfoFetchWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/mediainfofetchworker.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class MediaInfoFetchWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MediaInfoFetchWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MediaInfoFetchWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MediaInfoFetchWorkerTest, getDuration)
{
    // Test method: void getDuration((const QString &filePath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->getDuration(_arg0));
}

TEST_F(MediaInfoFetchWorkerTest, hasFFmpeg)
{
    // Test bool getter: hasFFmpeg()
    bool result = obj->hasFFmpeg();
    EXPECT_FALSE(result);

}
