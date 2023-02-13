// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/filemanager/dfmplugin-myshares/events/shareeventhelper.h"
#include "plugins/filemanager/dfmplugin-myshares/events/shareeventscaller.h"

#include <gtest/gtest.h>

class UT_ShareEventHelper : public testing::Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

using namespace dfmplugin_myshares;

TEST_F(UT_ShareEventHelper, BlockPaste)
{
    EXPECT_TRUE(ShareEventHelper::instance()->blockPaste(0, {}, QUrl("usershare:///hello")));
    EXPECT_FALSE(ShareEventHelper::instance()->blockPaste(0, {}, QUrl("file:///world")));
}

TEST_F(UT_ShareEventHelper, BlockDelete)
{
    stub.set_lamda(&ShareEventHelper::containsShareUrl, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(ShareEventHelper::instance()->blockDelete(0, { QUrl("usershare:///hello") }));

    stub.set_lamda(&ShareEventHelper::containsShareUrl, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(ShareEventHelper::instance()->blockDelete(0, { QUrl("usershare:///hello") }));
}

TEST_F(UT_ShareEventHelper, BlockMoveToTrash)
{
    stub.set_lamda(&ShareEventHelper::containsShareUrl, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(ShareEventHelper::instance()->blockMoveToTrash(0, { QUrl("usershare:///hello") }));

    stub.set_lamda(&ShareEventHelper::containsShareUrl, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(ShareEventHelper::instance()->blockMoveToTrash(0, { QUrl("usershare:///hello") }));
}

TEST_F(UT_ShareEventHelper, HookSendOpenWindow)
{
    stub.set_lamda(ShareEventsCaller::sendOpenDirs, [] { __DBG_STUB_INVOKE__ });
    EXPECT_FALSE(ShareEventHelper::instance()->hookSendOpenWindow({}));
    EXPECT_FALSE(ShareEventHelper::instance()->hookSendOpenWindow({ QUrl::fromLocalFile("/hello") }));
    EXPECT_FALSE(ShareEventHelper::instance()->hookSendOpenWindow({ QUrl("usershare:///") }));
    EXPECT_TRUE(ShareEventHelper::instance()->hookSendOpenWindow({ QUrl("usershare:///hello") }));
    EXPECT_TRUE(ShareEventHelper::instance()->hookSendOpenWindow({ QUrl("usershare:///hello"), QUrl::fromLocalFile("/hello") }));
}

TEST_F(UT_ShareEventHelper, HookSendChangeCurrentUrl)
{
    stub.set_lamda(ShareEventsCaller::sendOpenDirs, [] { __DBG_STUB_INVOKE__ });
    EXPECT_FALSE(ShareEventHelper::instance()->hookSendChangeCurrentUrl(0, QUrl::fromLocalFile("/hello/world")));
    EXPECT_FALSE(ShareEventHelper::instance()->hookSendChangeCurrentUrl(0, QUrl("usershare:///")));
    EXPECT_TRUE(ShareEventHelper::instance()->hookSendChangeCurrentUrl(0, QUrl("usershare:///hello/world")));
    EXPECT_TRUE(ShareEventHelper::instance()->hookSendChangeCurrentUrl(12345, QUrl("usershare:///hello/world")));
}

TEST_F(UT_ShareEventHelper, ContainsShareUrl)
{
    QList<QUrl> urls;
    EXPECT_FALSE(ShareEventHelper::instance()->containsShareUrl(urls));

    urls = { QUrl("usershare:///hello/world"), QUrl::fromLocalFile("/hello/c++") };
    EXPECT_TRUE(ShareEventHelper::instance()->containsShareUrl(urls));

    urls = { QUrl::fromLocalFile("/this/is/local/file") };
    EXPECT_FALSE(ShareEventHelper::instance()->containsShareUrl(urls));
}
