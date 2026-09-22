// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (gui/encryptprogressdialog.cpp):
//   EncryptProgressDialog ctor / initUI / setText / updateProgress / showResultPage /
//   showExportPage / onCicked / saveRecKey; header inline: setRecoveryKey
// Branch map (from get_code_snippet):
//   updateProgress: negative clamped to 0, normal value = p*100-1;
//   showResultPage: success/error icon paths;
//   onCicked: mismatched text early-return / invalid export path / valid path -> saveRecKey;
//   saveRecKey: open failure (shows error) / success writes file and accepts.

#include "stubext.h"

#include "gui/encryptprogressdialog.h"
#include "utils/encryptutils.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QAbstractButton>
#include <QDialog>
#include <QLabel>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimer>
#include <DFileDialog>
#include <DWaterProgress>
#include <DDialog>

using namespace dfmplugin_diskenc;
DWIDGET_USE_NAMESPACE

namespace {
class TestableProgressDialog : public EncryptProgressDialog
{
public:
    explicit TestableProgressDialog(QWidget *parent = nullptr)
        : EncryptProgressDialog(parent) {}

    using EncryptProgressDialog::onCicked;
    using EncryptProgressDialog::saveRecKey;
};
}   // namespace

class UT_EncryptProgressDialogCov : public testing::Test
{
protected:
    void TearDown() override
    {
        stub.clear();
    }

    DWaterProgress *waterProgress(EncryptProgressDialog *dlg) const
    {
        return dlg->findChild<DWaterProgress *>();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_EncryptProgressDialogCov, Construct_InitUiBuildsPages)
{
    // Arrange
    // Act
    // Assert
    TestableProgressDialog dlg;

    // Assert
    ASSERT_NE(waterProgress(&dlg), nullptr);
    EXPECT_EQ(dlg.maximumWidth(), 400);   // fixed width set by initUI
    EXPECT_EQ(dlg.getButtons().size(), 0);
}

TEST_F(UT_EncryptProgressDialogCov, SetText_UpdatesTitleAndMessage)
{
    // Arrange
    TestableProgressDialog dlg;

    // Act
    dlg.setText("Enc title", "Enc message");

    // Assert
    EXPECT_EQ(dlg.title(), QString("Enc title"));
    EXPECT_FALSE(dlg.findChildren<QLabel *>().isEmpty());
}

TEST_F(UT_EncryptProgressDialogCov, UpdateProgress_NormalAndNegative_Clamped)
{
    // Arrange
    TestableProgressDialog dlg;
    auto *prog = waterProgress(&dlg);
    ASSERT_NE(prog, nullptr);

    // Act
    dlg.updateProgress(0.5);
    int half = prog->value();
    dlg.updateProgress(-1.5);
    int clamped = prog->value();

    // Assert
    EXPECT_EQ(half, 49);   // 0.5*100-1, never shows 100
    EXPECT_EQ(clamped, 0);
}

TEST_F(UT_EncryptProgressDialogCov, ShowResultPage_SetsFullProgressAndConfirmButton)
{
    // Arrange
    TestableProgressDialog dlg;

    // Act
    dlg.showResultPage(true, "Done title", "Done message");

    // Assert
    EXPECT_EQ(waterProgress(&dlg)->value(), 100);
    EXPECT_EQ(dlg.getButtons().size(), 1);
    dlg.showResultPage(false, "Fail title", "Fail message");
    EXPECT_EQ(dlg.getButtons().size(), 2);   // second call adds another button
}

TEST_F(UT_EncryptProgressDialogCov, ShowExportPage_AddsReExportButton)
{
    // Arrange
    TestableProgressDialog dlg;

    // Act
    dlg.showExportPage();

    // Assert
    ASSERT_EQ(dlg.getButtons().size(), 1);
    EXPECT_EQ(dlg.getButton(0)->text().contains("Re-export"), true);
    EXPECT_NE(dlg.getButton(0)->text().contains("Re-export"), false);   // double-check stable state
}

TEST_F(UT_EncryptProgressDialogCov, OnCicked_MismatchedText_IgnoresClick)
{
    // Arrange
    TestableProgressDialog dlg;
    dlg.showExportPage();
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);

    // Act
    dlg.onCicked(0, "Some other button");

    // Assert
    EXPECT_EQ(acceptSpy.count(), 0);
    EXPECT_EQ(dlg.getButtons().size(), 1);
}

TEST_F(UT_EncryptProgressDialogCov, OnCicked_InvalidExportPath_ShowsError)
{
    // Arrange
    TestableProgressDialog dlg;
    dlg.showExportPage();
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    auto fileDlgFunc = static_cast<QUrl (*)(QWidget *, const QString &, const QUrl &, QFileDialog::Options, const QStringList &)>(
            &DFileDialog::getExistingDirectoryUrl);
    stub.set_lamda(fileDlgFunc, [](QWidget *, const QString &, const QUrl &, QFileDialog::Options, const QStringList &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/nonexistent-dir-ut");
    });
    int errorShown = 0;
    stub.set_lamda(&dialog_utils::showDialog, [&errorShown](const QString &, const QString &) -> int {
        __DBG_STUB_INVOKE__
        ++errorShown;
        return 0;
    });
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);

    // Act
    dlg.onCicked(0, QObject::tr("Re-export the recovery key"));

    // Assert
    EXPECT_EQ(errorShown, 1);
    EXPECT_EQ(acceptSpy.count(), 0);
}

TEST_F(UT_EncryptProgressDialogCov, SaveRecKey_ValidPath_WritesFileAndAccepts)
{
    // Arrange
    TestableProgressDialog dlg;
    dlg.setRecoveryKey("REC-KEY-123", "/dev/sdb1");   // (key, device)
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);

    // Act
    dlg.saveRecKey(dir.path());
    QString fileName = dir.path() + "/sdb1_recovery_key.txt";
    QFile f(fileName);

    // Assert
    EXPECT_EQ(acceptSpy.count(), 1);
    ASSERT_TRUE(f.exists());
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(QString::fromLocal8Bit(f.readAll()), QString("REC-KEY-123"));
}

TEST_F(UT_EncryptProgressDialogCov, SaveRecKey_InvalidPath_ShowsError)
{
    // Arrange
    TestableProgressDialog dlg;
    dlg.setRecoveryKey("REC-KEY-123", "/dev/sdb1");   // (key, device)
    int errorShown = 0;
    stub.set_lamda(&dialog_utils::showDialog, [&errorShown](const QString &, const QString &) -> int {
        __DBG_STUB_INVOKE__
        ++errorShown;
        return 0;
    });
    QSignalSpy acceptSpy(&dlg, &QDialog::accepted);

    // Act: a file path used as directory -> open fails
    QFile blocker("/tmp/ut_enc_prog_blocker");
    blocker.open(QIODevice::WriteOnly);
    blocker.close();
    dlg.saveRecKey("/tmp/ut_enc_prog_blocker/sub");

    // Assert
    EXPECT_EQ(errorShown, 1);
    EXPECT_EQ(acceptSpy.count(), 0);
    QFile::remove("/tmp/ut_enc_prog_blocker");
}
