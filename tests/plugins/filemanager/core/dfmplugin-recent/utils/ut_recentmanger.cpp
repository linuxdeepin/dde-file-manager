// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/recentmanager.h"
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <gtest/gtest.h>

#include <QMimeData>

using namespace testing;
using namespace dfmplugin_recent;

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

TEST_F(RecentManagerTest, scheme)
{
    DFMBASE_USE_NAMESPACE

    EXPECT_TRUE(RecentHelper::scheme() == "recent");
}

TEST_F(RecentManagerTest, icon)
{
    DFMBASE_USE_NAMESPACE

    EXPECT_TRUE(RecentHelper::icon().name() == "document-open-recent-symbolic");
}

TEST_F(RecentManagerTest, xbelPath)
{
    DFMBASE_USE_NAMESPACE

    EXPECT_TRUE(RecentHelper::xbelPath() == QDir::homePath() + "/.local/share/recently-used.xbel");
}

TEST_F(RecentManagerTest, rootUrl)
{
    DFMBASE_USE_NAMESPACE

    QUrl url;
    url.setScheme(RecentHelper::scheme());
    url.setPath("/");
    EXPECT_TRUE(RecentHelper::rootUrl() == url);
}

TEST_F(RecentManagerTest, openFileLocation)
{
    DFMBASE_USE_NAMESPACE

    EXPECT_TRUE(RecentHelper::xbelPath() == QDir::homePath() + "/.local/share/recently-used.xbel");
}

TEST_F(RecentManagerTest, urlTransform)
{
    DFMBASE_USE_NAMESPACE
    stub.set_lamda(&UrlRoute::isRootUrl, []() -> bool {
        return false;
    });
    EXPECT_TRUE(RecentHelper::urlTransform(QUrl("recent:/hello/world")) == QUrl("file:/hello/world"));
    stub.set_lamda(&UrlRoute::isRootUrl, []() -> bool {
        return true;
    });
    EXPECT_TRUE(RecentHelper::urlTransform(QUrl("recent:/hello/world")) == QUrl("recent:/hello/world"));
}

TEST_F(RecentManagerTest, customColumnRole)
{
    DFMBASE_USE_NAMESPACE
    DFMGLOBAL_USE_NAMESPACE
    stub.set_lamda(&RecentManager::init, []() {});
    QList<ItemRoles> list;
    EXPECT_TRUE(RecentManager::instance()->customColumnRole(QUrl("recent:/hello/world"), &list));
    EXPECT_FALSE(RecentManager::instance()->customColumnRole(QUrl("file:/hello/world"), &list));
}

TEST_F(RecentManagerTest, customRoleDisplayName)
{
    DFMBASE_USE_NAMESPACE
    DFMGLOBAL_USE_NAMESPACE
    stub.set_lamda(&RecentManager::init, []() {});
    QString displayName;
    EXPECT_TRUE(RecentManager::instance()->customRoleDisplayName(QUrl("recent:/hello/world"), kItemFilePathRole, &displayName));
    EXPECT_TRUE(RecentManager::instance()->customRoleDisplayName(QUrl("recent:/hello/world"), kItemFileLastReadRole, &displayName));
    EXPECT_FALSE(RecentManager::instance()->customRoleDisplayName(QUrl("recent:/hello/world"), kItemNameRole, &displayName));
    EXPECT_FALSE(RecentManager::instance()->customRoleDisplayName(QUrl("file:/hello/world"), kItemFileLastReadRole, &displayName));
}

TEST_F(RecentManagerTest, detailViewIcon)
{
    DFMBASE_USE_NAMESPACE
    DFMGLOBAL_USE_NAMESPACE
    stub.set_lamda(&RecentManager::init, []() {});
    QString iconName;
    EXPECT_TRUE(RecentManager::instance()->detailViewIcon(RecentHelper::rootUrl(), &iconName));
    EXPECT_FALSE(RecentManager::instance()->detailViewIcon(QUrl("recent:/hello/world"), &iconName));
}

TEST_F(RecentManagerTest, sepateTitlebarCrumb)
{
    DFMBASE_USE_NAMESPACE
    DFMGLOBAL_USE_NAMESPACE
    stub.set_lamda(&RecentManager::init, []() {});
    QList<QVariantMap> mapGroup;
    EXPECT_TRUE(RecentManager::instance()->sepateTitlebarCrumb(RecentHelper::rootUrl(), &mapGroup));
    EXPECT_FALSE(RecentManager::instance()->sepateTitlebarCrumb(QUrl("file:/hello/world"), &mapGroup));
}

TEST_F(RecentManagerTest, isTransparent)
{
    DFMBASE_USE_NAMESPACE
    DFMGLOBAL_USE_NAMESPACE
    stub.set_lamda(&RecentManager::init, []() {});
    TransparentStatus status;
    EXPECT_TRUE(RecentManager::instance()->isTransparent(RecentHelper::rootUrl(), &status));
    EXPECT_TRUE(status == TransparentStatus::kUntransparent);
    EXPECT_FALSE(RecentManager::instance()->isTransparent(QUrl("file:/hello/world"), &status));
}

TEST_F(RecentManagerTest, checkDragDropAction)
{
    DFMBASE_USE_NAMESPACE
    DFMGLOBAL_USE_NAMESPACE
    stub.set_lamda(&RecentManager::init, []() {});
    Qt::DropAction action;
    EXPECT_TRUE(RecentManager::instance()->checkDragDropAction(QList<QUrl>() << RecentHelper::rootUrl(), QUrl("file:/hello/world"), &action));
    EXPECT_TRUE(action == Qt::CopyAction);
    EXPECT_FALSE(RecentManager::instance()->checkDragDropAction(QList<QUrl>(), QUrl("file:/hello/world"), &action));
    EXPECT_FALSE(RecentManager::instance()->checkDragDropAction(QList<QUrl>() << QUrl("file:/hello/world"), QUrl("file:/hello/world"), &action));
}

TEST_F(RecentManagerTest, handleDropFiles)
{
    DFMBASE_USE_NAMESPACE
    DFMGLOBAL_USE_NAMESPACE
    stub.set_lamda(&RecentManager::init, []() {});
    stub.set_lamda(&RecentHelper::removeRecent, []() {});
    EXPECT_TRUE(RecentManager::instance()->handleDropFiles(QList<QUrl>() << RecentHelper::rootUrl(), QUrl("trash:/hello/world")));
    EXPECT_FALSE(RecentManager::instance()->handleDropFiles(QList<QUrl>(), QUrl("file:/hello/world")));
    EXPECT_FALSE(RecentManager::instance()->handleDropFiles(QList<QUrl>() << QUrl("recent:/hello/world"), QUrl("file:/hello/world")));
}
