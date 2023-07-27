// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "bookmarkcallback.h"
#include "events/bookmarkeventcaller.h"
#include "controller/bookmarkmanager.h"
#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/event/eventchannel.h>
#include <dfm-framework/event/eventdispatcher.h>

#include <gtest/gtest.h>
#include <QAction>
#include <QMenu>
#include <QApplication>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_bookmark;
using namespace dpf;

class UT_BookmarkCallBack : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_BookmarkCallBack, contextMenuHandle)
{
    auto exec_QPoint_QAction = static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec);
    QAction *act = new QAction("hello");
    stub.set_lamda(exec_QPoint_QAction, [=] { __DBG_STUB_INVOKE__ return act; });

    typedef bool (dpf::EventDispatcherManager::*Publish)(const QString &, const QString &, QString, QList<QUrl> &);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(BookmarkCallBack::contextMenuHandle(0, QUrl(), QPoint()));
    delete act;
}

TEST_F(UT_BookmarkCallBack, renameCallBack)
{
    bool isCall { false };
    typedef QVariant (EventChannelManager::*FuncType)(const QString &, const QString &, QUrl, QVariantMap &);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [&isCall] {
        isCall = true;
        return true;
    });
    stub.set_lamda(&BookMarkManager::bookMarkRename, [] { return true; });
    BookmarkCallBack::renameCallBack(1, QUrl(), "test");

    EXPECT_TRUE(isCall);
}

TEST_F(UT_BookmarkCallBack, cdBookMarkUrlCallBack)
{
    bool isCall { false };
    QUrl url;
    url.setScheme("smb");
    stub.set_lamda(&BookMarkManager::getBookMarkDataMap, [url, &isCall] {
        QMap<QUrl, BookmarkData> data;
        isCall = true;
        data.insert(QUrl("test"), BookmarkData());
        data.insert(url, BookmarkData());
        return data;
    });
    stub.set_lamda(&QApplication::restoreOverrideCursor, [] {});
    stub.set_lamda(&BookMarkManager::showRemoveBookMarkDialog, [] { return 1; });
    stub.set_lamda(&BookMarkManager::removeBookMark, [] { return true; });

    BookmarkCallBack::cdBookMarkUrlCallBack(1, url);
    BookmarkCallBack::cdBookMarkUrlCallBack(1, QUrl("test1"));
    BookmarkCallBack::cdBookMarkUrlCallBack(1, QUrl("test"));

    EXPECT_TRUE(isCall);
}

TEST_F(UT_BookmarkCallBack, cdDefaultItemUrlCallBack)
{
    bool isCall { false };
    stub.set_lamda(&BookMarkEventCaller::sendOpenBookMarkInWindow, [&isCall] {
        isCall = true;
    });
    stub.set_lamda(&BookMarkManager::removeBookMark, [] { return true; });
    BookmarkCallBack::cdDefaultItemUrlCallBack(1, QUrl("test"));
    EXPECT_TRUE(isCall);
}
