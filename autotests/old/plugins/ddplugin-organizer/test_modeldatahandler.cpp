// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "models/modeldatahandler.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QVector>

using namespace ddplugin_organizer;

class UT_ModelDataHandler : public testing::Test
{
protected:
    void SetUp() override
    {
        handler = new ModelDataHandler();
    }

    void TearDown() override
    {
        delete handler;
        handler = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    ModelDataHandler *handler = nullptr;
};

TEST_F(UT_ModelDataHandler, Constructor_Default_InitializesCorrectly)
{
    EXPECT_NE(handler, nullptr);
}

TEST_F(UT_ModelDataHandler, acceptInsert_WithValidUrl_ReturnsTrue)
{
    QUrl url("file:///home/test/file.txt");
    bool result = handler->acceptInsert(url);
    EXPECT_TRUE(result);
}

TEST_F(UT_ModelDataHandler, acceptInsert_WithInvalidUrl_ReturnsFalse)
{
    QUrl url;
    bool result = handler->acceptInsert(url);
    EXPECT_FALSE(result);
}

TEST_F(UT_ModelDataHandler, acceptReset_WithValidUrls_ReturnsUrls)
{
    QList<QUrl> urls;
    urls << QUrl("file:///home/test/file1.txt");
    urls << QUrl("file:///home/test/file2.txt");
    
    QList<QUrl> result = handler->acceptReset(urls);
    EXPECT_EQ(result.size(), urls.size());
}

TEST_F(UT_ModelDataHandler, acceptReset_WithEmptyUrls_ReturnsEmptyList)
{
    QList<QUrl> urls;
    QList<QUrl> result = handler->acceptReset(urls);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_ModelDataHandler, acceptRename_WithValidUrls_ReturnsTrue)
{
    QUrl oldUrl("file:///home/test/old_file.txt");
    QUrl newUrl("file:///home/test/new_file.txt");
    bool result = handler->acceptRename(oldUrl, newUrl);
    EXPECT_TRUE(result);
}

TEST_F(UT_ModelDataHandler, acceptUpdate_WithValidUrl_ReturnsTrue)
{
    QUrl url("file:///home/test/file.txt");
    QVector<int> roles;
    bool result = handler->acceptUpdate(url, roles);
    EXPECT_TRUE(result);
}

TEST_F(UT_ModelDataHandler, acceptUpdate_WithEmptyUrl_ReturnsFalse)
{
    QUrl url;
    QVector<int> roles;
    bool result = handler->acceptUpdate(url, roles);
    EXPECT_FALSE(result);
}
