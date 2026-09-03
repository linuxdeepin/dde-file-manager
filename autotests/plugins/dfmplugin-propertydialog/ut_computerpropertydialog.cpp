// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <QShowEvent>
#include <QCloseEvent>

#include "stubext.h"
#include "dfmplugin_propertydialog_global.h"
#include "views/computerpropertydialog.h"

DPPROPERTYDIALOG_USE_NAMESPACE

class ComputerPropertyDialogImpl : public testing::Test
{
protected:
    void SetUp() override { stub.clear(); }
    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(ComputerPropertyDialogImpl, ConstructDestruct)
{
    ComputerPropertyDialog *dialog = new ComputerPropertyDialog();
    EXPECT_NE(dialog, nullptr);
    delete dialog;
}

TEST_F(ComputerPropertyDialogImpl, ComputerProcess)
{
    ComputerPropertyDialog dialog;
    QMap<ComputerInfoItem, QString> info;
    info.insert(ComputerInfoItem::kName, "test-pc");
    info.insert(ComputerInfoItem::kVersion, "23");
    info.insert(ComputerInfoItem::kEdition, "Pro");
    info.insert(ComputerInfoItem::kOSBuild, "12345");
    info.insert(ComputerInfoItem::kType, "64Bit");
    info.insert(ComputerInfoItem::kCpu, "x86");
    info.insert(ComputerInfoItem::kMemory, "8 GB");

    EXPECT_NO_THROW(dialog.computerProcess(info));
}

TEST_F(ComputerPropertyDialogImpl, ShowAndCloseEvent)
{
    bool started = false;
    bool stopped = false;

    stub.set_lamda(&ComputerInfoThread::startThread, [&started](ComputerInfoThread *) { started = true; });
    stub.set_lamda(&ComputerInfoThread::stopThread, [&stopped](ComputerInfoThread *) { stopped = true; });

    ComputerPropertyDialog dialog;
    QShowEvent showEvent;
    EXPECT_NO_THROW(QApplication::sendEvent(&dialog, &showEvent));
    EXPECT_TRUE(started);

    QCloseEvent closeEvent;
    EXPECT_NO_THROW(dialog.closeEvent(&closeEvent));
    EXPECT_TRUE(stopped);
}

TEST_F(ComputerPropertyDialogImpl, ComputerInfoThreadStartStop)
{
    class TestThread : public ComputerInfoThread
    {
    public:
        using ComputerInfoThread::ComputerInfoThread;
        bool runCalled = false;

    protected:
        void run() override { runCalled = true; }
    };

    TestThread thread;
    thread.startThread();
    EXPECT_TRUE(thread.wait(3000));
    EXPECT_TRUE(thread.runCalled);
    thread.stopThread();
}
