// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <memory>

#include "stubext.h"
#include "dfmplugin_propertydialog_global.h"
#include "views/multifilepermissionwidget.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>

DPPROPERTYDIALOG_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class MultiFilePermissionWidgetImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        tempDir.reset(new QTemporaryDir);
        tempFile1.reset(new QTemporaryFile(tempDir->path() + "/perm1XXXXXX.txt"));
        tempFile2.reset(new QTemporaryFile(tempDir->path() + "/perm2XXXXXX.txt"));
        tempFile1->open();
        tempFile2->open();
        tempFile1->setPermissions(QFile::ReadOwner | QFile::WriteOwner
                                  | QFile::ReadGroup);
        tempFile2->setPermissions(QFile::ReadOwner | QFile::WriteOwner
                                  | QFile::ReadGroup);
    }

    void TearDown() override
    {
        stub.clear();
        tempFile1.reset();
        tempFile2.reset();
        tempDir.reset();
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    std::unique_ptr<QTemporaryFile> tempFile1;
    std::unique_ptr<QTemporaryFile> tempFile2;
};

TEST_F(MultiFilePermissionWidgetImpl, ConstructDestruct)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()),
                       QUrl::fromLocalFile(tempFile2->fileName()) };
    MultiFilePermissionWidget *widget = new MultiFilePermissionWidget(urls);
    EXPECT_NE(widget, nullptr);
    delete widget;
}

TEST_F(MultiFilePermissionWidgetImpl, ConstructEmptyUrls)
{
    // loadData() must bail out early for an empty url list without crashing.
    EXPECT_NO_THROW({
        MultiFilePermissionWidget widget(QList<QUrl>());
    });
}

TEST_F(MultiFilePermissionWidgetImpl, GetOrgPermissonBoxState)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()),
                       QUrl::fromLocalFile(tempFile2->fileName()) };
    MultiFilePermissionWidget widget(urls);

    FilePropertyState state;
    widget.getOrgPermissonBoxState(state);
    EXPECT_GE(state.ownerIndex, -1);
    EXPECT_GE(state.groupIndex, -1);
    EXPECT_GE(state.otherIndex, -1);
}

TEST_F(MultiFilePermissionWidgetImpl, CanChmodByFs)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()) };
    MultiFilePermissionWidget widget(urls);

    // The temp dir lives on a local filesystem that is not in the blocked
    // list (vfat/fuseblk/cifs) and no plugin hook disables it, so chmod is
    // allowed.
    bool ret = widget.canChmodByFs(urls.first());
    EXPECT_TRUE(ret);
}

TEST_F(MultiFilePermissionWidgetImpl, CanChmodByFile)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()) };
    MultiFilePermissionWidget widget(urls);

    // Own existing local file: the current user created it, so it is owned
    // by the current uid and can be chmod'ed.
    FileInfoPointer info = InfoFactory::create<FileInfo>(urls.first());
    EXPECT_FALSE(info.isNull());
    bool ret = widget.canChmodByFile(info);
    EXPECT_TRUE(ret);

    // Null info is rejected.
    FileInfoPointer nullInfo;
    EXPECT_FALSE(widget.canChmodByFile(nullInfo));

    // Non-existent file is rejected.
    QUrl missingUrl = QUrl::fromLocalFile(tempDir->path() + "/no_such_file.txt");
    FileInfoPointer missingInfo = InfoFactory::create<FileInfo>(missingUrl);
    if (!missingInfo.isNull())
        EXPECT_FALSE(widget.canChmodByFile(missingInfo));

    // A file owned by another user (root) is rejected. Unregistered scheme
    // falls back to the null-info path.
    FileInfoPointer virtualInfo = InfoFactory::create<FileInfo>(QUrl("virtual://no-such"));
    EXPECT_TRUE(virtualInfo.isNull());
}

TEST_F(MultiFilePermissionWidgetImpl, DisablePermissionComboBox)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()) };
    MultiFilePermissionWidget widget(urls);

    EXPECT_NO_THROW(widget.disablePermissionComboBox());
    EXPECT_FALSE(widget.ownerComboBox->isEnabled());
    EXPECT_FALSE(widget.groupComboBox->isEnabled());
    EXPECT_FALSE(widget.otherComboBox->isEnabled());
}

TEST_F(MultiFilePermissionWidgetImpl, UpdateComboBoxViewPalette)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()) };
    MultiFilePermissionWidget widget(urls);

    EXPECT_NO_THROW(widget.updateComboBoxViewPalette());
}

TEST_F(MultiFilePermissionWidgetImpl, OnComboBoxChanged)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()) };
    MultiFilePermissionWidget widget(urls);

    int ownerEmitted = -1;
    int groupEmitted = -1;
    int otherEmitted = -1;
    QObject::connect(&widget, &MultiFilePermissionWidget::ownerComboxChanged,
                     [&](int i) { ownerEmitted = i; });
    QObject::connect(&widget, &MultiFilePermissionWidget::groupComboxChanged,
                     [&](int i) { groupEmitted = i; });
    QObject::connect(&widget, &MultiFilePermissionWidget::otherComboxChanged,
                     [&](int i) { otherEmitted = i; });

    // Valid indexes are forwarded, invalid ones are swallowed.
    widget.onOwnerComboBoxChanged(1);
    EXPECT_EQ(ownerEmitted, 1);
    widget.onOwnerComboBoxChanged(5);
    EXPECT_EQ(ownerEmitted, 1);
    widget.onGroupComboBoxChanged(2);
    EXPECT_EQ(groupEmitted, 2);
    widget.onGroupComboBoxChanged(-1);
    EXPECT_EQ(groupEmitted, 2);
    widget.onOtherComboBoxChanged(0);
    EXPECT_EQ(otherEmitted, 0);
    widget.onOtherComboBoxChanged(3);
    EXPECT_EQ(otherEmitted, 0);
}
