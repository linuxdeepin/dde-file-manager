// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialog/processdialog.h"
#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QProcess>
#include <QIODevice>
#include <signal.h>

using namespace dfm_upgrade;

class TestProcessDialog : public testing::Test {
protected:
    void SetUp() override {
        dialog = new ProcessDialog();
    }

    void TearDown() override {
        delete dialog;
        dialog = nullptr;
        // Clear all stubs to ensure test isolation
        stub.clear();
    }

    ProcessDialog* dialog {nullptr};
    stub_ext::StubExt stub;
};

TEST_F(TestProcessDialog, constructor)
{
    // Constructor should not throw
    SUCCEED();
}

TEST_F(TestProcessDialog, initialize_DesktopMode)
{
    dialog->initialize(true);
    EXPECT_TRUE(dialog->onDesktop);
    EXPECT_GT(dialog->accept, -1);
    EXPECT_TRUE(dialog->message().contains("File Manager"));
}

TEST_F(TestProcessDialog, initialize_FileManagerMode)
{
    dialog->initialize(false);
    EXPECT_FALSE(dialog->onDesktop);
    EXPECT_GT(dialog->accept, -1);
    EXPECT_TRUE(dialog->message().contains("desktop services"));
}

TEST_F(TestProcessDialog, execDialog_fm_nopreocess)
{
    dialog->accept = 1;
    dialog->onDesktop = false;

    QString path;

    stub.set_lamda(&ProcessDialog::queryProcess, [&path](ProcessDialog *, const QString &exe){
        path = exe;
        return QList<int>();
    });

    bool execed = false;
    stub.set_lamda(VADDR(ProcessDialog, exec), [&execed](){
        execed = true;
        return 0;
    });

    EXPECT_TRUE(dialog->execDialog());
    EXPECT_FALSE(execed);
    EXPECT_EQ(path, QString("/usr/bin/dde-shell"));
}

TEST_F(TestProcessDialog, execDialog_desktop_noprocess)
{
    dialog->accept = 1;
    dialog->onDesktop = true;

    QString path;

    stub.set_lamda(&ProcessDialog::queryProcess, [&path](ProcessDialog *, const QString &exe){
        path = exe;
        return QList<int>();
    });

    bool execed = false;
    stub.set_lamda(VADDR(ProcessDialog, exec), [&execed](){
        execed = true;
        return 0;
    });

    EXPECT_TRUE(dialog->execDialog());
    EXPECT_FALSE(execed);
    EXPECT_EQ(path, QString("/usr/libexec/dde-file-manager"));
}

TEST_F(TestProcessDialog, execDialog_reject)
{
    dialog->accept = 1;

    stub.set_lamda(&ProcessDialog::queryProcess, [](){
        return QList<int>{0, 1};
    });

    bool execed = false;
    stub.set_lamda(VADDR(ProcessDialog, exec), [&execed](){
        execed = true;
        return 0;
    });

    QList<int> killed;
    stub.set_lamda(&ProcessDialog::killAll, [&killed](ProcessDialog *, const QList<int> &list){
        killed = list;
        return ;
    });

    EXPECT_FALSE(dialog->execDialog());
    EXPECT_TRUE(execed);
    EXPECT_TRUE(killed.isEmpty());
    EXPECT_FALSE(dialog->killed);
}

TEST_F(TestProcessDialog, execDialog_accept)
{
    dialog->accept = 1;

    QList<int> ret {1, 2};
    stub.set_lamda(&ProcessDialog::queryProcess, [ret](){
        return ret;
    });

    bool execed = false;
    stub.set_lamda(VADDR(ProcessDialog, exec), [&execed](){
        execed = true;
        return 1;
    });

    QList<int> killed;
    stub.set_lamda(&ProcessDialog::killAll, [&killed](ProcessDialog *, const QList<int> &list){
        killed = list;
        return ;
    });

    EXPECT_TRUE(dialog->execDialog());
    EXPECT_TRUE(execed);
    EXPECT_TRUE(dialog->killed);
    EXPECT_EQ(killed, ret);
}

TEST_F(TestProcessDialog, restart_DesktopMode)
{
    dialog->onDesktop = true;
    dialog->killed = false;
    // Should not restart for desktop mode if not killed
    dialog->restart();
    SUCCEED();
}

TEST_F(TestProcessDialog, restart_FileManagerMode)
{
    dialog->onDesktop = false;
    dialog->killed = true;

    bool processStarted = false;
    using StartDetachedFuncPtr = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
    stub.set_lamda(static_cast<StartDetachedFuncPtr>(&QProcess::startDetached), [&](const QString &, const QStringList &, const QString &, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        processStarted = true;
        return true;
    });

    dialog->restart();
    EXPECT_TRUE(processStarted);
}

TEST_F(TestProcessDialog, queryProcess)
{
    // This test relies on the actual file system, so we create temporary files.
    QString procDir = QDir::tempPath() + "/fake_proc_query";
    QDir().mkpath(procDir + "/123");
    QDir().mkpath(procDir + "/456");

    QFile exeFile(procDir + "/123/exe");
    exeFile.open(QIODevice::WriteOnly);
    exeFile.write(QFile::encodeName("/usr/bin/dde-file-manager"));
    exeFile.close();

    QFile exeFile2(procDir + "/456/exe");
    exeFile2.open(QIODevice::WriteOnly);
    exeFile2.write(QFile::encodeName("/usr/bin/dde-shell"));
    exeFile2.close();

    QFile loginUidFile(procDir + "/123/loginuid");
    loginUidFile.open(QIODevice::WriteOnly);
    loginUidFile.write("1000");
    loginUidFile.close();

    // FIXME: Mock QDir to use our fake proc directory
    stub.set_lamda(&QDirIterator::path, [&](QDirIterator *self) -> QString {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return procDir;
    });

    // Mock QFile::symLinkTarget to return the path we wrote
    stub.set_lamda((QString(*)(const QString &))&QFile::symLinkTarget, [procDir](const QString &link) {
        __DBG_STUB_INVOKE__
        if (link == procDir + "/123/exe") {
            return "/usr/bin/dde-file-manager";
        }
        return "";
    });

    EXPECT_NO_THROW(dialog->queryProcess("/usr/bin/dde-file-manager"));

    // Clean up
    QDir(procDir).removeRecursively();
}

TEST_F(TestProcessDialog, killAll)
{
    QList<int> pids = {123, 456};

    QList<int> killedPids;
    stub.set_lamda((int (*)(pid_t, int))&kill, [&killedPids](pid_t pid, int) {
        killedPids.append(pid);
        return 0;
    });

    dialog->killAll(pids);
    EXPECT_EQ(killedPids.size(), 2);
    EXPECT_TRUE(killedPids.contains(123));
    EXPECT_TRUE(killedPids.contains(456));
}

TEST_F(TestProcessDialog, targetExe)
{
    QString procDir = QDir::tempPath() + "/fake_proc_target";
    QDir().mkpath(procDir + "/123");

    QFile exeFile(procDir + "/123/exe");
    exeFile.open(QIODevice::WriteOnly);
    exeFile.write(QFile::encodeName("/usr/bin/dde-file-manager"));
    exeFile.close();

    // Stub QFileInfo::symLinkTarget to return the path we wrote
    stub.set_lamda((QString(*)())&QFileInfo::symLinkTarget, [procDir]() -> QString {
        __DBG_STUB_INVOKE__
        qDebug() << "Mock the symLinkTarget: " << procDir + "/123/exe";
        return "/usr/bin/dde-file-manager";
    });

    QString result = dialog->targetExe(procDir + "/123");
    EXPECT_EQ(result, QString("/usr/bin/dde-file-manager"));

    // Clean up
    QDir(procDir).removeRecursively();
}

TEST_F(TestProcessDialog, targetUid)
{
    QString procDir = QDir::tempPath() + "/fake_proc_uid";
    QDir().mkpath(procDir + "/123");

    QFile loginUidFile(procDir + "/123/loginuid");
    loginUidFile.open(QIODevice::WriteOnly);
    loginUidFile.write("1000");
    loginUidFile.close();

    int result = dialog->targetUid(procDir + "/123");
    EXPECT_EQ(result, 1000);

    // Clean up
    QDir(procDir).removeRecursively();
}

TEST_F(TestProcessDialog, isEqual_ExactMatch)
{
    bool result = dialog->isEqual("/usr/bin/dde-file-manager", "/usr/bin/dde-file-manager");
    EXPECT_TRUE(result);
}

TEST_F(TestProcessDialog, isEqual_DeletedSuffix)
{
    bool result = dialog->isEqual("/usr/bin/dde-file-manager (deleted)", "/usr/bin/dde-file-manager");
    EXPECT_TRUE(result);
}

TEST_F(TestProcessDialog, isEqual_NoMatch)
{
    bool result = dialog->isEqual("/usr/bin/dde-file-manager", "/usr/bin/dde-shell");
    EXPECT_FALSE(result);
}