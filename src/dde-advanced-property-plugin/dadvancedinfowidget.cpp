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

#include "dadvancedinfowidget.h"

#include <QFileInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QMimeData>
#include <QFormLayout>
#include <QProcess>
#include <QLineEdit>

#include "../utils/utils.h"
#include "shutil/fileutils.h"
#include "interfaces/dfileinfo.h"
#include "interfaces/dfileservices.h"
#include "models/avfsfileinfo.h"

DAdvancedInfoWidget::DAdvancedInfoWidget(QWidget *parent, const QString &file) :
    QWidget(parent),
    m_url(QUrl::fromUserInput(file))
{
    initData();
    initUI();
}

void DAdvancedInfoWidget::initData()
{
    const DAbstractFileInfoPointer& info = DFileService::instance()->createFileInfo(this, DUrl(m_url));
    if(!info || !info->exists())
        return;
    QString realParentPath = info->parentUrl().path();
    QString realFilePath = info->filePath();
    QString realFileUri = info->fileUrl().toString();
    bool isAvfsFile = info->fileUrl().isAVFSFile();
    QProcess p;

    if(isAvfsFile){
        realParentPath = AVFSFileInfo::realFileUrl(info->fileUrl()).parentUrl().path();
        realFilePath = AVFSFileInfo::realFileUrl(info->fileUrl()).path();
        realFileUri = AVFSFileInfo::realFileUrl(info->fileUrl()).toString();
    }


    //qio mime type
    m_InfoList << QStringPair("QMimeType:", info->mimeTypeName());

    //gio mime type
    QString g_mimeType = FileUtils::getMimeTypeByGIO(realFileUri);
    m_InfoList << QStringPair("GMimeType:", g_mimeType);

    //icon name
    m_InfoList << QStringPair("Icon name:", info->iconName());

    //acess
    QString cmd = "/bin/bash";
    QStringList args;
    args << "-c" << "ls -l " + realParentPath + "|grep " + info->fileName();

    p.start(cmd, args);
    p.waitForFinished(200);
    QString access = p.readAll().constData();
    access = access.split(" ").first();
    m_InfoList << QStringPair("Access:", access);

    //size md5
    QString sizeMD5 = FileUtils::md5(QString::number(FileUtils::totalSize(info->filePath())));
    m_InfoList << QStringPair("MD5:", sizeMD5);

    //avfs real url
    if(info->fileUrl().isAVFSFile()){
        QString realPath = AVFSFileInfo::realFileUrl(info->fileUrl()).path();
        m_InfoList << QStringPair("Arch reflect path:", realPath);
    }

    //symlink real path
    if(info->isSymLink()){
        m_InfoList << QStringPair("Sym root target:", info->rootSymLinkTarget().path());
    }
}

void DAdvancedInfoWidget::initUI()
{
    m_listWidget = new QListWidget(this);
    m_listWidget->setSpacing(2);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_listWidget);
    setLayout(mainLayout);

    foreach (const QStringPair& pair, m_InfoList) {
        QFrame *frame = nullptr;
        QFormLayout *fLayout = nullptr;
        QLineEdit *valLabel = nullptr;
        QListWidgetItem *item = nullptr;

        valLabel = new QLineEdit(frame);
        valLabel->setText(pair.second);
        valLabel->setReadOnly(true);

        fLayout = new QFormLayout;
        fLayout->setContentsMargins(5, 0, 5, 0);
        fLayout->addRow(pair.first, valLabel);

        frame = new QFrame(this);
        frame->setContentsMargins(0, 0, 0, 0);
        frame->setLayout(fLayout);

        item = new QListWidgetItem;
        m_listWidget->addItem(item);
        m_listWidget->setItemWidget(item, frame);
    }

    setFixedWidth(310);
}
