// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/openwith/openwithdialog.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-framework/dpf.h>

#include <QCheckBox>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_OpenWithDialog : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(ADDR(MimesAppsManager, initMimeTypeApps),
                       [] {
                           __DBG_STUB_INVOKE__
                       });

        stub.set_lamda(qOverload<const QMimeType &>(&MimesAppsManager::getDefaultAppByMimeType),
                       [] {
                           __DBG_STUB_INVOKE__
                           return QString();
                       });

        stub.set_lamda(ADDR(MimesAppsManager, getRecommendedApps),
                       [] {
                           __DBG_STUB_INVOKE__
                           return QStringList();
                       });

        stub.set_lamda(ADDR(MimesAppsManager, getRecommendedAppsByQio),
                       [] {
                           __DBG_STUB_INVOKE__
                           return QStringList();
                       });

        stub.set_lamda(&InfoFactory::create<FileInfo>,
                       [](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                           __DBG_STUB_INVOKE__
                           return nullptr;
                       });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_OpenWithDialog, Constructor_WithUrlList_CreatesDialog)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };

    OpenWithDialog *dialog = new OpenWithDialog(urls);

    EXPECT_NE(dialog, nullptr);
    delete dialog;
}

TEST_F(UT_OpenWithDialog, Constructor_WithSingleUrl_CreatesDialog)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");

    OpenWithDialog *dialog = new OpenWithDialog(url);

    EXPECT_NE(dialog, nullptr);
    delete dialog;
}

TEST_F(UT_OpenWithDialog, openFileByApp_NoCheckedItem_Returns)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };

    OpenWithDialog *dialog = new OpenWithDialog(urls);

    dialog->openFileByApp();

    delete dialog;
}

// ========== OpenWithDialogListItem 测试 ==========

class UT_OpenWithDialogListItem : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_OpenWithDialogListItem, Constructor_CreatesItem)
{
    OpenWithDialogListItem *item = new OpenWithDialogListItem("application-x-desktop", "Test App");

    EXPECT_NE(item, nullptr);
    EXPECT_EQ(item->text(), "Test App");

    delete item;
}

TEST_F(UT_OpenWithDialogListItem, setChecked_UpdatesCheckState)
{
    OpenWithDialogListItem *item = new OpenWithDialogListItem("application-x-desktop", "Test App");

    item->setChecked(true);
    item->setChecked(false);

    delete item;
}

TEST_F(UT_OpenWithDialogListItem, text_ReturnsLabelText)
{
    OpenWithDialogListItem *item = new OpenWithDialogListItem("icon", "My Application");

    EXPECT_EQ(item->text(), "My Application");

    delete item;
}
