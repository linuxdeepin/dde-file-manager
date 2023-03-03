// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialog/processdialog.h"

#include "units/unitlist.h"

#include "builtininterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace dfm_upgrade;

TEST(ProcessDialog, initialize_fm)
{
    ProcessDialog dlg;
    dlg.initialize(false);

    EXPECT_FALSE(dlg.onDesktop);
    EXPECT_GT(dlg.accept, -1);

    EXPECT_TRUE(dlg.message().startsWith("The desktop services"));
}

TEST(ProcessDialog, initialize_dekstop)
{
    ProcessDialog dlg;
    dlg.initialize(true);

    EXPECT_TRUE(dlg.onDesktop);
    EXPECT_GT(dlg.accept, -1);

    EXPECT_TRUE(dlg.message().startsWith("File Manager"));
}

TEST(ProcessDialog, execDialog_fm_nopreocess)
{
    ProcessDialog dlg;
    dlg.accept = 1;
    dlg.onDesktop = false;

    QString path;

    stub_ext::StubExt stub;
    stub.set_lamda(&ProcessDialog::queryProcess, [&path](ProcessDialog *, const QString &exe){
        path = exe;
        return QList<int>();
    });

    bool execed = false;
    stub.set_lamda(VADDR(ProcessDialog, exec), [&execed](){
        execed = true;
        return 0;
    });

    EXPECT_TRUE(dlg.execDialog());
    EXPECT_FALSE(execed);
    EXPECT_EQ(path, QString("/usr/bin/dde-desktop"));
}

TEST(ProcessDialog, execDialog_desktop_noprocess)
{
    ProcessDialog dlg;
    dlg.accept = 1;
    dlg.onDesktop = true;

    QString path;

    stub_ext::StubExt stub;
    stub.set_lamda(&ProcessDialog::queryProcess, [&path](ProcessDialog *, const QString &exe){
        path = exe;
        return QList<int>();
    });

    bool execed = false;
    stub.set_lamda(VADDR(ProcessDialog, exec), [&execed](){
        execed = true;
        return 0;
    });

    EXPECT_TRUE(dlg.execDialog());
    EXPECT_FALSE(execed);
    EXPECT_EQ(path, QString("/usr/bin/dde-file-manager"));
}

TEST(ProcessDialog, execDialog_reject)
{
    ProcessDialog dlg;
    dlg.accept = 1;

    stub_ext::StubExt stub;
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

    EXPECT_FALSE(dlg.execDialog());
    EXPECT_TRUE(execed);
    EXPECT_TRUE(killed.isEmpty());
    EXPECT_FALSE(dlg.killed);
}

TEST(ProcessDialog, execDialog_accept)
{
    ProcessDialog dlg;
    dlg.accept = 1;

    stub_ext::StubExt stub;
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

    EXPECT_TRUE(dlg.execDialog());
    EXPECT_TRUE(execed);
    EXPECT_TRUE(dlg.killed);
    EXPECT_EQ(killed, ret);
}
