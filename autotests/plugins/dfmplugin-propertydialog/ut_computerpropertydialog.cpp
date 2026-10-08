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

// ===================== PMS sev-2 regression additions =====================
#include <dfm-base/utils/universalutils.h>
#include <DSysInfo>
#include <QSignalSpy>

namespace {
class Bug371305Thread : public ComputerInfoThread
{
public:
    using ComputerInfoThread::ComputerInfoThread;
    void callRun() { run(); }   // run() 为 protected，这里显式暴露
};
}   // namespace

// PMS:371305 memoryInfo 必须优先使用 UniversalUtils::computerMemory（DBus 实时值），而不是直接用 DSysInfo::memoryInstalledSize
TEST_F(ComputerPropertyDialogImpl, BUG371305_MemoryInfo_PrefersComputerMemoryDBusValue)
{
    qRegisterMetaType<QMap<ComputerInfoItem, QString>>("QMap<ComputerInfoItem, QString>");

    constexpr qint64 kDBusMemory = 34359738368LL;   // 32 GiB
    stub.set_lamda(&dfmbase::UniversalUtils::computerMemory, []() -> qint64 {
        __DBG_STUB_INVOKE__
        return kDBusMemory;
    });
    // 同时固定 DSysInfo 值，证明结果取的是 computerMemory 而非 DSysInfo
    stub.set_lamda(&Dtk::Core::DSysInfo::memoryInstalledSize, []() -> qint64 {
        __DBG_STUB_INVOKE__
        return 8589934592LL;   // 8 GiB
    });

    Bug371305Thread thread;
    QSignalSpy spy(&thread, &ComputerInfoThread::sigSendComputerInfo);
    thread.callRun();

    ASSERT_EQ(spy.count(), 1);
    const QMap<ComputerInfoItem, QString> info =
            spy.at(0).at(0).value<QMap<ComputerInfoItem, QString>>();
    // formatCap(32GiB, 1024, 0) == "32 GB"；若回退到 DSysInfo 则会是 "8 GB"
    const QString memoryStr = info.value(ComputerInfoItem::kMemory);
    EXPECT_TRUE(memoryStr.contains(QStringLiteral("32 GB"))) << memoryStr.toStdString();
}

// PMS:371305 当 UniversalUtils::computerMemory 返回 -1（DBus 不可用）时，应回退到 DSysInfo::memoryInstalledSize
TEST_F(ComputerPropertyDialogImpl, BUG371305_MemoryInfo_FallsBackWhenDBusUnavailable)
{
    qRegisterMetaType<QMap<ComputerInfoItem, QString>>("QMap<ComputerInfoItem, QString>");

    stub.set_lamda(&dfmbase::UniversalUtils::computerMemory, []() -> qint64 {
        __DBG_STUB_INVOKE__
        return -1;
    });
    stub.set_lamda(&Dtk::Core::DSysInfo::memoryInstalledSize, []() -> qint64 {
        __DBG_STUB_INVOKE__
        return 8589934592LL;   // 8 GiB → formatCap → "8 GB"
    });

    Bug371305Thread fallbackThread;
    QSignalSpy fallbackSpy(&fallbackThread, &ComputerInfoThread::sigSendComputerInfo);
    fallbackThread.callRun();
    ASSERT_EQ(fallbackSpy.count(), 1);
    const QString fallbackStr = fallbackSpy.at(0).at(0)
                                        .value<QMap<ComputerInfoItem, QString>>()
                                        .value(ComputerInfoItem::kMemory);
    EXPECT_TRUE(fallbackStr.contains(QStringLiteral("8 GB"))) << fallbackStr.toStdString();

    // 主路径恢复后应得到与回退值不同的结果（32 GB vs 8 GB）
    stub.set_lamda(&dfmbase::UniversalUtils::computerMemory, []() -> qint64 {
        __DBG_STUB_INVOKE__
        return 34359738368LL;   // 32 GiB
    });

    Bug371305Thread primaryThread;
    QSignalSpy primarySpy(&primaryThread, &ComputerInfoThread::sigSendComputerInfo);
    primaryThread.callRun();
    ASSERT_EQ(primarySpy.count(), 1);
    const QString primaryStr = primarySpy.at(0).at(0)
                                       .value<QMap<ComputerInfoItem, QString>>()
                                       .value(ComputerInfoItem::kMemory);

    EXPECT_NE(primaryStr, fallbackStr);
    EXPECT_TRUE(primaryStr.contains(QStringLiteral("32 GB"))) << primaryStr.toStdString();
}
