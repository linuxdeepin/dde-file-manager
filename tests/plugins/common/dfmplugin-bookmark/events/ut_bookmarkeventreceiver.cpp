// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "events/bookmarkeventreceiver.h"
#include "controller/bookmarkmanager.h"
#include "controller/defaultitemmanager.h"
#include <dfm-base/base/application/application.h>
#include <dfm-framework/event/event.h>
#include <gtest/gtest.h>

using namespace dfmplugin_bookmark;
class BookMarkEventReceiverTest : public testing::Test
{
public:
    void SetUp() override
    {
        ins = BookMarkEventReceiver::instance();
    }
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    BookMarkEventReceiver *ins;
};

TEST_F(BookMarkEventReceiverTest, handleRenameFile)
{
    QMap<QUrl, QUrl> renamedUrls;
    renamedUrls.insert(QUrl("/hello"), QUrl("/hello"));
    bool isRun = false;
    stub.set_lamda(&BookMarkManager::fileRenamed, [&isRun]() { isRun = true; });
    ins->handleRenameFile(1, renamedUrls, true, QString());
    EXPECT_TRUE(isRun);
}

TEST_F(BookMarkEventReceiverTest, handleAddSchemeOfBookMarkDisabled)
{
    bool isRun = false;
    stub.set_lamda(&BookMarkManager::addSchemeOfBookMarkDisabled, [&isRun]() { isRun = true; });
    ins->handleAddSchemeOfBookMarkDisabled("/hello");
    EXPECT_TRUE(isRun);
}

TEST_F(BookMarkEventReceiverTest, handleSidebarOrderChanged)
{
    bool isRun = false;
    stub.set_lamda(&BookMarkManager::sortItemsByOrder, [&isRun]() { isRun = true; });
    ins->handleSidebarOrderChanged(1, "Group_Common");
    EXPECT_TRUE(isRun);
}

TEST_F(BookMarkEventReceiverTest, handleItemSort)
{
    bool isRun = false;
    stub.set_lamda(&BookMarkManager::handleItemSort, [&isRun]() { isRun = true;  return true; });
    ins->handleItemSort("Group_Common", "", QUrl("/hello"), QUrl("/hello"));
    EXPECT_TRUE(isRun);
}

TEST_F(BookMarkEventReceiverTest, handlePluginItem)
{
    bool isRun = false;
    stub.set_lamda(&DefaultItemManager::addPluginItem, [&isRun]() { isRun = true; });
    ins->handlePluginItem(QVariantMap());
    EXPECT_TRUE(isRun);
}
