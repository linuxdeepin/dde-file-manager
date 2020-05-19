/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include <QFormLayout>
#include <QProcess>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

ShareInfoFrame::ShareInfoFrame(const DAbstractFileInfoPointer &info, QWidget *parent) :
    QFrame(parent),
    m_fileinfo(info)
{
    m_jobTimer = new QTimer();
    m_jobTimer->setInterval(1000);
    m_jobTimer->setSingleShot(true);
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
    connect(m_shareNamelineEdit, &QLineEdit::cursorPositionChanged, this,  [ = ]() {
        QPalette palette;
        palette.setColor(QPalette::Text, Qt::black);
        m_shareNamelineEdit->setPalette(palette); //将字体设置为黑色
    });

    SectionKeyLabel *permissionLabel = new SectionKeyLabel(tr("Permission:"));
    permissionLabel->setFixedWidth(labelWidth);
    m_permissoComBox = new QComboBox(this);
    m_permissoComBox->setFixedWidth(fieldWidth);
    QStringList permissions;
    permissions << tr("Read and write") << tr("Read only");
    m_permissoComBox->addItems(permissions);

    SectionKeyLabel *anonymityLabel = new SectionKeyLabel(tr("Anonymous:"));
    anonymityLabel->setFixedWidth(labelWidth);
    m_anonymityCombox = new QComboBox(this);
    m_anonymityCombox->setFixedWidth(fieldWidth);
    QStringList anonymityChoices;
    anonymityChoices << tr("Not allow") << tr("Allow");
    m_anonymityCombox->addItems(anonymityChoices);

    QFormLayout *mainLayoyt = new QFormLayout(this);

    mainLayoyt->addRow(centerAlignContainer);
    mainLayoyt->addRow(shareNameLabel, m_shareNamelineEdit);
    mainLayoyt->addRow(permissionLabel, m_permissoComBox);
    mainLayoyt->addRow(anonymityLabel, m_anonymityCombox);
    mainLayoyt->setLabelAlignment(Qt::AlignVCenter | Qt::AlignRight);
    mainLayoyt->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    mainLayoyt->setFormAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mainLayoyt->setContentsMargins(10, 10, 10, 10);
    setLayout(mainLayoyt);
}

void ShareInfoFrame::initConnect()
{
    connect(m_shareCheckBox, &QCheckBox::clicked, this, &ShareInfoFrame::handleCheckBoxChanged);
    connect(m_shareNamelineEdit, &QLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);
    connect(m_permissoComBox, SIGNAL(currentIndexChanged(int)), this, SLOT(handlePermissionComboxChanged(int)));
    connect(m_anonymityCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(handleAnonymityComboxChanged(int)));
    connect(m_jobTimer, &QTimer::timeout, this, &ShareInfoFrame::doShareInfoSetting);
    connect(userShareManager, &UserShareManager::userShareAdded, this, &ShareInfoFrame::updateShareInfo);
    connect(userShareManager, &UserShareManager::userShareDeleted, this, &ShareInfoFrame::updateShareInfo);
    connect(userShareManager, &UserShareManager::userShareDeletedFailed, this, &ShareInfoFrame::updateShareInfo);
}

void ShareInfoFrame::handleCheckBoxChanged(const bool &checked)
{
    if (checked) { //判断是否添加共享
        QProcess process;
        process.setProgram("net");
        process.setArguments(QStringList() << "usershare" << "info" << m_shareNamelineEdit->text());
        process.start();
        process.waitForFinished(-1);
        QString shareState = process.readAll();
        if (!shareState.isEmpty()) { //判断共享名是否重复
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"), QSize(64, 64));
            dialog.setTitle(tr("Share name %1 already exists, do you want to replace it?").arg(m_shareNamelineEdit->text()));
            dialog.addButton(tr("Cancel"), true);
            dialog.addButton(tr("Replace"), false, DDialog::ButtonWarning);
            if (dialog.exec() != DDialog::Accepted) {
                m_shareCheckBox->setChecked(false);
                QPalette palette;
                palette.setColor(QPalette::Text, Qt::red);
                m_shareNamelineEdit->setPalette(palette); //将字体设置为红色
                return;
            }
        }
    }

    bool ret = doShareInfoSetting();

    if (ret) {
        if (checked) {
            emit folderShared(m_fileinfo->absoluteFilePath());
            activateWidgets();
        }
    } else {
        m_shareCheckBox->setChecked(false);
        disactivateWidgets();
    }
}

void ShareInfoFrame::handleShareNameChanged(const QString &name)
{
    if (name.isEmpty() || name == "") {
        m_jobTimer->stop();
        return;
    }
    handShareInfoChanged();
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
    m_jobTimer->start();
}

bool ShareInfoFrame::doShareInfoSetting()
{
    if (!m_shareCheckBox->isChecked()) {
        return DFileService::instance()->unShareFolder(this, m_fileinfo->fileUrl());
    }

    if (m_permissoComBox->currentIndex() == 0 && m_anonymityCombox->currentIndex() != 0) {
        QString cmd = "chmod";
        QStringList args;
        args << "777" << m_fileinfo->fileUrl().toLocalFile();
        QProcess::startDetached(cmd, args);
        qDebug() << cmd << args;
    }

    bool ret = DFileService::instance()->shareFolder(this, m_fileinfo->fileUrl(), m_shareNamelineEdit->text(),
                                                     m_permissoComBox->currentIndex() == 0,
                                                     m_anonymityCombox->currentIndex() != 0);
    return ret;
}

void ShareInfoFrame::updateShareInfo(const QString &filePath)
{
    if (filePath != m_fileinfo->absoluteFilePath())
        return;

    if (!m_fileinfo->fileSharedName().isEmpty()) {
        m_shareCheckBox->setChecked(true);
        disconnect(m_shareNamelineEdit, &QLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);
        int cursorPos = m_shareNamelineEdit->cursorPosition();
        m_shareNamelineEdit->setText(m_fileinfo->fileSharedName());
        m_shareNamelineEdit->setCursorPosition(cursorPos);
        connect(m_shareNamelineEdit, &QLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);

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
    m_shareNamelineEdit->setEnabled(true);
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

ShareInfoFrame::~ShareInfoFrame()
{

}

