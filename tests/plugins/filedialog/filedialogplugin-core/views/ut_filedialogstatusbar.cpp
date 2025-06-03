// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filedialog/filedialogplugin-core/views/filedialogstatusbar.h"

#include <gtest/gtest.h>

#include <DLineEdit>
#include <QKeyEvent>
#include <QSignalSpy>

DIALOGCORE_USE_NAMESPACE

class UT_FileDialogStatusBar : public testing::Test
{
protected:
    void SetUp() override { }
    void TearDown() override { }
};

TEST_F(UT_FileDialogStatusBar, onViewItemClicked)
{
    FileDialogStatusBar bar;
    bar.fileNameEdit->lineEdit()->setText("test1.txt");
    bar.changeFileNameEditText("test2");
    QString ss = bar.fileNameEdit->lineEdit()->text();
    EXPECT_TRUE(bar.fileNameEdit->lineEdit()->text() == "test2.txt");
}

TEST_F(UT_FileDialogStatusBar, focusAlwaysOnFileNameEdit)
{
    FileDialogStatusBar bar;
    
    // 确保文件名编辑框可见
    bar.fileNameEdit->setVisible(true);
    
    // 测试保存模式
    bar.setMode(FileDialogStatusBar::kSave);
    bar.setAppropriateWidgetFocus();
    EXPECT_TRUE(bar.fileNameEdit->hasFocus());
    
    // 测试打开模式
    bar.setMode(FileDialogStatusBar::kOpen);
    bar.setAppropriateWidgetFocus();
    EXPECT_TRUE(bar.fileNameEdit->hasFocus());
}

TEST_F(UT_FileDialogStatusBar, enterKeyTriggersAcceptButton)
{
    FileDialogStatusBar bar;
    
    // 设置接受按钮可见且可用
    bar.curAcceptButton->setVisible(true);
    bar.curAcceptButton->setEnabled(true);
    
    // 创建信号监听器
    QSignalSpy spy(bar.curAcceptButton, &QPushButton::clicked);
    
    // 模拟在文件名编辑框中按下Enter键
    emit bar.fileNameEdit->returnPressed();
    
    // 验证接受按钮的点击信号是否被触发
    EXPECT_EQ(spy.count(), 1);
}
