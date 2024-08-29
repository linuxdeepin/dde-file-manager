// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/recentmanager.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/watchercache.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/private/localfilewatcher_p.h>
#include <dfm-framework/dpf.h>
#include <gtest/gtest.h>
#include <dfm-framework/event/eventdispatcher.h>
#include <QMimeData>
#include <QMenu>
#include <DRecentManager>
#include <DDialog>

using namespace testing;
using namespace dfmplugin_recent;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPF_USE_NAMESPACE
class RecentManagerTest : public Test
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

#if 0
TEST_F(RecentManagerTest, scheme)
{
    EXPECT_TRUE(RecentHelper::scheme() == "recent");
}

TEST_F(RecentManagerTest, icon)
{
    EXPECT_TRUE(RecentHelper::icon().name() == "document-open-recent-symbolic");
}

TEST_F(RecentManagerTest, init)
{
    QSharedPointer<LocalFileWatcher> w(new LocalFileWatcher(QUrl::fromLocalFile("/home")));
    bool isCall = false;
    stub.set_lamda(&WatcherFactory::create<AbstractFileWatcher>, [&w, &isCall] {
        isCall = true;
        return w;
    });
    RecentManager::instance()->init();
    EXPECT_TRUE(isCall);
}

TEST_F(RecentManagerTest, rootUrl)
{
    QUrl url;
    url.setScheme(RecentHelper::scheme());
    url.setPath("/");
    EXPECT_TRUE(RecentHelper::rootUrl() == url);
}

TEST_F(RecentManagerTest, getRecentNodes)
{
    RecentManager::instance()->onUpdateRecentFileInfo(QUrl("file:/hello/world"), QString("file:/hello/world"), 1);
    EXPECT_TRUE(!RecentManager::instance()->getRecentNodes().isEmpty());
    EXPECT_TRUE(!RecentManager::instance()->getRecentOriginPaths().isEmpty());
    EXPECT_TRUE(RecentManager::instance()->removeRecentFile(QUrl("file:/hello/world")));
}

TEST_F(RecentManagerTest, onDeleteExistRecentUrls)
{
    RecentManager::instance()->onUpdateRecentFileInfo(QUrl("file:/hello/world"), QString("file:/hello/world"), 1);
    EXPECT_TRUE(!RecentManager::instance()->getRecentNodes().isEmpty());
    RecentManager::instance()->onDeleteExistRecentUrls(QList<QUrl>() << QUrl("file:/hello/world"));
    EXPECT_TRUE(RecentManager::instance()->getRecentNodes().isEmpty());
}

TEST_F(RecentManagerTest, onUpdateRecentFileInfo)
{
    RecentManager::instance()->updateRecent();
    RecentManager::instance()->onUpdateRecentFileInfo(QUrl("file:/hello/world"), QString("file:/hello/world"), 1);
    EXPECT_TRUE(!RecentManager::instance()->getRecentNodes().isEmpty());
}

TEST_F(RecentManagerTest, openFileLocation)
{
    typedef bool (EventDispatcherManager::*FuncType)(const EventType, const QUrl);
    int flag = 0;
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [&flag] {
        flag++;
        __DBG_STUB_INVOKE__ return true;
    });
    RecentHelper::openFileLocation(QList<QUrl>() << RecentHelper::rootUrl());
    EXPECT_EQ(flag, 1);
}

TEST_F(RecentManagerTest, urlTransform)
{
    stub.set_lamda(&UrlRoute::isRootUrl, []() -> bool {
        return false;
    });
    EXPECT_TRUE(RecentHelper::urlTransform(QUrl("recent:/hello/world")) == QUrl("file:/hello/world"));
    stub.set_lamda(&UrlRoute::isRootUrl, []() -> bool {
        return true;
    });
    EXPECT_TRUE(RecentHelper::urlTransform(QUrl("recent:/hello/world")) == QUrl("recent:/hello/world"));
}

#    if 0
TEST_F(RecentManagerTest, customColumnRole)
{
    stub.set_lamda(&RecentManager::init, []() {});
    QList<ItemRoles> list;
    EXPECT_TRUE(RecentManager::instance()->customColumnRole(QUrl("recent:/hello/world"), &list));
    EXPECT_FALSE(RecentManager::instance()->customColumnRole(QUrl("file:/hello/world"), &list));
}

TEST_F(RecentManagerTest, customRoleDisplayName)
{
    stub.set_lamda(&RecentManager::init, []() {});
    QString displayName;
    EXPECT_TRUE(RecentManager::instance()->customRoleDisplayName(QUrl("recent:/hello/world"), kItemFilePathRole, &displayName));
    EXPECT_TRUE(RecentManager::instance()->customRoleDisplayName(QUrl("recent:/hello/world"), kItemFileLastReadRole, &displayName));
    EXPECT_FALSE(RecentManager::instance()->customRoleDisplayName(QUrl("recent:/hello/world"), kItemNameRole, &displayName));
    EXPECT_FALSE(RecentManager::instance()->customRoleDisplayName(QUrl("file:/hello/world"), kItemFileLastReadRole, &displayName));
}

TEST_F(RecentManagerTest, detailViewIcon)
{
    stub.set_lamda(&RecentManager::init, []() {});
    QString iconName;
    EXPECT_TRUE(RecentManager::instance()->detailViewIcon(RecentHelper::rootUrl(), &iconName));
    EXPECT_FALSE(RecentManager::instance()->detailViewIcon(QUrl("recent:/hello/world"), &iconName));
}

TEST_F(RecentManagerTest, sepateTitlebarCrumb)
{
    stub.set_lamda(&RecentManager::init, []() {});
    QList<QVariantMap> mapGroup;
    EXPECT_TRUE(RecentManager::instance()->sepateTitlebarCrumb(RecentHelper::rootUrl(), &mapGroup));
    EXPECT_FALSE(RecentManager::instance()->sepateTitlebarCrumb(QUrl("file:/hello/world"), &mapGroup));
}

TEST_F(RecentManagerTest, isTransparent)
{
    stub.set_lamda(&RecentManager::init, []() {});
    TransparentStatus status;
    EXPECT_TRUE(RecentManager::instance()->isTransparent(RecentHelper::rootUrl(), &status));
    EXPECT_TRUE(status == TransparentStatus::kUntransparent);
    EXPECT_FALSE(RecentManager::instance()->isTransparent(QUrl("file:/hello/world"), &status));
}

TEST_F(RecentManagerTest, checkDragDropAction)
{
    stub.set_lamda(&RecentManager::init, []() {});
    Qt::DropAction action;
    EXPECT_TRUE(RecentManager::instance()->checkDragDropAction(QList<QUrl>() << RecentHelper::rootUrl(), QUrl("file:/hello/world"), &action));
    EXPECT_TRUE(action == Qt::CopyAction);
    EXPECT_FALSE(RecentManager::instance()->checkDragDropAction(QList<QUrl>(), QUrl("file:/hello/world"), &action));
    EXPECT_FALSE(RecentManager::instance()->checkDragDropAction(QList<QUrl>() << QUrl("file:/hello/world"), QUrl("file:/hello/world"), &action));
}

TEST_F(RecentManagerTest, handleDropFiles)
{
    stub.set_lamda(&RecentManager::init, []() {});
    stub.set_lamda(&RecentHelper::removeRecent, []() {});
    EXPECT_TRUE(RecentManager::instance()->handleDropFiles(QList<QUrl>() << RecentHelper::rootUrl(), QUrl("trash:/hello/world")));
    EXPECT_FALSE(RecentManager::instance()->handleDropFiles(QList<QUrl>(), QUrl("file:/hello/world")));
    EXPECT_FALSE(RecentManager::instance()->handleDropFiles(QList<QUrl>() << QUrl("recent:/hello/world"), QUrl("file:/hello/world")));
}
#    endif

TEST_F(RecentManagerTest, propetyExtensionFunc)
{
    stub.set_lamda(&RecentManager::init, []() {});
    EXPECT_TRUE(RecentHelper::propetyExtensionFunc(RecentHelper::rootUrl()).size() > 0);
}

TEST_F(RecentManagerTest, contenxtMenuHandle)
{
    stub.set_lamda(&RecentManager::init, []() {});
    QPoint q;
    bool isCall = false;
    stub.set_lamda((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), [&]() {
        isCall = true;
        return nullptr;
    });
    RecentHelper::contenxtMenuHandle(1, QUrl(), q);
    EXPECT_TRUE(isCall);
}

TEST_F(RecentManagerTest, removeRecent)
{
    bool isCall = false;
    stub.set_lamda(&RecentManager::init, []() {});
    stub.set_lamda(VADDR(DDialog, exec), [&isCall] {
        isCall = true;
        return 1;
    });
    RecentHelper::removeRecent(QList<QUrl>());
    EXPECT_TRUE(isCall);
}
#endif
