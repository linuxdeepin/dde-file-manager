// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_repairdialog.cpp - RepairDialog（DDialog 子类）单元测试
// 离屏模式下驱动四个状态 UI、模拟进度与 reject 语义。

#include <gtest/gtest.h>

#include <QApplication>
#include <QtTest>
#include <QTimer>
#include <QDateTime>
#include <QAbstractButton>

#include <DDialog>
#include <DWaterProgress>

#include "stubext.h"

#include "repairdialog.h"

class RepairDialogTest : public testing::Test
{
protected:
    void SetUp() override { stub.clear(); }
    void TearDown() override
    {
        stub.clear();
        delete dialog;
        dialog = nullptr;
    }

    stub_ext::StubExt stub;
    RepairDialog *dialog { nullptr };

};

// 构造：固定尺寸 + 默认 kConfirm
TEST_F(RepairDialogTest, Construct_FixedSizeDefaultConfirm)
{
    // Arrange / Act
    dialog = new RepairDialog();

    // Assert
    EXPECT_EQ(dialog->width(), 360);
    EXPECT_EQ(dialog->height(), 250);
    EXPECT_EQ(dialog->getState(), RepairDialog::kConfirm);
}

// setDeviceInfo / setDevicePath / setErrorCode：字段与取值器
TEST_F(RepairDialogTest, Setters_StoreFieldsAndGettersReturn)
{
    // Arrange
    dialog = new RepairDialog();

    // Act
    dialog->setDeviceInfo("UT-Disk", "16 GB", "VFAT");
    dialog->setDevicePath("/dev/ut0", "/media/ut0");
    dialog->setErrorCode("E-01");

    // Assert
    EXPECT_EQ(dialog->m_deviceName, QString("UT-Disk"));
    EXPECT_EQ(dialog->m_deviceSize, QString("16 GB"));
    EXPECT_EQ(dialog->m_fsType, QString("VFAT"));
    EXPECT_EQ(dialog->devicePath(), QString("/dev/ut0"));
    EXPECT_EQ(dialog->mountPoint(), QString("/media/ut0"));
    EXPECT_EQ(dialog->m_errorCode, QString("E-01"));
}

// setState：四个状态分别构建对应 UI 与按钮
TEST_F(RepairDialogTest, SetState_AllFourStates_BuildsCorrespondingUi)
{
    // Arrange
    dialog = new RepairDialog();
    dialog->setDeviceInfo("UT-Disk", "16 GB", "VFAT");

    // Act / Assert：确认页 —— 两个按钮（取消 / 开始修复）
    dialog->setState(RepairDialog::kConfirm);
    EXPECT_EQ(dialog->getState(), RepairDialog::kConfirm);
    EXPECT_EQ(dialog->buttonCount(), 2);

    // 修复中 —— 无按钮 + 进度控件 + 模拟进度定时器启动
    dialog->setState(RepairDialog::kRepairing);
    EXPECT_EQ(dialog->getState(), RepairDialog::kRepairing);
    EXPECT_EQ(dialog->buttonCount(), 0);
    ASSERT_NE(dialog->m_progressWidget, nullptr);
    ASSERT_NE(dialog->m_progressTimer, nullptr);
    EXPECT_TRUE(dialog->m_progressTimer->isActive());

    // 成功页 —— 两个按钮（关闭 / 打开设备）
    dialog->setState(RepairDialog::kSuccess);
    EXPECT_EQ(dialog->getState(), RepairDialog::kSuccess);
    EXPECT_EQ(dialog->buttonCount(), 2);
    EXPECT_EQ(dialog->getButton(0)->text(), QString("Close"));

    // 失败页 —— 一个按钮 + 错误码标签
    dialog->setErrorCode("cannot read superblock");
    dialog->setState(RepairDialog::kFailed);
    EXPECT_EQ(dialog->getState(), RepairDialog::kFailed);
    EXPECT_EQ(dialog->buttonCount(), 1);
}

// setProgress：无效状态早退；有效范围更新进度
TEST_F(RepairDialogTest, SetProgress_UpdatesWaterProgressOnlyWhenValid)
{
    // Arrange
    dialog = new RepairDialog();

    // Act：未进入 kRepairing（无进度控件）→ 早退
    dialog->setProgress(50);
    EXPECT_EQ(dialog->m_progressWidget, nullptr);

    // Act：进入修复态后显示对话框并设置有效进度
    dialog->setState(RepairDialog::kRepairing);
    dialog->show();   // 父窗可见后进度控件才报告 isVisible
    QTest::qWait(20);
    dialog->setProgress(37);

    // Assert
    EXPECT_EQ(dialog->m_progressWidget->value(), 37);
    EXPECT_TRUE(dialog->m_progressWidget->isVisible());

    // Act：越界进度被忽略
    dialog->setProgress(150);
    EXPECT_EQ(dialog->m_progressWidget->value(), 37);
    dialog->setProgress(-5);
    EXPECT_EQ(dialog->m_progressWidget->value(), 37);
}

// reject：修复中拒绝关闭，其它状态允许
TEST_F(RepairDialogTest, Reject_BlockedWhileRepairingAllowedOtherwise)
{
    // Arrange
    dialog = new RepairDialog();
    dialog->setState(RepairDialog::kRepairing);
    dialog->show();
    QTest::qWait(20);
    ASSERT_TRUE(dialog->isVisible());

    // Act：修复中 reject → 窗口保留
    dialog->reject();
    EXPECT_TRUE(dialog->isVisible());

    // Act：成功态 reject → 正常关闭
    dialog->setState(RepairDialog::kSuccess);
    dialog->reject();
    EXPECT_FALSE(dialog->isVisible());
}

// startSimulatedProgress：定时器复用 + 启动动画
TEST_F(RepairDialogTest, StartSimulatedProgress_ReusesTimerAndStartsAnimation)
{
    // Arrange
    dialog = new RepairDialog();
    dialog->setState(RepairDialog::kRepairing);   // 自动启动模拟进度
    QTimer *timerBefore = dialog->m_progressTimer;
    ASSERT_TRUE(timerBefore->isActive());

    // Act：再次启动 → 复用同一定时器
    dialog->startSimulatedProgress();

    // Assert
    EXPECT_EQ(dialog->m_progressTimer, timerBefore);
    EXPECT_TRUE(timerBefore->isActive());
    EXPECT_TRUE(dialog->m_progressWidget->property("running").toBool());
}

// updateSimulatedProgress：缓动段 / 对数段 / 60 分钟封顶段
TEST_F(RepairDialogTest, UpdateSimulatedProgress_AllGrowthPhases_Bounded)
{
    // Arrange
    dialog = new RepairDialog();
    dialog->setState(RepairDialog::kRepairing);

    // Act / Assert：起始段（easeOutQuad）
    dialog->m_progressStartTime = QDateTime::currentMSecsSinceEpoch() - 1000;   // 1s
    dialog->updateSimulatedProgress();
    EXPECT_GE(dialog->m_progressWidget->value(), 0);
    EXPECT_LE(dialog->m_progressWidget->value(), 3);   // t=1/120 极小

    // Act / Assert：对数增长段（> 2 分钟）
    dialog->m_progressStartTime = QDateTime::currentMSecsSinceEpoch() - 180 * 1000;
    dialog->updateSimulatedProgress();
    EXPECT_GE(dialog->m_progressWidget->value(), 90);
    EXPECT_LT(dialog->m_progressWidget->value(), 99);

    // Act / Assert：超过 60 分钟 → 封顶 99 并停止定时器
    dialog->m_progressStartTime = QDateTime::currentMSecsSinceEpoch() - 3700 * 1000;
    dialog->updateSimulatedProgress();
    EXPECT_EQ(dialog->m_progressWidget->value(), 99);
    EXPECT_FALSE(dialog->m_progressTimer->isActive());
}

// easeOutQuad：缓动函数值
TEST_F(RepairDialogTest, EaseOutQuad_KnownValues)
{
    // Arrange
    dialog = new RepairDialog();

    // Act / Assert
    EXPECT_DOUBLE_EQ(dialog->easeOutQuad(0.0), 0.0);
    EXPECT_DOUBLE_EQ(dialog->easeOutQuad(0.5), 0.75);
    EXPECT_DOUBLE_EQ(dialog->easeOutQuad(1.0), 1.0);
}
