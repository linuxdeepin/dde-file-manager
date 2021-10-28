/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dfmvaultactivesavekeyfileview.h"
#include "accessibility/ac-lib-file-manager.h"
#include "vaultglobaldefine.h"
#include "operatorcenter.h"
#include "dguiapplicationhelper.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainterPath>
#include <QFileDialog>

DGUI_USE_NAMESPACE

DFMVaultActiveSaveKeyFileView::DFMVaultActiveSaveKeyFileView(QWidget *parent) :
    QWidget(parent)
{
    initUI();
}

void DFMVaultActiveSaveKeyFileView::initUI()
{
    m_title = new DLabel(this);
    m_title->setMargin(0);
    m_title->setText(tr("Save Recovery Key"));
    m_hintMsg = new DLabel(this);
    m_hintMsg->setWordWrap(true);
    m_hintMsg->setAlignment(Qt::AlignCenter);
    m_hintMsg->setText(tr("Keep the key safe to retrieve the vault password later"));
    m_defaultPathRadioBtn = new QRadioButton(this);
    AC_SET_ACCESSIBLE_NAME(m_defaultPathRadioBtn, AC_VAULT_DEFAULT_PATH_RADIOBTN);
    m_defaultPathRadioBtn->setChecked(true);
    m_defaultPathRadioBtn->setText(tr("Save to default path"));
    m_otherPathRadioBtn = new QRadioButton;
    AC_SET_ACCESSIBLE_NAME(m_otherPathRadioBtn, AC_VAULT_OTHER_PATH_RADIOBTN);
    m_otherPathRadioBtn->setText(tr("Save to other locations"));
    m_SelectfileSavePathEdit = new DFileChooserEdit;
    AC_SET_ACCESSIBLE_NAME(m_SelectfileSavePathEdit, AC_VAULT_SELECT_FILE_SAVE_PATH_EDIT);
    m_SelectfileSavePathEdit->lineEdit()->setReadOnly(true);
    m_SelectfileSavePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
    QFileDialog * filedialog = new QFileDialog(m_SelectfileSavePathEdit, QDir::homePath(), QString("pubKey.key")/*, tr("Key File(*.key)")*/);
    filedialog->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    filedialog->setNameFilter("KEY file(*.key)");
    m_SelectfileSavePathEdit->setFileDialog(filedialog);
    m_SelectfileSavePathEdit->setDirectoryUrl(QDir::homePath());
    m_SelectfileSavePathEdit->setEnabled(false);
    m_SelectfileSavePathEdit->setFileMode(QFileDialog::FileMode::Directory);

    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(m_defaultPathRadioBtn, 1);
    group->addButton(m_otherPathRadioBtn, 2);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotRadioBtn(QAbstractButton*)));


    // 下一步按钮
    m_pNext = new QPushButton(tr("Next"), this);
    AC_SET_ACCESSIBLE_NAME(m_pNext, AC_VAULT_ACTIVE_KEY_NEXT_BUTTON);
    connect(m_pNext, &QPushButton::clicked,
            this, &DFMVaultActiveSaveKeyFileView::slotNextBtnClicked);

    RadioFrame * frame = new RadioFrame;

    QLabel * checkBoxLabel = new QLabel(frame);
    checkBoxLabel->setWordWrap(true);
    checkBoxLabel->setAlignment(Qt::AlignCenter);
    QFont font = checkBoxLabel->font();
    font.setPixelSize(10);
    checkBoxLabel->setFont(font);
    checkBoxLabel->setText(tr("The default path is invisible to other users, and the path information will not be shown."));

    QHBoxLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_defaultPathRadioBtn);

    QHBoxLayout * layout1 = new QHBoxLayout();
    layout1->setContentsMargins(24,0,15,0);
    layout1->addWidget(checkBoxLabel);

    QVBoxLayout * layout2 = new QVBoxLayout(frame);
    layout2->setContentsMargins(0,0,0,0);
    layout2->addLayout(layout);
    layout2->addLayout(layout1);

    RadioFrame * frame1 = new RadioFrame;

    QHBoxLayout * layout3 = new QHBoxLayout();
    layout3->setContentsMargins(0,0,0,0);
    layout3->addWidget(m_otherPathRadioBtn);

    QHBoxLayout * layout4 = new QHBoxLayout();
    layout4->setContentsMargins(4,4,4,4);
    layout4->addWidget(m_SelectfileSavePathEdit);

    QVBoxLayout * layout5 = new QVBoxLayout(frame1);
    layout5->setContentsMargins(0,0,0,0);
    layout5->addLayout(layout3);
    layout5->setSpacing(2);
    layout5->addLayout(layout4);

    QHBoxLayout * hlayout1 = new QHBoxLayout;
    hlayout1->setContentsMargins(0,0,0,0);
    hlayout1->addStretch(1);
    hlayout1->addWidget(m_title);
    hlayout1->addStretch(1);

    QHBoxLayout * hlayout2 = new QHBoxLayout;
    hlayout2->setContentsMargins(25,0,25,0);
    hlayout2->addWidget(m_hintMsg, 1);

    QHBoxLayout * hlayout3 = new QHBoxLayout;
    hlayout3->setContentsMargins(25,0,25,0);
    hlayout3->addWidget(frame, 1);

    QHBoxLayout * hlayout4 = new QHBoxLayout;
    hlayout4->setContentsMargins(25,0,25,0);
    hlayout4->addWidget(frame1, 1);

    QHBoxLayout * hlayout5 = new QHBoxLayout;
    hlayout5->setContentsMargins(10,0,10,0);
    hlayout5->addWidget(m_pNext, 1);

    QVBoxLayout * vlayout1 = new QVBoxLayout;
    vlayout1->setContentsMargins(0,0,0,0);
    vlayout1->addLayout(hlayout1);
    vlayout1->addLayout(hlayout2);
    vlayout1->addLayout(hlayout3);
    vlayout1->addLayout(hlayout4);
    vlayout1->addLayout(hlayout5);

    setLayout(vlayout1);
}

void DFMVaultActiveSaveKeyFileView::slotNextBtnClicked()
{
    //! 获取密钥字符串
    QString pubKey = OperatorCenter::getInstance()->getPubKey();
    if(pubKey.isEmpty()) {
        return;
    }

    bool flg = false;
    if(m_defaultPathRadioBtn->isChecked()) {
        //! 密钥保存默认路径
       QString path = VAULT_BASE_PATH + QString("/") + (RSA_PUB_KEY_FILE_NAME) + QString(".key");
       flg = OperatorCenter::getInstance()->saveKey(pubKey, path);
    }else if(m_otherPathRadioBtn->isChecked()) {
        //! 密钥保存用户指定路径
       QString path = m_SelectfileSavePathEdit->text();
       flg = OperatorCenter::getInstance()->saveKey(pubKey, path);
    }

    if(flg) {
        emit sigAccepted();
    }
}

void DFMVaultActiveSaveKeyFileView::slotRadioBtn(QAbstractButton *btn)
{
    if(btn == m_defaultPathRadioBtn) {
        m_SelectfileSavePathEdit->setEnabled(false);
    } else if(btn == m_otherPathRadioBtn) {
        m_SelectfileSavePathEdit->setEnabled(true);
    }
}

void DFMVaultActiveSaveKeyFileView::showEvent(QShowEvent *event)
{
    m_defaultPathRadioBtn->setChecked(true);
    m_SelectfileSavePathEdit->clear();
    QWidget::showEvent(event);
}

RadioFrame::RadioFrame(QFrame *parent):
    QFrame(parent)
{

}

void RadioFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        painter.setBrush(QBrush(QColor("#4c252525")));
    }else if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType){
        painter.setBrush(QBrush(QColor("#4ce6e6e6")));
    }

    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    painter.drawRoundedRect(rect, 8, 8);
    //也可用QPainterPath 绘制代替 painter.drawRoundedRect(rect, 8, 8);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 8, 8);
        painter.drawPath(painterPath);
    }
    QFrame::paintEvent(event);
}
