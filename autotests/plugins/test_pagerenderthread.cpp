// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pagerenderthread.cpp
 * @brief Unit tests for PageRenderThread methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/pagerenderthread.h"

#include <QTest>

using namespace src;

class PageRenderThreadTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PageRenderThread();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PageRenderThread *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PageRenderThreadTest, PageRenderThread)
{
    // Test constructor: PageRenderThread((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PageRenderThreadTest, appendTask)
{
    // Test method: void appendTask((DocCloseTask task))
    EXPECT_NO_FATAL_FAILURE(obj->appendTask(DocCloseTask()));
}

TEST_F(PageRenderThreadTest, destroyForever)
{
    // Test method: void destroyForever(())
    EXPECT_NO_FATAL_FAILURE(obj->destroyForever());
}

TEST_F(PageRenderThreadTest, execNextDocCloseTask)
{
    // Test bool getter: execNextDocCloseTask()
    bool result = obj->execNextDocCloseTask();
    EXPECT_FALSE(result);

}

TEST_F(PageRenderThreadTest, execNextDocPageNormalImageTask)
{
    // Test bool getter: execNextDocPageNormalImageTask()
    bool result = obj->execNextDocPageNormalImageTask();
    EXPECT_FALSE(result);

}

TEST_F(PageRenderThreadTest, execNextDocPageThumbnailTask)
{
    // Test bool getter: execNextDocPageThumbnailTask()
    bool result = obj->execNextDocPageThumbnailTask();
    EXPECT_FALSE(result);

}

TEST_F(PageRenderThreadTest, hasNextTask)
{
    // Test bool getter: hasNextTask()
    bool result = obj->hasNextTask();
    EXPECT_FALSE(result);

}

TEST_F(PageRenderThreadTest, instance)
{
    // Test getter: PageRenderThread instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(PageRenderThreadTest, onDocOpenTask)
{
    // Test method: void onDocOpenTask((DocOpenTask task, Document::Error error, Document *document, QList<Page *> pages))
    EXPECT_NO_FATAL_FAILURE(obj->onDocOpenTask(DocOpenTask(), Document::Error(), nullptr, nullptr));
}

TEST_F(PageRenderThreadTest, onDocPageNormalImageTaskFinished)
{
    // Test method: void onDocPageNormalImageTaskFinished((DocPageNormalImageTask task, QPixmap pixmap))
    EXPECT_NO_FATAL_FAILURE(obj->onDocPageNormalImageTaskFinished(DocPageNormalImageTask(), QPixmap()));
}

TEST_F(PageRenderThreadTest, onDocPageThumbnailTask)
{
    // Test method: void onDocPageThumbnailTask((DocPageThumbnailTask task, QPixmap pixmap))
    EXPECT_NO_FATAL_FAILURE(obj->onDocPageThumbnailTask(DocPageThumbnailTask(), QPixmap()));
}

TEST_F(PageRenderThreadTest, popNextDocCloseTask)
{
    // Test method: bool popNextDocCloseTask((DocCloseTask &task))
    DocCloseTask _arg0{};
    auto result = obj->popNextDocCloseTask(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PageRenderThreadTest, popNextDocOpenTask)
{
    // Test method: bool popNextDocOpenTask((DocOpenTask &task))
    DocOpenTask _arg0{};
    auto result = obj->popNextDocOpenTask(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PageRenderThreadTest, popNextDocPageNormalImageTask)
{
    // Test method: bool popNextDocPageNormalImageTask((DocPageNormalImageTask &task))
    DocPageNormalImageTask _arg0{};
    auto result = obj->popNextDocPageNormalImageTask(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PageRenderThreadTest, popNextDocPageThumbnailTask)
{
    // Test method: bool popNextDocPageThumbnailTask((DocPageThumbnailTask &task))
    DocPageThumbnailTask _arg0{};
    auto result = obj->popNextDocPageThumbnailTask(_arg0);
    EXPECT_FALSE(result);

}
