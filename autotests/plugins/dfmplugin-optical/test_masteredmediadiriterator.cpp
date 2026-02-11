// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "mastered/masteredmediadiriterator.h"
#include "utils/opticalhelper.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QUrl>
#include <QVariantMap>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;
using namespace GlobalServerDefines;

class TestMasteredMediaDirIterator : public testing::Test
{
public:
    void SetUp() override
    {
        testUrl = QUrl("burn:///dev/sr0/staging");
        rootUrl = QUrl("burn:///dev/sr0");
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    QUrl testUrl;
    QUrl rootUrl;
    stub_ext::StubExt stub;
};

TEST_F(TestMasteredMediaDirIterator, Constructor_ValidUrl_CreatesIterator)
{
    QStringList nameFilters;
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    
    MasteredMediaDirIterator iterator(testUrl, nameFilters, filters, flags);
    EXPECT_EQ(iterator.url(), testUrl);
}

TEST_F(TestMasteredMediaDirIterator, Constructor_BlankDisc_OnlyStagingIterator)
{
    bool queryBlockInfoCalled = false;

    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [&](DeviceProxyManager *obj, const QString &id, bool reload) {
        __DBG_STUB_INVOKE__
        queryBlockInfoCalled = true;
        QVariantMap map;
        map[DeviceProperty::kMountPoint] = QString("/media/cdrom");
        map[DeviceProperty::kOpticalBlank] = true;
        return map;
    });

    QStringList nameFilters;
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    
    MasteredMediaDirIterator iterator(rootUrl, nameFilters, filters, flags);

    EXPECT_TRUE(queryBlockInfoCalled);
    EXPECT_EQ(iterator.url(), rootUrl);
}

TEST_F(TestMasteredMediaDirIterator, Constructor_NonBlankDisc_BothIterators)
{
    bool queryBlockInfoCalled = false;

    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [&](DeviceProxyManager *obj, const QString &id, bool reload) {
        __DBG_STUB_INVOKE__
        queryBlockInfoCalled = true;
        QVariantMap map;
        map[DeviceProperty::kMountPoint] = QString("/media/cdrom");
        map[DeviceProperty::kOpticalBlank] = false;
        return map;
    });

    QStringList nameFilters;
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    
    MasteredMediaDirIterator iterator(rootUrl, nameFilters, filters, flags);

    EXPECT_TRUE(queryBlockInfoCalled);
    EXPECT_EQ(iterator.url(), rootUrl);
}

TEST_F(TestMasteredMediaDirIterator, FileName_ValidCurrentUrl_ReturnsFileName)
{
    stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localStagingFile), [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/staging/test");
    });

    QStringList nameFilters;
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    
    MasteredMediaDirIterator iterator(testUrl, nameFilters, filters, flags);
    QString result = iterator.fileName();

    // Basic functionality test - should return a valid filename
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestMasteredMediaDirIterator, Url_ValidIterator_ReturnsTransformedUrl)
{
    stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localStagingFile), [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/staging/test");
    });

    QStringList nameFilters;
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    
    MasteredMediaDirIterator iterator(testUrl, nameFilters, filters, flags);
    QUrl result = iterator.url();

    EXPECT_EQ(result, testUrl);
}
