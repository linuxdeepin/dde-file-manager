/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "permissionmanagerwidget.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfilehandler.h"

#include <DLabel>

#include <QStorageInfo>
#include <QComboBox>
#include <QCheckBox>
#include <QFormLayout>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

PermissionManagerWidget::PermissionManagerWidget(QWidget *parent)
    : DArrowLineDrawer(parent)
{
    initUI();
}

PermissionManagerWidget::~PermissionManagerWidget()
{
}

void PermissionManagerWidget::selectFileUrl(const QUrl &url)
{
    selectUrl = url;

    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(selectUrl);

    QUrl parentUrl = info->parentUrl();
    QStorageInfo storageInfo(parentUrl.toLocalFile());
    const QString &fsType = storageInfo.fileSystemType();

    if (info->isFile()) {
        // append `Executable` string
        QString append = QStringLiteral(" , ") + QObject::tr("Executable");
        authorityList[3] += append;
        authorityList[5] += append;
        authorityList[7] += append;

        // file: read is read, read-write is read-write
        readOnlyIndex = readOnlyFlag;
        readWriteIndex = readWriteFlag;
    }

    if (info->isDir()) {
        // folder: read is read and executable, read-write is read-write and executable
        readOnlyIndex = readOnlyWithXFlag;
        readWriteIndex = readWriteWithXFlag;
        executableCheckBox->hide();
    }

    ownerComboBox->addItem(authorityList[readWriteIndex], QVariant(QFile::WriteOwner | QFile::ReadOwner));
    ownerComboBox->addItem(authorityList[readOnlyIndex], QVariant(QFile::ReadOwner));
    groupComboBox->addItem(authorityList[readWriteIndex], QVariant(QFile::WriteGroup | QFile::ReadGroup));
    groupComboBox->addItem(authorityList[readOnlyIndex], QVariant(QFile::ReadGroup));
    otherComboBox->addItem(authorityList[readWriteIndex], QVariant(QFile::WriteOther | QFile::ReadOther));
    otherComboBox->addItem(authorityList[readOnlyIndex], QVariant(QFile::ReadOther));

    setComboBoxByPermission(ownerComboBox, info->permissions() & 0x7000, 12);
    setComboBoxByPermission(groupComboBox, info->permissions() & 0x0070, 4);
    setComboBoxByPermission(otherComboBox, info->permissions() & 0x0007, 0);

    if (info->isFile()) {
        executableCheckBox = new QCheckBox;
        executableCheckBox->setText(tr("Allow to execute as program"));
        connect(executableCheckBox, &QCheckBox::toggled, this, &PermissionManagerWidget::toggleFileExecutable);
        if (info->ownerId() != getuid()) {
            executableCheckBox->setDisabled(true);
        }

        QString filePath = info->path();
        if (/*VaultController::ins()->isVaultFile(info->path())*/ false) {   // Vault file need to use stat function to read file permission.
            QString localFile = info->absolutePath();
            struct stat buf;
            std::string stdStr = localFile.toStdString();
            stat(stdStr.c_str(), &buf);
            if ((buf.st_mode & S_IXUSR) || (buf.st_mode & S_IXGRP) || (buf.st_mode & S_IXOTH)) {
                executableCheckBox->setChecked(true);
            }
        } else {
            if (info->permission(QFile::ExeUser) || info->permission(QFile::ExeGroup) || info->permission(QFile::ExeOther)) {
                executableCheckBox->setChecked(true);
            }
        }
        // 一些文件系统不支持修改可执行权限
        if (!canChmod(info) || canChmodFileType.contains(fsType)) {
            executableCheckBox->setDisabled(true);
        }
    }

    // 置灰：
    // 1. 本身用户无权限
    // 2. 所属文件系统无权限机制
    if (info->ownerId() != getuid()/* || !canChmod(info) || fsType == "fuseblk"*/) {
        ownerComboBox->setDisabled(true);
        groupComboBox->setDisabled(true);
        otherComboBox->setDisabled(true);
    }

    // tmp: 暂时的处理
    if (fsType == "vfat") {
        groupComboBox->setDisabled(true);
        otherComboBox->setDisabled(true);
        if (info->isDir()) {
            ownerComboBox->setDisabled(true);
        }
    }
}

void PermissionManagerWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    setTitle(QString(tr("Permissions")));

    setExpand(false);

    QFormLayout *layout = new QFormLayout;

    QFrame *frame = new QFrame(this);

    authorityList << QObject::tr("Access denied")   // 0
                  << QObject::tr("Executable")   // 1
                  << QObject::tr("Write only")   // 2
                  << QObject::tr("Write only")   // 3 with x
                  << QObject::tr("Read only")   // 4
                  << QObject::tr("Read only")   // 5 with x
                  << QObject::tr("Read-write")   // 6
                  << QObject::tr("Read-write");   // 7 with x

    canChmodFileType << "vfat"
                     << "fuseblk";

    ownerComboBox = new QComboBox(this);
    ownerComboBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);

    groupComboBox = new QComboBox(this);
    groupComboBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);

    otherComboBox = new QComboBox(this);
    otherComboBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);

    executableCheckBox = new QCheckBox(this);
    executableCheckBox->setText(tr("Allow to execute as program"));

    layout->setLabelAlignment(Qt::AlignLeft);
    layout->addRow(QObject::tr("Owner"), ownerComboBox);
    layout->addRow(QObject::tr("Group"), groupComboBox);
    layout->addRow(QObject::tr("Others"), otherComboBox);
    layout->addRow(executableCheckBox);

    layout->setContentsMargins(15, 10, 30, 10);

    frame->setLayout(layout);
    setContent(frame);

    connect(ownerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PermissionManagerWidget::onComboBoxChanged);
    connect(groupComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PermissionManagerWidget::onComboBoxChanged);
    connect(otherComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PermissionManagerWidget::onComboBoxChanged);
}

QString PermissionManagerWidget::getPermissionString(int enumFlag)
{
    enumFlag &= 0x0007;
    if (enumFlag < 0 || enumFlag > 7) {
        return QStringLiteral("..what?");
    } else {
        return authorityList[enumFlag];
    }
}

void PermissionManagerWidget::setComboBoxByPermission(QComboBox *cb, int permission, int offset)
{
    int index = permission >> offset;
    if (index == readWriteFlag || index == readWriteWithXFlag) {
        cb->setCurrentIndex(0);
    } else if (index == readOnlyFlag || index == readOnlyWithXFlag) {
        cb->setCurrentIndex(1);
    } else {
        cb->addItem(getPermissionString(index), QVariant(permission));
        cb->setCurrentIndex(2);
    }
}

void PermissionManagerWidget::toggleFileExecutable(bool isChecked)
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(selectUrl);
    LocalFileHandler filehandler;
    if (isChecked) {
        filehandler.setPermissions(selectUrl, info->permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
    } else {
        filehandler.setPermissions(selectUrl, info->permissions() & ~(QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther));
    }
}

bool PermissionManagerWidget::canChmod(const AbstractFileInfoPointer &info)
{
    bool ret = true;

    //    if (info->scheme() == BURN_SCHEME || info->isGvfsMountFile())
    //        ret = false;

    if (!info->canRename() /* || !info->canManageAuth()*/)
        ret = false;

    QString path = info->filePath();
    static QRegularExpression regExp("^/run/user/\\d+/gvfs/.+$",
                                     QRegularExpression::DotMatchesEverythingOption
                                             | QRegularExpression::DontCaptureOption
                                             | QRegularExpression::OptimizeOnFirstUsageOption);
    if (regExp.match(path, 0, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption).hasMatch())
        ret = false;

    return ret;
}

void PermissionManagerWidget::onComboBoxChanged()
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(selectUrl);

    struct stat fileStat;
    QByteArray infoBytes(info->absoluteFilePath().toUtf8());
    stat(infoBytes.data(), &fileStat);
    auto preMode = fileStat.st_mode;
    int ownerFlags = ownerComboBox->currentData().toInt();
    int groupFlags = groupComboBox->currentData().toInt();
    int otherFlags = otherComboBox->currentData().toInt();
    QFile::Permissions permissions = info->permissions();
    //点击combobox都需要保持执行权限，否则将失去相关权限位
    ownerFlags |= (permissions & QFile::ExeOwner);
    groupFlags |= (permissions & QFile::ExeGroup);
    otherFlags |= (permissions & QFile::ExeOther);
    LocalFileHandler filehandler;
    filehandler.setPermissions(selectUrl, QFileDevice::Permissions(ownerFlags) |
                                       /*(info->permissions() & 0x0700) |*/
                                       QFileDevice::Permissions(groupFlags) | QFileDevice::Permissions(otherFlags));

    infoBytes = info->absoluteFilePath().toUtf8();
    stat(infoBytes.data(), &fileStat);
    auto afterMode = fileStat.st_mode;
    // 修改权限失败
    // todo 回滚权限
    if (preMode == afterMode) {
        qDebug() << "chmod failed";
    }
}
