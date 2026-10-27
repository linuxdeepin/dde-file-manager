// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "dialog/processdialog.h"

#include <DDialog>

#include <QApplication>
#include <QProcess>
#include <QTimer>
#include <QVariant>

using namespace dfm_upgrade;

class ProcessDialogTest : public testing::Test
{
protected:
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(ProcessDialogTest, Constructor_DDialogBased_NoCrash)
{
    // Arrange / Act
    ProcessDialog dialog;

    // Assert
    EXPECT_TRUE(dialog.isWidgetType());
    EXPECT_EQ(dialog.accept, -1);
    EXPECT_EQ(dialog.getButtonIndexByText(ProcessDialog::tr("Update", "button")), -1);
}

TEST_F(ProcessDialogTest, Initialize_DesktopMode_SetsMessageAndButtons)
{
    // Arrange
    ProcessDialog dialog;

    // Act
    dialog.initialize(true);

    // Assert
    EXPECT_TRUE(dialog.onDesktop);
    EXPECT_NE(dialog.accept, -1);   // accept button index recorded
    EXPECT_NE(dialog.getButtonIndexByText(ProcessDialog::tr("Update", "button")), -1);
    EXPECT_NE(dialog.getButtonIndexByText(ProcessDialog::tr("Cancel", "button")), -1);
}

TEST_F(ProcessDialogTest, Initialize_ShellMode_SetsMessageAndButtons)
{
    // Arrange
    ProcessDialog dialog;

    // Act
    dialog.initialize(false);

    // Assert
    EXPECT_FALSE(dialog.onDesktop);
    EXPECT_NE(dialog.accept, -1);
    EXPECT_NE(dialog.getButtonIndexByText(ProcessDialog::tr("Update", "button")), -1);
    EXPECT_NE(dialog.getButtonIndexByText(ProcessDialog::tr("Cancel", "button")), -1);
}

TEST_F(ProcessDialogTest, ExecDialog_NoRunningProcess_ReturnsTrueWithoutAsk)
{
    // Arrange
    ProcessDialog dialog;
    dialog.initialize(true);
    QString queriedExec;
    stub.set_lamda(&ProcessDialog::queryProcess,
                   [&queriedExec](ProcessDialog *, const QString &exec) -> QList<int> {
                       __DBG_STUB_INVOKE__
                       queriedExec = exec;
                       return {};
                   });

    // Act
    const bool ok = dialog.execDialog();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(queriedExec, QString("/usr/libexec/dde-file-manager"));
}

TEST_F(ProcessDialogTest, Restart_NotKilledOrDesktopMode_DoesNothing)
{
    // Arrange: killed stays false -> no restart attempted
    ProcessDialog dialog;
    dialog.initialize(true);
    int startCalls = 0;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                   [&startCalls](const QString &, const QStringList &, const QString &, qint64 *) -> bool {
                       __DBG_STUB_INVOKE__
                       startCalls++;
                       return true;
                   });

    // Act
    dialog.restart();

    // Assert
    EXPECT_EQ(startCalls, 0);
}

TEST_F(ProcessDialogTest, Restart_KilledShellMode_RestartsDdeShellService)
{
    // Arrange
    ProcessDialog dialog;
    dialog.initialize(false);
    dialog.killed = true;
    int startCalls = 0;
    QStringList capturedArgs;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                   [&startCalls, &capturedArgs](const QString &program, const QStringList &args, const QString &, qint64 *) -> bool {
                       __DBG_STUB_INVOKE__
                       startCalls++;
                       capturedArgs = args;
                       return true;
                   });

    // Act
    dialog.restart();

    // Assert
    EXPECT_EQ(startCalls, 1);
    EXPECT_TRUE(capturedArgs.contains(QString("restart")));
    EXPECT_TRUE(capturedArgs.join(' ').contains(QString("dde-shell")));
}

TEST_F(ProcessDialogTest, TargetExe_ResolvesProcSelfSymlink)
{
    // Arrange
    ProcessDialog dialog;

    // Act
    const QString exe = dialog.targetExe("/proc/self");

    // Assert
    EXPECT_EQ(exe, QCoreApplication::applicationFilePath());
    EXPECT_TRUE(dialog.targetExe("/proc/0/does-not-exist").isEmpty());
}

TEST_F(ProcessDialogTest, KillAll_InvalidPid_Harmless)
{
    // Arrange
    ProcessDialog dialog;
    const QList<int> stalePids { 999999, 999998 };

    // Act / Assert: kill() fails with ESRCH, nothing happens
    EXPECT_NO_FATAL_FAILURE(dialog.killAll(stalePids));
}

TEST_F(ProcessDialogTest, IsEqual_ExactDeletedAndMismatch)
{
    // Arrange
    ProcessDialog dialog;

    // Act / Assert
    EXPECT_TRUE(dialog.isEqual("/usr/bin/dde-shell", "/usr/bin/dde-shell"));
    EXPECT_TRUE(dialog.isEqual("/usr/bin/dde-shell (deleted)", "/usr/bin/dde-shell"));
    EXPECT_FALSE(dialog.isEqual("/usr/bin/other", "/usr/bin/dde-shell"));
    EXPECT_FALSE(dialog.isEqual(QString(), QString("/usr/bin/dde-shell")));
}
