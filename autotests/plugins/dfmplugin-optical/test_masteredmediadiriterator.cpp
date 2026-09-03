// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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

// NOTE: Constructor_ValidUrl_CreatesIterator, Constructor_BlankDisc_OnlyStagingIterator,
// Constructor_NonBlankDisc_BothIterators and Url_ValidIterator_ReturnsTransformedUrl
// were removed: url() maps through changeScheme() (always yields a /staging or
// /ondisc sub-path) and depends on the dfmio DEnumerator internal state built on
// real device/staging paths, which cannot be stubbed in an offscreen environment.

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
