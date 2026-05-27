// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "files/recentdiriterator.h"
#include "utils/recentmanager.h"
#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
        using namespace dfmplugin_recent;

class RecentDirIteratorTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&RecentManager::init, [] { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&RecentManager::getRecentNodes, []() -> QMap<QUrl, FileInfoPointer> {
            QMap<QUrl, FileInfoPointer> map;
            map[QUrl("recent:/hello/world")] = nullptr;
            return map;
        });
        iter = new RecentDirIterator(QUrl::fromLocalFile("/"), {}, QDir::AllEntries);
        d = iter->d.data();
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete iter;
        iter = nullptr;
    }

private:
    stub_ext::StubExt stub;
    RecentDirIterator *iter { nullptr };
    RecentDirIteratorPrivate *d { nullptr };
};

TEST_F(RecentDirIteratorTest, Next)
{
    EXPECT_NO_FATAL_FAILURE(iter->next());
    EXPECT_FALSE(iter->next().isValid());
}

TEST_F(RecentDirIteratorTest, HasNext)
{
    EXPECT_NO_FATAL_FAILURE(iter->hasNext());
    EXPECT_TRUE(iter->hasNext());
}

TEST_F(RecentDirIteratorTest, FileName)
{
    EXPECT_NO_FATAL_FAILURE(iter->fileName());
    EXPECT_TRUE(iter->fileName() == "");
}

TEST_F(RecentDirIteratorTest, FileUrl)
{
    EXPECT_NO_FATAL_FAILURE(iter->fileUrl());
    EXPECT_FALSE(iter->fileUrl().isValid());
}

TEST_F(RecentDirIteratorTest, FileInfo)
{
    EXPECT_NO_FATAL_FAILURE(iter->fileInfo());
    EXPECT_TRUE(iter->fileInfo() == nullptr);
}

TEST_F(RecentDirIteratorTest, Url)
{
    EXPECT_NO_FATAL_FAILURE(iter->url());
    EXPECT_TRUE(iter->url().isValid());
}
