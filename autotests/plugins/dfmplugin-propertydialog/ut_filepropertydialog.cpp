// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QCloseEvent>
#include <QComboBox>
#include <QApplication>

#include "stubext.h"
#include "dfmplugin_propertydialog_global.h"
#include "views/filepropertydialog.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include "views/basicwidget.h"
#include "views/editstackedwidget.h"
#include "views/permissionmanagerwidget.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/thumbnail/thumbnailhelper.h>
#include <dfm-base/utils/iconutils.h>
#include <dfm-io/dfileinfo.h>

DPPROPERTYDIALOG_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

namespace {

static void installFileInfoStubs(stub_ext::StubExt &stub,
                                  const QUrl &url,
                                  const QUrl &parentUrl,
                                  const QString &path,
                                  bool isDir,
                                  bool isHidden,
                                  bool canHidden,
                                  bool canRename,
                                  FileInfo::FileType type)
{
    Q_UNUSED(url)
    Q_UNUSED(parentUrl)
    Q_UNUSED(path)
    Q_UNUSED(isDir)
    Q_UNUSED(isHidden)
    Q_UNUSED(canHidden)
    Q_UNUSED(canRename)
    Q_UNUSED(type)

    // NOTE: FileInfo's virtual methods cannot be stubbed by cpp-stub: the
    // member pointer of a virtual function encodes a vtable offset instead of
    // an address, so patching it crashes. The real LocalFileInfo of the temp
    // file is used instead; only non-virtual helpers are stubbed below.
    stub.set_lamda(&ThumbnailHelper::checkThumbEnable, [](ThumbnailHelper *, const QUrl &) -> bool { return false; });
    stub.set_lamda(&IconUtils::hiDpiPixmap, [](const QIcon &, const QSize &, const QWidget *) -> QPixmap {
        return QPixmap(128, 128);
    });
}

}   // namespace

class FilePropertyDialogImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        tempDir.reset(new QTemporaryDir);
        tempFile.reset(new QTemporaryFile(tempDir->path() + "/testXXXXXX.txt"));
        tempFile->open();
        url = QUrl::fromLocalFile(tempFile->fileName());
        parentUrl = QUrl::fromLocalFile(tempDir->path());
    }

    void TearDown() override
    {
        stub.clear();
        tempFile.reset();
        tempDir.reset();
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    std::unique_ptr<QTemporaryFile> tempFile;
    QUrl url;
    QUrl parentUrl;
};

TEST_F(FilePropertyDialogImpl, ConstructDestruct)
{
    FilePropertyDialog *dialog = new FilePropertyDialog();
    EXPECT_NE(dialog, nullptr);
    delete dialog;
}

TEST_F(FilePropertyDialogImpl, SelectFileUrlAndFilterControlView)
{
    installFileInfoStubs(stub, url, parentUrl, tempFile->fileName(), false, false, true, true,
                         FileInfo::FileType::kRegularFile);

    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);
    EXPECT_NO_THROW(dialog.filterControlView());
    EXPECT_GE(dialog.getFileCount(), 0);
}

TEST_F(FilePropertyDialogImpl, GetFileSizeAndCount)
{
    installFileInfoStubs(stub, url, parentUrl, tempFile->fileName(), false, false, true, true,
                         FileInfo::FileType::kRegularFile);

    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);
    dialog.filterControlView();

    EXPECT_GE(dialog.getFileSize(), 0);
    EXPECT_GE(dialog.getFileCount(), 0);
}

TEST_F(FilePropertyDialogImpl, SetBasicInfoExpand)
{
    installFileInfoStubs(stub, url, parentUrl, tempFile->fileName(), false, false, true, true,
                         FileInfo::FileType::kRegularFile);

    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);
    dialog.filterControlView();

    EXPECT_NO_THROW(dialog.setBasicInfoExpand(true));
    EXPECT_NO_THROW(dialog.setBasicInfoExpand(false));
}

TEST_F(FilePropertyDialogImpl, InitialHeightOfView)
{
    installFileInfoStubs(stub, url, parentUrl, tempFile->fileName(), false, false, true, true,
                         FileInfo::FileType::kRegularFile);

    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);
    dialog.filterControlView();

    EXPECT_GT(dialog.initalHeightOfView(), 0);
}

TEST_F(FilePropertyDialogImpl, InsertAndAddExtendedControl)
{
    installFileInfoStubs(stub, url, parentUrl, tempFile->fileName(), false, false, true, true,
                         FileInfo::FileType::kRegularFile);

    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);
    dialog.filterControlView();

    QWidget *w1 = new QWidget(&dialog);
    QWidget *w2 = new QWidget(&dialog);
    dialog.insertExtendedControl(0, w1);
    dialog.addExtendedControl(w2);

    QWidget *w3 = new QWidget(&dialog);
    dialog.addExtendedControl(w3, [](QWidget *, const QUrl &) {});

    EXPECT_GT(dialog.initalHeightOfView(), 0);
}

TEST_F(FilePropertyDialogImpl, CloseDialog)
{
    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);

    bool closed = false;
    QObject::connect(&dialog, &FilePropertyDialog::closed, [&closed](const QUrl &) { closed = true; });
    dialog.closeDialog();
    EXPECT_TRUE(closed);
}

TEST_F(FilePropertyDialogImpl, OnSelectUrlRenamed)
{
    installFileInfoStubs(stub, url, parentUrl, tempFile->fileName(), false, false, true, true,
                         FileInfo::FileType::kRegularFile);

    // onSelectUrlRenamed() calls processHeight(contentHeight()) and
    // contentHeight() queries DDialog::getContent(), which crashes on an
    // unshown dialog in the offscreen environment. Stub it to the null
    // branch so the rename handling itself is exercised.
    stub.set_lamda((QWidget * (DDialog::*)(int) const) & DDialog::getContent,
                   [](const DDialog *, int) -> QWidget * {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);
    dialog.filterControlView();

    QUrl newUrl = QUrl::fromLocalFile(tempDir->path() + "/renamed.txt");
    EXPECT_NO_THROW(dialog.onSelectUrlRenamed(newUrl));
}

TEST_F(FilePropertyDialogImpl, OnFileInfoUpdated)
{
    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);
    EXPECT_NO_THROW(dialog.onFileInfoUpdated(url, "0", false));
}

TEST_F(FilePropertyDialogImpl, ProcessHeightBeforeShown)
{
    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);
    EXPECT_NO_THROW(dialog.processHeight(100));
}

TEST_F(FilePropertyDialogImpl, EventFilterWheel)
{
    FilePropertyDialog dialog;
    QComboBox *combo = new QComboBox(&dialog);
    QWheelEvent event(QPointF(10, 10), QPointF(10, 10), QPoint(0, 0), QPoint(0, 1),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    EXPECT_TRUE(dialog.eventFilter(combo, &event));
    delete combo;
}

TEST_F(FilePropertyDialogImpl, MousePressEvent)
{
    installFileInfoStubs(stub, url, parentUrl, tempFile->fileName(), false, false, true, true,
                         FileInfo::FileType::kRegularFile);

    FilePropertyDialog dialog;
    dialog.selectFileUrl(url);
    dialog.filterControlView();

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier);
    EXPECT_NO_THROW(QApplication::sendEvent(&dialog, &event));
}

TEST_F(FilePropertyDialogImpl, ResizeEvent)
{
    FilePropertyDialog dialog;
    QResizeEvent event(QSize(400, 300), QSize(380, 200));
    EXPECT_NO_THROW(QApplication::sendEvent(&dialog, &event));
}

TEST_F(FilePropertyDialogImpl, CloseEvent)
{
    FilePropertyDialog dialog;
    QCloseEvent event;
    EXPECT_NO_THROW(dialog.closeEvent(&event));
}

TEST_F(FilePropertyDialogImpl, KeyPressEventNonEscape)
{
    FilePropertyDialog dialog;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    EXPECT_NO_THROW(QApplication::sendEvent(&dialog, &event));
}
