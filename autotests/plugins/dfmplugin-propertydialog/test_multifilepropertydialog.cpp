// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QList>
#include <QApplication>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <memory>
#include "stubext.h"

#include "views/multifilepropertiesdialog.h"
#include "events/propertyeventcall.h"
#include "dfmplugin_propertydialog_global.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/universalutils.h>

DPPROPERTYDIALOG_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class TestMultiFilePropertiesDialog : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        tempDir.reset(new QTemporaryDir);
        tempFile1.reset(new QTemporaryFile(tempDir->path() + "/mfprop1XXXXXX.txt"));
        tempFile2.reset(new QTemporaryFile(tempDir->path() + "/mfprop2XXXXXX.txt"));
        tempFile1->open();
        tempFile2->open();
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

// Test MultiFilePropertiesDialog class
TEST_F(TestMultiFilePropertiesDialog, MultiFilePropertiesDialogConstructor)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test1.txt") << QUrl::fromLocalFile("/tmp/test2.txt");

    MultiFilePropertiesDialog *dialog = new MultiFilePropertiesDialog(urls);
    EXPECT_NE(dialog, nullptr);
    delete dialog;
}

TEST_F(TestMultiFilePropertiesDialog, MultiFilePropertiesDialogDestructor)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test1.txt") << QUrl::fromLocalFile("/tmp/test2.txt");

    MultiFilePropertiesDialog *dialog = new MultiFilePropertiesDialog(urls);
    EXPECT_NO_THROW(delete dialog);
}

TEST_F(TestMultiFilePropertiesDialog, ConstructorWithRealFiles)
{
    // Real files exercise the normal loadData() paths of the embedded
    // MultiFileBasicInfoWidget and MultiFilePermissionWidget.
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()),
                       QUrl::fromLocalFile(tempFile2->fileName()) };
    EXPECT_NO_THROW({
        MultiFilePropertiesDialog dialog(urls);
    });
}

TEST_F(TestMultiFilePropertiesDialog, ShowEventProcessHeight)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()) };
    MultiFilePropertiesDialog dialog(urls);

    dialog.show();   // triggers showEvent on the offscreen platform
    QMetaObject::invokeMethod(&dialog, "processHeight");
    EXPECT_GT(dialog.height(), 0);
}

TEST_F(TestMultiFilePropertiesDialog, HandleStateChanged)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()),
                       QUrl::fromLocalFile(tempFile2->fileName()) };
    MultiFilePropertiesDialog dialog(urls);

    // Changing a field enables the save button; every handle slot must run.
    QMetaObject::invokeMethod(&dialog, "handleOwnerBoxStateChanged", Q_ARG(int, 2));
    EXPECT_TRUE(dialog.saveBtn->isEnabled());
    QMetaObject::invokeMethod(&dialog, "handleGroupBoxStateChanged", Q_ARG(int, 1));
    EXPECT_TRUE(dialog.saveBtn->isEnabled());
    QMetaObject::invokeMethod(&dialog, "handleOtherBoxStateChanged", Q_ARG(int, 1));
    EXPECT_TRUE(dialog.saveBtn->isEnabled());
    QMetaObject::invokeMethod(&dialog, "handleHideBoxStateChanged", Q_ARG(int, Qt::Unchecked));
    EXPECT_TRUE(dialog.saveBtn->isEnabled());
}

TEST_F(TestMultiFilePropertiesDialog, SaveBtnClickedNoChange)
{
    // With no state modification the save handler simply accepts the dialog.
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()) };
    MultiFilePropertiesDialog dialog(urls);

    QMetaObject::invokeMethod(&dialog, "saveBtnClicked");
    EXPECT_EQ(dialog.result(), static_cast<int>(QDialog::Accepted));
}

TEST_F(TestMultiFilePropertiesDialog, SaveBtnClickedWithHideChange)
{
    QList<QUrl> urls { QUrl::fromLocalFile(tempFile1->fileName()),
                       QUrl::fromLocalFile(tempFile2->fileName()) };
    MultiFilePropertiesDialog dialog(urls);
    dialog.show();

    // changeFilesHideState() resolves the active window; make it deterministic
    // on the offscreen platform.
    stub.set_lamda(&QApplication::activeWindow,
                   [&dialog]() -> QWidget * { return &dialog; });
    // Avoid real event dispatch and desktop notifications.
    stub.set_lamda(&PropertyEventCall::sendFilesHideOrVisible,
                   [](quint64, const QUrl &, const QList<QUrl> &, bool) {
                   });
    stub.set_lamda(static_cast<void (*)(const QString &, const QString &)>(&UniversalUtils::notifyMessage),
                   [](const QString &, const QString &) {
                   });

    QMetaObject::invokeMethod(&dialog, "handleHideBoxStateChanged", Q_ARG(int, Qt::Checked));
    EXPECT_TRUE(dialog.saveBtn->isEnabled());

    QMetaObject::invokeMethod(&dialog, "saveBtnClicked");
    EXPECT_EQ(dialog.result(), static_cast<int>(QDialog::Accepted));
}
