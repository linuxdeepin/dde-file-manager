// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (mastered/masteredmediafileinfo.cpp, complementing test_masteredmediafileinfo.cpp):
//   exists / nameOf(kFileCopyName) / extraProperties / refresh / updateAttributes /
//   canAttributes(kCanRename/kCanRedirectionFileUrl/kCanDrop/kCanHidden) / canDrop
// Branch notes (from get_code_snippet):
//   exists: empty url / invalid backerUrl compared with disc & staging roots / proxy->exists;
//   nameOf: kFileCopyName -> displayOf(kFileDisplayName) -> idLabel or suitable display name;
//   canDrop: staging backer -> true; disc backer with free size >0 / ==0.

#include <gtest/gtest.h>
#include "stubext.h"

#include "mastered/masteredmediafileinfo.h"
#include "utils/opticalhelper.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QTemporaryDir>
#include <QUrl>
#include <QVariantMap>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;
using namespace GlobalServerDefines;

class UT_MasteredMediaFileInfoCov : public testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        stagingDir = new QTemporaryDir();
        ASSERT_TRUE(stagingDir->isValid());

        // default: burn urls resolve to the local staging area
        stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &) -> QString {
            __DBG_STUB_INVOKE__
            return "/dev/sr0";
        });
        stub.set_lamda(&OpticalHelper::burnIsOnDisc, [](const QUrl &) -> bool {
            __DBG_STUB_INVOKE__
            return false;   // staging side
        });
        stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localStagingFile),
                       [this](const QUrl &url) -> QUrl {
                           __DBG_STUB_INVOKE__
                           return QUrl::fromLocalFile(stagingDir->path() + url.path());
                       });
    }

    void TearDown() override
    {
        stub.clear();
        delete stagingDir;
    }

    QTemporaryDir *stagingDir = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_MasteredMediaFileInfoCov, Exists_DiscRootUrl_ReturnsTrue)
{
    // Arrange: staging backer is invalid so exists() compares with the roots
    stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localStagingFile),
                   [](const QUrl &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl();   // invalid backer
                   });
    stub.set_lamda(static_cast<QUrl (*)(const QString &)>(&OpticalHelper::discRoot),
                   [](const QString &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl("burn:///dev/sr0");
                   });

    // Act
    MasteredMediaFileInfo rootInfo(QUrl("burn:///dev/sr0"));
    MasteredMediaFileInfo other(QUrl("burn:///dev/sr0/whatever"));

    // Assert
    EXPECT_EQ(rootInfo.exists(), true);
    EXPECT_EQ(other.exists(), false);
}

TEST_F(UT_MasteredMediaFileInfoCov, Exists_StagingRootUrl_ReturnsTrue)
{
    // Arrange
    stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localStagingFile),
                   [](const QUrl &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl();   // invalid backer
                   });
    stub.set_lamda(static_cast<QUrl (*)(const QString &)>(&OpticalHelper::discRoot),
                   [](const QString &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl();   // not the disc root
                   });
    stub.set_lamda(static_cast<QUrl (*)()>(&OpticalHelper::localStagingRoot),
                   []() -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl("burn:///dev/sr0/staging");
                   });

    // Act
    MasteredMediaFileInfo stagingRoot(QUrl("burn:///dev/sr0/staging"));

    // Assert
    EXPECT_EQ(stagingRoot.exists(), true);
    EXPECT_NE(stagingRoot.exists(), false);   // double-check stable state
}

TEST_F(UT_MasteredMediaFileInfoCov, Exists_WithProxy_DelegatesToProxy)
{
    // Arrange: proxy points to a real existing temp file
    QFile f(stagingDir->path() + "/real.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();
    FileInfoPointer proxy(new SyncFileInfo(QUrl::fromLocalFile(stagingDir->path() + "/real.txt")));
    MasteredMediaFileInfo info(QUrl("burn:///dev/sr0/staging/real.txt"), proxy);

    // Act

    // Assert
    EXPECT_EQ(info.exists(), true);

    MasteredMediaFileInfo missing(QUrl("burn:///dev/sr0/staging/gone.txt"),
                                  FileInfoPointer(new SyncFileInfo(QUrl::fromLocalFile(stagingDir->path() + "/gone.txt"))));
    EXPECT_EQ(missing.exists(), false);
}

TEST_F(UT_MasteredMediaFileInfoCov, NameOf_CopyName_RootUsesDiscLabel)
{
    // Arrange: root path with an id label from the device database
    QVariantMap blockInfo;
    blockInfo[DeviceProperty::kIdLabel] = "MyDisc";
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [blockInfo](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       return blockInfo;
                   });
    stub.set_lamda(&OpticalHelper::burnFilePath, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "/";
    });
    stub.set_lamda(static_cast<QString (*)(const QVariantMap &)>(&DeviceUtils::convertSuitableDisplayName),
                   [](const QVariantMap &) -> QString {
                       __DBG_STUB_INVOKE__
                       return "fallback";
                   });

    // Act
    MasteredMediaFileInfo rootInfo(QUrl("burn:///dev/sr0"));
    QString name = rootInfo.nameOf(FileInfo::FileNameInfoType::kFileCopyName);

    // Assert
    EXPECT_EQ(name, QString("MyDisc"));
    EXPECT_EQ(name.length(), 6);
}

TEST_F(UT_MasteredMediaFileInfoCov, NameOf_CopyName_EmptyLabelUsesSuitableName)
{
    // Arrange: no id label -> suitable display name fallback
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       return QVariantMap();
                   });
    stub.set_lamda(&OpticalHelper::burnFilePath, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "/";
    });
    stub.set_lamda(static_cast<QString (*)(const QVariantMap &)>(&DeviceUtils::convertSuitableDisplayName),
                   [](const QVariantMap &) -> QString {
                       __DBG_STUB_INVOKE__
                       return "sr0-suitable";
                   });

    // Act
    MasteredMediaFileInfo rootInfo(QUrl("burn:///dev/sr0"));
    QString name = rootInfo.nameOf(FileInfo::FileNameInfoType::kFileCopyName);

    // Assert
    EXPECT_EQ(name, QString("sr0-suitable"));
    EXPECT_EQ(name.length(), 12);
}

TEST_F(UT_MasteredMediaFileInfoCov, ExtraProperties_ContainsBackerPath)
{
    // Arrange
    MasteredMediaFileInfo info(QUrl("burn:///dev/sr0/staging/test.txt"));

    // Act
    QVariantHash extra = info.extraProperties();

    // Assert
    EXPECT_TRUE(extra.contains("mm_backer"));
    // localStagingFile() maps burn:///dev/sr0/staging/test.txt onto the staging root
    EXPECT_EQ(extra.value("mm_backer").toString(),
              QString(stagingDir->path() + "/dev/sr0/staging/test.txt"));
}

TEST_F(UT_MasteredMediaFileInfoCov, Refresh_RebacksInfoAndKeepsProxy)
{
    // Arrange: the constructor already installs a proxy through the factory
    MasteredMediaFileInfo info(QUrl("burn:///dev/sr0/staging/test.txt"));
    ASSERT_TRUE(info.canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl));

    // Act
    info.refresh();

    // Assert: refresh keeps the info usable (proxy path)
    EXPECT_EQ(info.canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl), true);
    EXPECT_EQ(info.extraProperties().value("mm_backer").toString().isEmpty(), false);
}

TEST_F(UT_MasteredMediaFileInfoCov, UpdateAttributes_WithProxy_EarlyReturn)
{
    // Arrange: give the info a valid proxy first
    MasteredMediaFileInfo info(QUrl("burn:///dev/sr0/staging/test.txt"));
    info.refresh();
    ASSERT_TRUE(info.canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl));

    // Act: proxy present -> pure delegation, no re-backing

    // Assert
    EXPECT_NO_FATAL_FAILURE(info.updateAttributes({ FileInfo::FileInfoAttributeID::kStandardFileExists }));
    EXPECT_EQ(info.canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl), true);
    EXPECT_NE(info.canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl), false);   // double-check stable state
}

TEST_F(UT_MasteredMediaFileInfoCov, CanAttributes_AllBranches)
{
    // Arrange
    QFile pf(stagingDir->path() + "/proxied.txt");
    ASSERT_TRUE(pf.open(QIODevice::WriteOnly));
    pf.close();
    MasteredMediaFileInfo stagingInfo(QUrl("burn:///dev/sr0/staging/test.txt"));
    FileInfoPointer proxy(new SyncFileInfo(QUrl::fromLocalFile(stagingDir->path() + "/proxied.txt")));
    MasteredMediaFileInfo proxied(QUrl("burn:///dev/sr0/staging/proxied.txt"), proxy);

    // Act: rename needs a proxy

    // Assert
    EXPECT_EQ(stagingInfo.canAttributes(FileInfo::FileCanType::kCanRename), false);
    EXPECT_EQ(proxied.canAttributes(FileInfo::FileCanType::kCanRename), true);

    // redirection reflects proxy presence
    EXPECT_EQ(proxied.canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl), true);

    // hidden never allowed
    EXPECT_EQ(proxied.canAttributes(FileInfo::FileCanType::kCanHidden), false);
    EXPECT_EQ(proxied.canAttributes(FileInfo::FileCanType::kCanDragCompress), false);
}

TEST_F(UT_MasteredMediaFileInfoCov, CanDrop_StagingBacker_AlwaysTrue)
{
    // Arrange: burnIsOnDisc(backer) == false (default stub)
    MasteredMediaFileInfo info(QUrl("burn:///dev/sr0/staging/test.txt"));

    // Act

    // Assert
    EXPECT_EQ(info.canAttributes(FileInfo::FileCanType::kCanDrop), true);
    EXPECT_NE(info.canAttributes(FileInfo::FileCanType::kCanDrop), false);   // double-check stable state
}

TEST_F(UT_MasteredMediaFileInfoCov, CanDrop_DiscBacker_RespectsFreeSize)
{
    // Arrange: on-disc backer with free space
    QVariantMap blockInfo;
    blockInfo[DeviceProperty::kSizeFree] = QVariant::fromValue(static_cast<quint64>(1024));
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [blockInfo](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       return blockInfo;
                   });
    stub.set_lamda(&OpticalHelper::burnIsOnDisc, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    MasteredMediaFileInfo info(QUrl("burn:///dev/sr0/ondisc/test.txt"));

    // Act

    // Assert
    EXPECT_EQ(info.canAttributes(FileInfo::FileCanType::kCanDrop), true);

    // zero free space -> cannot drop
    QVariantMap empty;
    empty[DeviceProperty::kSizeFree] = QVariant::fromValue(static_cast<quint64>(0));
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [empty](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       return empty;
                   });
    MasteredMediaFileInfo full(QUrl("burn:///dev/sr0/ondisc/test.txt"));
    EXPECT_EQ(full.canAttributes(FileInfo::FileCanType::kCanDrop), false);
}
