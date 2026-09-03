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
#include "views/multifilebasicinfowidget.h"
#include "views/skippartiallycheckbox.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/filescanner.h>
#include <dfm-io/dfileinfo.h>

DPPROPERTYDIALOG_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

namespace {

// NOTE: the FileInfo interface methods (exists/canAttributes/timeOf/
// isAttributes/fileName...) are all VIRTUAL. cpp-stub cannot patch them:
// a member-pointer kept as-is decodes to an odd vtable index and crashes
// Stub::set, while the VADDR() C-style cast yields the address of a
// compiler-generated thunk that virtual dispatch never goes through
// (patch succeeds but has no effect). Therefore these tests rely on real
// files created by the fixture instead of stubbing FileInfo.

}   // namespace

class MultiFileBasicInfoWidgetImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        tempDir.reset(new QTemporaryDir);
        tempFile1.reset(new QTemporaryFile(tempDir->path() + "/multi1XXXXXX.txt"));
        tempFile2.reset(new QTemporaryFile(tempDir->path() + "/multi2XXXXXX.txt"));
        tempFileHidden.reset(new QTemporaryFile(tempDir->path() + "/.multiHiddenXXXXXX.txt"));
        tempFile1->open();
        tempFile2->open();
        tempFileHidden->open();
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
    std::unique_ptr<QTemporaryFile> tempFileHidden;
};

TEST_F(MultiFileBasicInfoWidgetImpl, ConstructDestruct)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()), QUrl::fromLocalFile(tempFile2->fileName()) };
    MultiFileBasicInfoWidget *widget = new MultiFileBasicInfoWidget(urls);
    EXPECT_NE(widget, nullptr);
    delete widget;
}

TEST_F(MultiFileBasicInfoWidgetImpl, GetOrgHideBoxStateUnchecked)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()), QUrl::fromLocalFile(tempFile2->fileName()) };

    MultiFileBasicInfoWidget widget(urls);
    FilePropertyState state;
    widget.getOrgHideBoxState(state);
    EXPECT_EQ(state.hideState, Qt::Unchecked);
}

TEST_F(MultiFileBasicInfoWidgetImpl, GetOrgHideBoxStateHiddenFileNotManaged)
{
    // setHideState() refuses to manage dot-prefixed files
    // ("the hidden attribute does not manage files starting with '.'"):
    // it disables the checkbox and returns without setting
    // PartiallyChecked, so no real filesystem state can produce it.
    QList<QUrl> urls { QUrl::fromLocalFile(tempFileHidden->fileName()), QUrl::fromLocalFile(tempFile2->fileName()) };

    MultiFileBasicInfoWidget widget(urls);
    FilePropertyState state;
    widget.getOrgHideBoxState(state);
    EXPECT_EQ(state.hideState, Qt::Unchecked);
}

TEST_F(MultiFileBasicInfoWidgetImpl, FilesHideStateChanged)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()), QUrl::fromLocalFile(tempFile2->fileName()) };

    MultiFileBasicInfoWidget widget(urls);
    bool emitted = false;
    QObject::connect(&widget, &MultiFileBasicInfoWidget::hideBoxStateChanged, [&emitted](int) { emitted = true; });

    QMetaObject::invokeMethod(&widget, "filesHideStateChanged", Q_ARG(int, Qt::Checked));
    EXPECT_TRUE(emitted);
}

TEST_F(MultiFileBasicInfoWidgetImpl, UpdateFilesCountAndSizeLabel)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()), QUrl::fromLocalFile(tempFile2->fileName()) };

    MultiFileBasicInfoWidget widget(urls);
    FileScanner::ScanResult result;
    result.totalSize = 8192;
    result.fileCount = 3;
    result.directoryCount = 1;
    EXPECT_NO_THROW(widget.updateFilesCountAndSizeLabel(result));
}
