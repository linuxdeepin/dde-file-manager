// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (mastered/masteredmediadiriterator.cpp, complementing test_masteredmediadiriterator.cpp):
//   next / hasNext / url (fileInfo is skipped: it asserts a non-GUI thread)
// Branch notes (from get_code_snippet):
//   hasNext: disc iterator first, falls back to the staging iterator;
//   next: takes from disc iterator if present else from staging iterator and
//   maps the url through changeScheme (burn scheme).

#include <gtest/gtest.h>
#include "stubext.h"

#include "mastered/masteredmediadiriterator.h"
#include "utils/opticalhelper.h"

#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QTemporaryDir>
#include <QUrl>
#include <QVariantMap>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;
using namespace GlobalServerDefines;

class UT_MasteredMediaDirIteratorCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stagingDir = new QTemporaryDir();
        ASSERT_TRUE(stagingDir->isValid());
        // a real file inside the staging dir so the enumerator has something
        QFile f(stagingDir->path() + "/hello.txt");
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("hi");
        f.close();

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
        // blank disc: only the staging iterator is created
        stub.set_lamda(&DeviceUtils::getBlockDeviceId, [](const QString &) -> QString {
            __DBG_STUB_INVOKE__
            return "sr0_id";
        });
        stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                       [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                           __DBG_STUB_INVOKE__
                           QVariantMap map;
                           map[DeviceProperty::kMountPoint] = QString("");
                           map[DeviceProperty::kOpticalBlank] = true;
                           return map;
                       });
    }

    void TearDown() override
    {
        stub.clear();
        delete stagingDir;
    }

    MasteredMediaDirIterator makeIterator()
    {
        return MasteredMediaDirIterator(QUrl("burn:///dev/sr0/staging"),
                                        QStringList(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                                        QDirIterator::NoIteratorFlags);
    }

    QTemporaryDir *stagingDir = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_MasteredMediaDirIteratorCov, HasNext_StagingWithFile_ReturnsTrueThenFalse)
{
    // Arrange
    auto it = makeIterator();

    // Act

    // Assert
    EXPECT_EQ(it.hasNext(), true);
    ASSERT_FALSE(it.next().isEmpty());
    EXPECT_EQ(it.hasNext(), false);
}

TEST_F(UT_MasteredMediaDirIteratorCov, Next_ReturnsBurnSchemeUrl)
{
    // Arrange
    auto it = makeIterator();

    // Act: hasNext advances the underlying enumerator first
    ASSERT_TRUE(it.hasNext());
    QUrl url = it.next();

    // Assert: changeScheme turned the staging path into a burn url
    EXPECT_EQ(url.scheme(), QString("burn"));
    EXPECT_FALSE(url.path().isEmpty());
}

TEST_F(UT_MasteredMediaDirIteratorCov, Url_MapsStagingUriToBurnScheme)
{
    // Arrange
    auto it = makeIterator();

    // Act
    QUrl url = it.url();

    // Assert
    EXPECT_EQ(url.scheme(), QString("burn"));
    EXPECT_FALSE(url.path().isEmpty());
}

TEST_F(UT_MasteredMediaDirIteratorCov, EmptyStaging_HasNextFalse)
{
    // Arrange: staging dir without entries
    QTemporaryDir empty;
    ASSERT_TRUE(empty.isValid());
    stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localStagingFile),
                   [&empty](const QUrl &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl::fromLocalFile(empty.path());
                   });

    // Act

    // Assert
    auto it = makeIterator();
    EXPECT_EQ(it.hasNext(), false);
    EXPECT_NE(it.hasNext(), true);   // double-check stable state
    EXPECT_NO_FATAL_FAILURE(it.next());
}
