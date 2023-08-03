// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/previewdialogmanager.h"

#include <gtest/gtest.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>

DPFILEPREVIEW_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_PreviewDialogManager, showPreviewDialog_one)
{
    bool isOk { false };

    PreviewDialogManager::instance()->showPreviewDialog(0, QList<QUrl>(), QList<QUrl>());

    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, []{
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    });
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), []{
        return true;
    });
    stub.set_lamda(&DialogManager::showErrorDialog, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> selectUrls;
    selectUrls << QUrl("file:///UT/UT_TEST");
    QList<QUrl> dirUrls;
    dirUrls << QUrl("file:///UT");
    PreviewDialogManager::instance()->showPreviewDialog(0, selectUrls, dirUrls);

    EXPECT_TRUE(isOk);
}

TEST(UT_PreviewDialogManager, showPreviewDialog_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, []{
        return nullptr;
    });
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&FilePreviewDialog::setEntryUrlList, []{});
    stub.set_lamda(&QWidget::show, []{});
    stub.set_lamda(&QWidget::raise, []{});
    stub.set_lamda(&QWidget::activateWindow, []{});
    stub.set_lamda(&DAbstractDialog::moveToCenter, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> selectUrls;
    selectUrls << QUrl("file:///UT/UT_TEST");
    QList<QUrl> dirUrls;
    dirUrls << QUrl("file:///UT");
    FilePreviewDialog dialog(dirUrls);
    PreviewDialogManager::instance()->filePreviewDialog = &dialog;
    PreviewDialogManager::instance()->showPreviewDialog(0, selectUrls, dirUrls);

    EXPECT_TRUE(isOk);
}
