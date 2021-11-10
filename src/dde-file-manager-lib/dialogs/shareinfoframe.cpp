/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "shareinfoframe.h"
#include "propertydialog.h"
#include "usershare/shareinfo.h"
#include "usershare/usersharemanager.h"
#include "singleton.h"
#include "app/define.h"
#include "dfileservices.h"
#include "dialogmanager.h"

#include <QStandardPaths>
#include <QFormLayout>
#include <QProcess>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QAbstractItemView>
#include <unistd.h>

ShareInfoFrame::ShareInfoFrame(const DAbstractFileInfoPointer &info, QWidget *parent) :
    QFrame(parent),
    m_fileinfo(info)
{
//    m_jobTimer = new QTimer();
//    m_jobTimer->setInterval(1000);
//    ->setSingleShot(true);
    initUI();
    updateShareInfo(m_fileinfo->absoluteFilePath());
    initConnect();
}

void ShareInfoFrame::initUI()
{
    int labelWidth = 100;
    int fieldWidth = 160;

    m_shareCheckBox = new QCheckBox(this);
    m_shareCheckBox->setFixedWidth(fieldWidth);
    m_shareCheckBox->setText(tr("Share this folder"));

    QWidget *centerAlignContainer = new QWidget();
    QHBoxLayout *centerAlignLayout = new QHBoxLayout(centerAlignContainer);
    centerAlignLayout->addWidget(m_shareCheckBox);
    centerAlignLayout->setAlignment(Qt::AlignCenter);
    centerAlignLayout->setContentsMargins(0, 0, 0, 0);
    centerAlignContainer->setLayout(centerAlignLayout);

    SectionKeyLabel *shareNameLabel = new SectionKeyLabel(tr("Share name:"));
    shareNameLabel->setFixedWidth(labelWidth);
    m_shareNamelineEdit = new QLineEdit(this);
    m_shareNamelineEdit->setObjectName("ShareNameEdit");
    m_shareNamelineEdit->setText(m_fileinfo->fileDisplayName());
    m_shareNamelineEdit->setFixedWidth(fieldWidth);
    // sp3需求 共享文件名设置限制
    // 设置只能输入大小写字母、数字和部分符号的正则表达式
    QRegExp regx("^[^\\s+\\[\\]\"'/\\\\:|<>+=;,?*\r\n\t]*$");
    // 创建验证器
    QValidator *validator = new QRegExpValidator(regx, this);
    m_shareNamelineEdit->setValidator(validator);

    SectionKeyLabel *permissionLabel = new SectionKeyLabel(tr("Permission:"));
    permissionLabel->setFixedWidth(labelWidth);
    m_permissoComBox = new QComboBox(this);
    m_permissoComBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    m_permissoComBox->setFixedWidth(fieldWidth);
    QStringList permissions;
    permissions << tr("Read and write") << tr("Read only");
    m_permissoComBox->addItems(permissions);

    SectionKeyLabel *anonymityLabel = new SectionKeyLabel(tr("Anonymous:"));
    anonymityLabel->setFixedWidth(labelWidth);
    m_anonymityCombox = new QComboBox(this);
    m_anonymityCombox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    m_anonymityCombox->setFixedWidth(fieldWidth);
    QStringList anonymityChoices;
    anonymityChoices << tr("Not allow") << tr("Allow");
    m_anonymityCombox->addItems(anonymityChoices);

    QFormLayout *mainLayoyt = new QFormLayout(this);

    DFMGlobal::setToolTip(shareNameLabel);
    DFMGlobal::setToolTip(permissionLabel);
    DFMGlobal::setToolTip(anonymityLabel);

    mainLayoyt->addRow(centerAlignContainer);
    mainLayoyt->addRow(shareNameLabel, m_shareNamelineEdit);
    mainLayoyt->addRow(permissionLabel, m_permissoComBox);
    mainLayoyt->addRow(anonymityLabel, m_anonymityCombox);
    mainLayoyt->setLabelAlignment(Qt::AlignVCenter | Qt::AlignRight);
    mainLayoyt->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    mainLayoyt->setFormAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mainLayoyt->setContentsMargins(10, 10, 10, 10);
    setLayout(mainLayoyt);

    //当前文件夹已被共享
    if (userShareManager->isShareFile(m_fileinfo->filePath())) {
        auto creatorShareUid = userShareManager->getCreatorUidByShareName(m_fileinfo->fileSharedName().toLower());
        //文件共享创建者不是当前process的打开者或者不是文件所有者 排除root用户
        if ((creatorShareUid != getuid() || creatorShareUid != m_fileinfo->ownerId())
                && getuid() != 0) {
            this->setEnabled(false);
        }
    }

    //判断文件属主与进程属主是否相同，排除进程属主为根用户情况
    if (m_fileinfo->ownerId() != getuid() && getuid() != 0) {
        this->setEnabled(false);
//        m_shareCheckBox->setEnabled(false);
//        m_shareNamelineEdit->setEnabled(false);
//        m_permissoComBox->setEnabled(false);
//        m_anonymityCombox->setEnabled(false);
    }
}

void ShareInfoFrame::initConnect()
{
    connect(m_shareCheckBox, &QCheckBox::clicked, this, &ShareInfoFrame::handleCheckBoxChanged);
//    connect(m_shareNamelineEdit, &QLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);
    connect(m_shareNamelineEdit, &QLineEdit::editingFinished, this, &ShareInfoFrame::handleShareNameFinished);
    //connect(m_shareNamelineEdit, &QLineEdit::returnPressed, this, [ = ]() {qDebug() << "回车按下";}); //不知为何没有发送returnPressed信号
    connect(m_permissoComBox, SIGNAL(currentIndexChanged(int)), this, SLOT(handlePermissionComboxChanged(int)));
    connect(m_anonymityCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(handleAnonymityComboxChanged(int)));
    //connect(m_jobTimer, &QTimer::timeout, this, &ShareInfoFrame::doShareInfoSetting);
    connect(userShareManager, &UserShareManager::userShareAdded, this, &ShareInfoFrame::updateShareInfo);
    connect(userShareManager, &UserShareManager::userShareDeleted, this, &ShareInfoFrame::updateShareInfo);
    connect(userShareManager, &UserShareManager::userShareDeletedFailed, this, &ShareInfoFrame::updateShareInfo);
}

void ShareInfoFrame::handleCheckBoxChanged(const bool &checked)
{
    //为了避免在高速点击时引发ui错乱，只有在share流程完全结束后才允许用户再次修改共享状态
    m_shareCheckBox->setEnabled(false);
    bool ret = doShareInfoSetting();

    if (ret) {
        if (checked) {
//            emit folderShared(m_fileinfo->absoluteFilePath());
            activateWidgets();
        }
    } else {
        m_shareCheckBox->setChecked(false);
        m_shareCheckBox->setEnabled(true);
        disactivateWidgets();
    }
}

void ShareInfoFrame::handleShareNameFinished()
{
    // 修复bug-54080
    // 当失去焦点时，判断文件名是否符合规则
    if (!m_shareNamelineEdit->hasFocus())
        doShareInfoSetting();
    else    // 如果焦点存在，将焦点设置到下一个控件
        m_permissoComBox->setFocus();
    //handShareInfoChanged();
}

void ShareInfoFrame::handleShareNameChanged(const QString &str)
{
    // fix bug 69970 与文件名规则保持一致
    // 采用validator过滤，屏蔽信号
    QString dstText = DFMGlobal::preprocessingFileName(str);
    if (str != dstText) {
        QSignalBlocker blocker(m_shareNamelineEdit);

        int currPos = m_shareNamelineEdit->cursorPosition();
        m_shareNamelineEdit->setText(dstText);
        currPos += dstText.length() - str.length();
        m_shareNamelineEdit->setText(dstText);
        m_shareNamelineEdit->setCursorPosition(currPos);
    }
}

void ShareInfoFrame::handlePermissionComboxChanged(const int &index)
{
    Q_UNUSED(index);
    doShareInfoSetting();
}

void ShareInfoFrame::handleAnonymityComboxChanged(const int &index)
{
    Q_UNUSED(index);
    doShareInfoSetting();
}

void ShareInfoFrame::handShareInfoChanged()
{
    //m_jobTimer->start();
}

bool ShareInfoFrame::doShareInfoSetting()
{
    if (m_shareCheckBox->isChecked()) { //判断是否为添加共享
        if (!checkShareName()) { //检查共享名
            return false;
        }
    } else {
        emit unfolderShared();
//        hide();
        return DFileService::instance()->unShareFolder(this, m_fileinfo->fileUrl());
    }
    // fix bug#51124 只读共享文件夹，修改“匿名访问”，此文件夹权限改变
    // 用户权限保持不变，修改组、其他权限为可读写
    if (m_permissoComBox->currentIndex() == 0 && m_anonymityCombox->currentIndex() != 0) {
        DUrl localUrl = DUrl::fromLocalFile(m_fileinfo->fileUrl().toLocalFile());
        fileService->setPermissions(nullptr, localUrl, m_fileinfo->permissions()
                                    | QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther
                                    | QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup);
    }

    bool ret = DFileService::instance()->shareFolder(this, m_fileinfo->fileUrl(), m_shareNamelineEdit->text(),
                                                     m_permissoComBox->currentIndex() == 0,
                                                     m_anonymityCombox->currentIndex() != 0);

    //修改用户目录其他权限为可执行
    //该权限修改逻辑只针对普通用户共享自己的文件时有效
    //root用户共享的行为不主动修改目录权限，既共享时不修改/root的其他执行权限和普通用户主目录的其他执行权限
    if (ret && m_anonymityCombox->currentIndex() != 0 && getuid() != 0) {
        DUrl userUrl = DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        if (m_fileinfo->path().startsWith(userUrl.path())) {
            DAbstractFileInfoPointer userFileInfo = fileService->createFileInfo(this, userUrl);
            if (userFileInfo && userFileInfo->exists()
                    && (userFileInfo->permissions() & QFileDevice::ExeOther) != QFileDevice::ExeOther)
                fileService->setPermissions(this, userUrl, userFileInfo->permissions() | QFileDevice::ExeOther);
        }
    }

    return ret;
}

void ShareInfoFrame::updateShareInfo(const QString &filePath)
{
    //为了避免在高速点击时引发ui错乱，只有在share流程完全结束后才允许用户再次修改共享状态
    m_shareCheckBox->setEnabled(true);
    if (filePath != m_fileinfo->absoluteFilePath())
        return;

    if (!m_fileinfo->fileSharedName().isEmpty()) {
        m_shareCheckBox->setChecked(true);
        activateWidgets();
        //disconnect(m_shareNamelineEdit, &QLineEdit::editingFinished, this, &ShareInfoFrame::handleShareNameChanged);
        //int cursorPos = m_shareNamelineEdit->cursorPosition();
        m_shareNamelineEdit->setText(m_fileinfo->fileSharedName());
        //m_shareNamelineEdit->setCursorPosition(cursorPos);
        //connect(m_shareNamelineEdit, &QLineEdit::editingFinished, this, &ShareInfoFrame::handleShareNameChanged);
        if (m_fileinfo->isWritableShared()) {
            m_permissoComBox->setCurrentIndex(0);
        } else {
            m_permissoComBox->setCurrentIndex(1);
        }

        if (m_fileinfo->isAllowGuestShared()) {
            m_anonymityCombox->setCurrentIndex(1);
        } else {
            m_anonymityCombox->setCurrentIndex(0);
        }
    } else {
        m_shareCheckBox->setChecked(false);
        m_permissoComBox->setCurrentIndex(0);
        m_anonymityCombox->setCurrentIndex(0);

        const QString share_name = m_fileinfo->fileDisplayName().remove(QRegExp("[%<>*?|\\\\+=;:\",]"));

        m_shareNamelineEdit->setText(share_name);

        disactivateWidgets();
    }
}

void ShareInfoFrame::activateWidgets()
{
    //m_shareNamelineEdit->setEnabled(true);
    m_permissoComBox->setEnabled(true);
    m_anonymityCombox->setEnabled(true);
}

void ShareInfoFrame::disactivateWidgets()
{
//    m_shareNamelineEdit->setEnabled(false);
    m_permissoComBox->setEnabled(false);
    m_anonymityCombox->setEnabled(false);
}

void ShareInfoFrame::setFileinfo(const DAbstractFileInfoPointer &fileinfo)
{
    m_fileinfo = fileinfo;
    updateShareInfo(m_fileinfo->absoluteFilePath());
}

bool ShareInfoFrame::checkShareName() //返回值表示是否继续
{
    // 共享名不能为空
    const QString &name = m_shareNamelineEdit->text();
    if (name.isEmpty() || name == "") {
        return false;
    }
    // 修复BUG-44972
    // 当共享文件名为“..”或“.”时，弹出提示框
    if (name == ".." || name == ".") {
        QString strMsg = tr("The share name must not be two dots (..) or one dot (.)");
        dialogManager->showMessageDialog(DialogManager::msgWarn, strMsg);
        return false;
    }

    if (m_fileinfo->fileSharedName().toLower() == m_shareNamelineEdit->text().toLower()) { //共享名未更改（不区分大小写）时，直接返回true
        return true;
    }
    QDir d("/var/lib/samba/usershares"); //该目录存放了通过程序net共享的共享信息，文件名是共享名,文件名统一小写
    QFileInfoList infolist = d.entryInfoList(QDir::Files); //读取/var/lib/samba/usershares目录下文件信息
    for (QFileInfo info : infolist) {
        if (m_shareNamelineEdit->text().toLower() == info.fileName()) { //查询共享名是否重复，因为程序net保存的文件名统一小写，所以先将共享名转为小写判断
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));

            if (!info.isWritable()) { //不可则无法替换
                dialog.setTitle(tr("The share name is used by another user."));
                dialog.addButton(tr("OK","button"), true);
            } else { //可写则添加替换按钮
                dialog.setTitle(tr("The share name already exists. Do you want to replace the shared folder?"));
                dialog.addButton(tr("Cancel","button"), true);
                dialog.addButton(tr("Replace","button"), false, DDialog::ButtonWarning);
            }

            if (dialog.exec() != DDialog::Accepted) {
                //m_shareCheckBox->setChecked(false);
                m_shareNamelineEdit->setFocus(); //进入编辑
                return false;
            }
            break; //终止循环
        }
    }
    return  true;
}

ShareInfoFrame::~ShareInfoFrame()
{

}

