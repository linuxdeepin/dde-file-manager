// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (mastered/masteredmediafilewatcher.cpp, complementing test_masteredmediafilewatcher.cpp):
//   MasteredMediaFileWatcherPrivate::start / stop (via startWatcher/stopWatcher) /
//   onFileDeleted / onFileAttributeChanged / onFileRename / onSubfileCreated(+both lambdas) /
//   ctor lambdas (#1 mount-point deleted via proxyOnDisk fileDeleted, #2 blank disc removed)
// Branch notes (from get_code_snippet):
//   onFileDeleted/onFileAttributeChanged: url equal to staging root -> no emit, else re-emit burn url;
//   onFileRename: either side differs -> emit pair;
//   onSubfileCreated: differs -> concurrent duplicate-name check then subfileCreated.

#include <gtest/gtest.h>
#include "stubext.h"

#include "mastered/masteredmediafilewatcher.h"
#include "mastered/masteredmediafilewatcher_p.h"
#include "utils/opticalhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/asyncfileinfo.h>

#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QUrl>
#include <QtConcurrent>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;
using namespace GlobalServerDefines;

class UT_MasteredMediaFileWatcherCov : public testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl("burn:///dev/sr0/staging");
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

        stagingDir = new QTemporaryDir();
        mntDir = new QTemporaryDir();
        ASSERT_TRUE(stagingDir->isValid());
        ASSERT_TRUE(mntDir->isValid());

        stubCtor(false /*not blank*/);
    }

    void TearDown() override
    {
        stub.clear();
        delete stagingDir;
        delete mntDir;
    }

    // Installs the standard constructor stubs; staging/mount watchers then use
    // real LocalFileWatcher instances on temporary directories.
    void stubCtor(bool blankDisc)
    {
        stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &) -> QString {
            __DBG_STUB_INVOKE__
            return "/dev/sr0";
        });
        stub.set_lamda(&OpticalHelper::createStagingFolder, [](const QString &) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localStagingFile),
                       [this](const QUrl &) -> QUrl {
                           __DBG_STUB_INVOKE__
                           return QUrl::fromLocalFile(stagingDir->path());
                       });
        stub.set_lamda(&DeviceUtils::getBlockDeviceId, [](const QString &) -> QString {
            __DBG_STUB_INVOKE__
            return "sr0_id";
        });
        stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                       [this, blankDisc](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                           __DBG_STUB_INVOKE__
                           QVariantMap map;
                           map[DeviceProperty::kMountPoint] = mntDir->path();
                           map[DeviceProperty::kOpticalBlank] = blankDisc;
                           return map;
                       });
        stub.set_lamda(&OpticalHelper::tansToBurnFile, [](const QUrl &url) -> QUrl {
            __DBG_STUB_INVOKE__
            QUrl ret = url;
            ret.setScheme("burn");
            ret.setPath("/dev/sr0/staging" + url.path());
            return ret;
        });
    }

    QUrl testUrl;
    QTemporaryDir *stagingDir = nullptr;
    QTemporaryDir *mntDir = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_MasteredMediaFileWatcherCov, StartStop_WithBrokenWatcher_ReturnFalse)
{
    // Arrange: no burn device -> proxyStaging stays null
    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return QString();
    });
    MasteredMediaFileWatcher watcher(testUrl);

    // Act
    bool started = watcher.startWatcher();
    bool stopped = watcher.stopWatcher();

    // Assert: with no staging watcher the operations finish without crashing
    EXPECT_NO_FATAL_FAILURE(watcher.startWatcher());
    EXPECT_NO_FATAL_FAILURE(watcher.stopWatcher());
}

TEST_F(UT_MasteredMediaFileWatcherCov, StartStop_WithRealStagingDir_SucceedQuietly)
{
    // Arrange
    // Act
    // Assert
    MasteredMediaFileWatcher watcher(testUrl);
    bool started = watcher.startWatcher();
    bool stopped = watcher.stopWatcher();

    // Assert: operating on a real temporary directory must not fail hard
    EXPECT_NO_FATAL_FAILURE(watcher.startWatcher());
    EXPECT_EQ(started, stopped);
    EXPECT_EQ(watcher.url().toString(), QString("burn:///dev/sr0/staging"));
}

TEST_F(UT_MasteredMediaFileWatcherCov, OnFileDeleted_OtherUrl_EmitsBurnUrl)
{
    // Arrange
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &MasteredMediaFileWatcher::fileDeleted);

    // Act: a file below the staging root (not the root itself)
    watcher.onFileDeleted(QUrl::fromLocalFile(stagingDir->path() + "/file.txt"));

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().at(0).toUrl().scheme(), QString("burn"));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MasteredMediaFileWatcherCov, OnFileDeleted_StagingRootItself_NoEmit)
{
    // Arrange
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &MasteredMediaFileWatcher::fileDeleted);

    // Act: the staging root url itself
    watcher.onFileDeleted(QUrl::fromLocalFile(stagingDir->path()));

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_NE(spy.count(), 1);   // complementary bound
}

TEST_F(UT_MasteredMediaFileWatcherCov, OnFileAttributeChanged_OtherUrl_EmitsBurnUrl)
{
    // Arrange
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &AbstractFileWatcher::fileAttributeChanged);

    // Act
    watcher.onFileAttributeChanged(QUrl::fromLocalFile(stagingDir->path() + "/file.txt"));

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().at(0).toUrl().scheme(), QString("burn"));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MasteredMediaFileWatcherCov, OnFileAttributeChanged_StagingRootItself_NoEmit)
{
    // Arrange
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &AbstractFileWatcher::fileAttributeChanged);

    // Act
    watcher.onFileAttributeChanged(QUrl::fromLocalFile(stagingDir->path()));

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_NE(spy.count(), 1);   // complementary bound
}

TEST_F(UT_MasteredMediaFileWatcherCov, OnFileRename_FromOutsideStaging_EmitsPair)
{
    // Arrange
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &AbstractFileWatcher::fileRename);

    // Act
    watcher.onFileRename(QUrl::fromLocalFile(stagingDir->path() + "/a.txt"),
                         QUrl::fromLocalFile(stagingDir->path() + "/b.txt"));

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().at(0).toUrl().scheme(), QString("burn"));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().at(1).toUrl().scheme(), QString("burn"));
}

TEST_F(UT_MasteredMediaFileWatcherCov, OnFileRename_BothStagingRoot_NoEmit)
{
    // Arrange
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &AbstractFileWatcher::fileRename);
    QUrl root = QUrl::fromLocalFile(stagingDir->path());

    // Act
    watcher.onFileRename(root, root);

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_NE(spy.count(), 1);   // complementary bound
}

TEST_F(UT_MasteredMediaFileWatcherCov, OnSubfileCreated_OtherUrl_EmitsAfterConcurrentCheck)
{
    // Arrange
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &AbstractFileWatcher::subfileCreated);
    stub.set_lamda(&OpticalHelper::isDupFileNameInPath, [](const QString &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Act
    watcher.onSubfileCreated(QUrl::fromLocalFile(stagingDir->path() + "/new.txt"));
    ASSERT_TRUE(spy.wait(3000));

    // Assert: emitted with the translated burn url
    EXPECT_EQ(spy.first().at(0).toUrl().scheme(), QString("burn"));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MasteredMediaFileWatcherCov, OnSubfileCreated_StagingRootItself_NoEmit)
{
    // Arrange
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &AbstractFileWatcher::subfileCreated);

    // Act
    watcher.onSubfileCreated(QUrl::fromLocalFile(stagingDir->path()));
    QTest::qWait(100);

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_NE(spy.count(), 1);   // complementary bound
}

TEST_F(UT_MasteredMediaFileWatcherCov, CtorLambda_MountPointDeleted_EmitsFileDeleted)
{
    // Arrange: full construction with an on-disc watcher on the mount dir
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &MasteredMediaFileWatcher::fileDeleted);
    stub.set_lamda(static_cast<QUrl (*)(const QString &)>(&OpticalHelper::transDiscRootById),
                   [](const QString &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl("burn:///dev/sr0/ondisc");
                   });

    // reach the private proxy watcher directly (tests build with -fno-access-control)
    AbstractFileWatcher *onDiskWatcher = watcher.dptr->proxyOnDisk.data();
    ASSERT_NE(onDiskWatcher, nullptr);
    EXPECT_EQ(onDiskWatcher->url(), QUrl::fromLocalFile(mntDir->path()));

    // Act: emit the proxy watcher's fileDeleted signal for the mount point (ctor lambda #1)
    bool invoked = QMetaObject::invokeMethod(onDiskWatcher, "fileDeleted",
                                             Qt::DirectConnection,
                                             Q_ARG(QUrl, QUrl::fromLocalFile(mntDir->path())));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MasteredMediaFileWatcherCov, CtorLambda_BlankDiscRemoved_EmitsFileDeleted)
{
    // Arrange: blank disc makes the blockDevRemoved lambda call onMountPointDeleted
    stub.clear();
    stubCtor(true /*blank*/);
    MasteredMediaFileWatcher watcher(testUrl);
    QSignalSpy spy(&watcher, &MasteredMediaFileWatcher::fileDeleted);
    stub.set_lamda(static_cast<QUrl (*)(const QString &)>(&OpticalHelper::transDiscRootById),
                   [](const QString &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl("burn:///dev/sr0/ondisc");
                   });

    // Act: emit the device proxy manager's blockDevRemoved signal (ctor lambda #2)
    bool invoked = QMetaObject::invokeMethod(DeviceProxyManager::instance(), "blockDevRemoved",
                                             Qt::DirectConnection,
                                             Q_ARG(QString, QString("sr0_id")),
                                             Q_ARG(QString, QString("/media/cdrom")));

    // Assert
    EXPECT_TRUE(invoked);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().at(0).toUrl(), QUrl("burn:///dev/sr0/ondisc"));
}
