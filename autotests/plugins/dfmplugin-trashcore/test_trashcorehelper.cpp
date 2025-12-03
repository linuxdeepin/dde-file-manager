// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>

#include "dfmplugin_trashcore_global.h"
#include "stubext.h"
#include "utils/trashcorehelper.h"
#include "views/trashpropertydialog.h"
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-io/denumerator.h>

using namespace dfmplugin_trashcore;

class TrashCoreHelperTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TrashCoreHelperTest, Scheme_Basic)
{
    EXPECT_EQ(TrashCoreHelper::scheme(), "trash");
}

TEST_F(TrashCoreHelperTest, Icon_Basic)
{
    QIcon icon = TrashCoreHelper::icon();
    EXPECT_FALSE(icon.isNull());
}

TEST_F(TrashCoreHelperTest, RootUrl_Basic)
{
    QUrl expected;
    expected.setScheme("trash");
    expected.setPath("/");

    EXPECT_EQ(TrashCoreHelper::rootUrl(), expected);
}

TEST_F(TrashCoreHelperTest, CalculateTrashRoot_Basic)
{
    // Just ensure the function doesn't crash, since it involves complex DFMIO operations
    EXPECT_NO_THROW(TrashCoreHelper::calculateTrashRoot());
}

TEST_F(TrashCoreHelperTest, CreateTrashPropertyDialog_RootUrl)
{
    QUrl rootUrl("trash:///");

    // Mock FileUtils::trashRootUrl
    stub.set_lamda(ADDR(DFMBASE_NAMESPACE::FileUtils, trashRootUrl), []() {
        return QUrl("trash:///");
    });

    stub.set_lamda(ADDR(DFMBASE_NAMESPACE::UniversalUtils, urlEquals), [](const QUrl &url1, const QUrl &url2) {
        return url1 == url2 && url1.toString() == "trash:///";
    });

    // Mock FileUtils::isTrashDesktopFile
    stub.set_lamda(ADDR(DFMBASE_NAMESPACE::FileUtils, isTrashDesktopFile), [](const QUrl &url) {
        return url.toString() == "trash:///";  // Only for root URL
    });

    EXPECT_NO_THROW(TrashCoreHelper::createTrashPropertyDialog(rootUrl));
}

TEST_F(TrashCoreHelperTest, CreateTrashPropertyDialog_NotTrashRoot)
{
    QUrl nonRootUrl("trash:///somefile.txt");

    // Mock FileUtils::trashRootUrl
    stub.set_lamda(ADDR(DFMBASE_NAMESPACE::FileUtils, trashRootUrl), []() {
        return QUrl("trash:///");
    });

    // Mock UniversalUtils::urlEquals
    stub.set_lamda(ADDR(DFMBASE_NAMESPACE::UniversalUtils, urlEquals), [](const QUrl &url1, const QUrl &url2) {
        return url1 == url2 && url1.toString() == "trash:///";
    });

    // Mock FileUtils::isTrashDesktopFile
    stub.set_lamda(ADDR(DFMBASE_NAMESPACE::FileUtils, isTrashDesktopFile), [](const QUrl &url) {
        return url.toString() == "trash:///";  // Only root URL returns true
    });

    QWidget *result = TrashCoreHelper::createTrashPropertyDialog(nonRootUrl);
    EXPECT_EQ(result, nullptr);
}
