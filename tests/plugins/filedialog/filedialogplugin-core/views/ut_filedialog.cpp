// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filedialog/filedialogplugin-core/views/filedialog.h"
#include "plugins/filedialog/filedialogplugin-core/views/filedialog_p.h"
#include "plugins/filedialog/filedialogplugin-core/events/coreeventscaller.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

DIALOGCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_FileDialog : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override { stub.clear(); }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_FileDialog, bug_133235_EnterHandle)
{
    FileDialog dialog(QUrl("~"));
    dialog.d->isFileView = true;
    QSharedPointer<SyncFileInfo> info(new SyncFileInfo(QUrl::fromLocalFile("/home")));
    stub.set_lamda(&InfoFactory::create<FileInfo>, [&info] { __DBG_STUB_INVOKE__ return info; });
    stub.set_lamda(&CoreEventsCaller::sendGetSelectedFiles, [] {
        __DBG_STUB_INVOKE__
        QList<QUrl> urls;
        urls.push_back(QUrl::fromLocalFile("/home"));
        return urls;
    });

    bool click { false };
    stub.set_lamda(&QAbstractButton::animateClick, [&click] {
        __DBG_STUB_INVOKE__
        click = true;
    });

    // check is dir
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes),
                   [](SyncFileInfo *, const SyncFileInfo::FileIsType type) {
                       __DBG_STUB_INVOKE__
                       if (type == OptInfoType::kIsDir)
                           return true;
                       return false;
                   });
    dialog.handleEnterPressed();
    EXPECT_FALSE(click);

    // check not is dir
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes),
                   [](SyncFileInfo *, const SyncFileInfo::FileIsType type) {
                       __DBG_STUB_INVOKE__
                       if (type == OptInfoType::kIsDir)
                           return false;
                       return true;
                   });
    dialog.handleEnterPressed();
    EXPECT_TRUE(click);
}

TEST_F(UT_FileDialog, onViewItemClicked)
{
    FileDialog dialog(QUrl("~"));
    QVariantMap data;
    data.insert("url", "~");
    data.insert("displayName", "test");
    dialog.onViewItemClicked(data);
    EXPECT_TRUE(data["displayName"] == "test");
}
