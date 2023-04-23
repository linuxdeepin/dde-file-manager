// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/createvaultview/vaultactivesavekeyfileview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/policy/policymanager.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>
#include <DFileChooserEdit>

#include <QShowEvent>
#include <QFileDialog>
#include <QPainter>

DPVAULT_USE_NAMESPACE
DGUI_USE_NAMESPACE

TEST(UT_VaultActiveSaveKeyFileView, slotNextBtnClicked_DefaultPath)
{
    bool isSaveKey { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::getPubKey, [] { __DBG_STUB_INVOKE__ return "abcdef"; });
    stub.set_lamda(&OperatorCenter::saveKey, [ &isSaveKey ] { __DBG_STUB_INVOKE__ isSaveKey = true; return true; });

    VaultActiveSaveKeyFileView view;
    view.defaultPathRadioBtn->setChecked(true);
    view.otherPathRadioBtn->setChecked(false);
    view.slotNextBtnClicked();
    EXPECT_TRUE(isSaveKey);
}

TEST(UT_VaultActiveSaveKeyFileView, slotNextBtnClicked_OtherPath)
{
    bool isSaveKey { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::getPubKey, [] { __DBG_STUB_INVOKE__ return "abcdef"; });
    stub.set_lamda(&OperatorCenter::saveKey, [ &isSaveKey ] { __DBG_STUB_INVOKE__ isSaveKey = true; return true; });

    VaultActiveSaveKeyFileView view;
    view.defaultPathRadioBtn->setChecked(false);
    view.otherPathRadioBtn->setChecked(true);
    view.slotNextBtnClicked();
    EXPECT_TRUE(isSaveKey);
}

TEST(UT_VaultActiveSaveKeyFileView, slotSelectRadioBtn_DefaultPathBtn)
{
    VaultActiveSaveKeyFileView view;
    view.slotSelectRadioBtn(view.defaultPathRadioBtn);
    EXPECT_FALSE(view.selectfileSavePathEdit->isEnabled());
}

TEST(UT_VaultActiveSaveKeyFileView, slotSelectRadioBtn_OtherPathBtn)
{
    VaultActiveSaveKeyFileView view;
    view.slotSelectRadioBtn(view.otherPathRadioBtn);
    EXPECT_TRUE(view.selectfileSavePathEdit->isEnabled());
}

TEST(UT_VaultActiveSaveKeyFileView, slotChangeEdit)
{
    VaultActiveSaveKeyFileView view;
    view.slotChangeEdit("ut_test89");
    EXPECT_TRUE(view.nextBtn->isEnabled());
}

TEST(UT_VaultActiveSaveKeyFileView, slotSelectCurrentFile_dir)
{
    bool isSelectFile { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&QFileDialog::selectFile, [ &isSelectFile ] { __DBG_STUB_INVOKE__ isSelectFile = true; });
    stub.set_lamda(&QFileInfo::isDir, [] { __DBG_STUB_INVOKE__ return true; });

    VaultActiveSaveKeyFileView view;
    view.slotSelectCurrentFile("/test");
    EXPECT_TRUE(isSelectFile);
}

TEST(UT_VaultActiveSaveKeyFileView, slotSelectCurrentFile_file)
{
    bool isSelectFile { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&QFileDialog::selectFile, [ &isSelectFile ] { __DBG_STUB_INVOKE__ isSelectFile = true; });
    stub.set_lamda(&QFileInfo::isDir, [] { __DBG_STUB_INVOKE__ return false; });

    VaultActiveSaveKeyFileView view;
    view.slotSelectCurrentFile("/test");
    EXPECT_TRUE(isSelectFile);
}

TEST(UT_VaultActiveSaveKeyFileView, showEvent)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::setVauleCurrentPageMark, [] { __DBG_STUB_INVOKE__ });

    VaultActiveSaveKeyFileView view;
    QShowEvent event;
    view.showEvent(&event);
    EXPECT_TRUE(view.defaultPathRadioBtn->isEnabled());
}

TEST(UT_VaultActiveSaveKeyFileView, eventFilter)
{
    bool isDraw { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&QPainter::drawPath, [ &isDraw ] { __DBG_STUB_INVOKE__ isDraw = true; });

    VaultActiveSaveKeyFileView view;
    QFrame line;
    line.setObjectName(QString("line"));
    line.setFixedHeight(1);
    QEvent event(QEvent::Paint);
    view.eventFilter(&line, &event);
    EXPECT_TRUE(isDraw);
}

TEST(UT_RadioFrame, paintEvent_Dark)
{
    bool isDraw { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&QPainter::drawPath, [ &isDraw ] { __DBG_STUB_INVOKE__ isDraw = true; });
    stub.set_lamda(&DGuiApplicationHelper::themeType, [] { __DBG_STUB_INVOKE__ return DGuiApplicationHelper::DarkType; });

    RadioFrame radio;
    QPaintEvent event(QRect(0, 0, 10, 10));
    radio.paintEvent(&event);
    EXPECT_TRUE(isDraw);
}

TEST(UT_RadioFrame, paintEvent_Light)
{
    bool isDraw { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&QPainter::drawPath, [ &isDraw ] { __DBG_STUB_INVOKE__ isDraw = true; });
    stub.set_lamda(&DGuiApplicationHelper::themeType, [] { __DBG_STUB_INVOKE__ return DGuiApplicationHelper::LightType; });


    RadioFrame radio;
    QPaintEvent event(QRect(0, 0, 10, 10));
    radio.paintEvent(&event);
    EXPECT_TRUE(isDraw);
}
