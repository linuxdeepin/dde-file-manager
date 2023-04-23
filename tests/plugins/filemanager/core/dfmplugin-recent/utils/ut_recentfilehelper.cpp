// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/recentfilehelper.h"
#include "utils/recentmanager.h"
#include "events/recenteventcaller.h"
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-io/dfmio_utils.h>

#include <gtest/gtest.h>

#include <QMimeData>

using namespace testing;

using namespace dfmplugin_recent;

class RecentFileHelperTest : public Test
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

TEST_F(RecentFileHelperTest, setPermissionHandle)
{
    DFMBASE_USE_NAMESPACE

    RecentFileHelper *helper = RecentFileHelper::instance();
    bool ok;
    QString error;
    stub.set_lamda(&LocalFileHandler::setPermissions, []() -> bool {
        return false;
    });
    stub.set_lamda(&LocalFileHandler::errorString, []() -> QString {
        return QString("error");
    });

    EXPECT_TRUE(helper->setPermissionHandle(quint64(111), QUrl("recent:/hello/world"), QFileDevice::ReadUser, &ok, &error));
    EXPECT_FALSE(helper->setPermissionHandle(quint64(111), QUrl("file:/hello/world"), QFileDevice::ReadUser, &ok, &error));
}

TEST_F(RecentFileHelperTest, cutFile)
{
    DFMBASE_USE_NAMESPACE

    RecentFileHelper *helper = RecentFileHelper::instance();

    EXPECT_TRUE(helper->cutFile(quint64(111), QList<QUrl>() << QUrl("recent:/hello/world"), QUrl("recent:/hello/world"), AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_FALSE(helper->cutFile(quint64(111), QList<QUrl>() << QUrl("file:/hello/world"), QUrl("file:/hello/world"), AbstractJobHandler::JobFlag::kNoHint));
}

TEST_F(RecentFileHelperTest, copyFile)
{
    DFMBASE_USE_NAMESPACE

    RecentFileHelper *helper = RecentFileHelper::instance();

    EXPECT_TRUE(helper->copyFile(quint64(111), QList<QUrl>() << QUrl("recent:/hello/world"), QUrl("recent:/hello/world"), AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_FALSE(helper->copyFile(quint64(111), QList<QUrl>() << QUrl("file:/hello/world"), QUrl("file:/hello/world"), AbstractJobHandler::JobFlag::kNoHint));
}

TEST_F(RecentFileHelperTest, moveToTrash)
{
    DFMBASE_USE_NAMESPACE

    RecentFileHelper *helper = RecentFileHelper::instance();

    stub.set_lamda(&RecentHelper::removeRecent, []() {});

    EXPECT_TRUE(helper->moveToTrash(quint64(111), QList<QUrl>() << QUrl("recent:/hello/world"), AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_FALSE(helper->moveToTrash(quint64(111), QList<QUrl>() << QUrl("file:/hello/world"), AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_FALSE(helper->moveToTrash(quint64(111), QList<QUrl>(), AbstractJobHandler::JobFlag::kNoHint));
}

TEST_F(RecentFileHelperTest, openFileInPlugin)
{
    DFMBASE_USE_NAMESPACE

    RecentFileHelper *helper = RecentFileHelper::instance();

    stub.set_lamda(&RecentEventCaller::sendOpenFiles, []() {});

    EXPECT_TRUE(helper->openFileInPlugin(quint64(111), QList<QUrl>() << QUrl("recent:/hello/world")));
    EXPECT_FALSE(helper->openFileInPlugin(quint64(111), QList<QUrl>() << QUrl("file:/hello/world")));
    EXPECT_FALSE(helper->openFileInPlugin(quint64(111), QList<QUrl>()));
}

TEST_F(RecentFileHelperTest, linkFile)
{
    DFMBASE_USE_NAMESPACE

    RecentFileHelper *helper = RecentFileHelper::instance();
    bool ok = true;
    stub.set_lamda(&LocalFileHandler::deleteFile, []() -> bool {
        return true;
    });
    stub.set_lamda(&FileUtils::nonExistSymlinkFileName, []() -> QString {
        return QString("recent:/hello/world");
    });
    stub.set_lamda(&RecentHelper::urlTransform, []() -> QUrl {
        return QUrl("recent:/hello/world");
    });
    stub.set_lamda(&LocalFileHandler::createSystemLink, []() -> bool {
        return true;
    });
    EXPECT_TRUE(helper->linkFile(quint64(111), QUrl("recent:/hello/world"), QUrl("file:/hello/world"), ok, ok));
    EXPECT_FALSE(helper->linkFile(quint64(111), QUrl("file:/hello/world"), QUrl("recent:/hello/world"), ok, ok));
}

TEST_F(RecentFileHelperTest, writeUrlsToClipboard)
{
    DFMBASE_USE_NAMESPACE

    RecentFileHelper *helper = RecentFileHelper::instance();

    stub.set_lamda(&RecentEventCaller::sendWriteToClipboard, []() {});

    EXPECT_TRUE(helper->writeUrlsToClipboard(quint64(111), ClipBoard::ClipboardAction::kCutAction, QList<QUrl>() << QUrl("recent:/hello/world")));
    EXPECT_TRUE(helper->writeUrlsToClipboard(quint64(111), ClipBoard::ClipboardAction::kCopyAction, QList<QUrl>() << QUrl("recent:/hello/world")));
    EXPECT_FALSE(helper->writeUrlsToClipboard(quint64(111), ClipBoard::ClipboardAction::kCutAction, QList<QUrl>() << QUrl("file:/hello/world")));
    EXPECT_FALSE(helper->writeUrlsToClipboard(quint64(111), ClipBoard::ClipboardAction::kCutAction, QList<QUrl>()));
}

TEST_F(RecentFileHelperTest, openFileInTerminal)
{
    DFMBASE_USE_NAMESPACE

    RecentFileHelper *helper = RecentFileHelper::instance();

    EXPECT_TRUE(helper->openFileInTerminal(quint64(111), QList<QUrl>() << QUrl("recent:/hello/world")));
    EXPECT_FALSE(helper->openFileInTerminal(quint64(111), QList<QUrl>() << QUrl("file:/hello/world")));
    EXPECT_FALSE(helper->openFileInTerminal(quint64(111), QList<QUrl>()));
}
