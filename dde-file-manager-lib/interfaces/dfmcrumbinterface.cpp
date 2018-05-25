/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmcrumbinterface.h"

#include "dfmcrumbitem.h"

#include "controllers/jobcontroller.h"

#include "dfileservices.h"
#include "dfileinfo.h"

#include <QPointer>

DFM_BEGIN_NAMESPACE

CrumbData::CrumbData(DUrl url, QString displayText, QString iconName, QString iconKey)
{
    this->url = url;
    this->displayText = displayText;
    this->iconName = iconName;
    this->iconKey = iconKey;
}

CrumbData::operator QString() const
{
     QString ret = "url='" + url.toString();

     if (!displayText.isEmpty()) {
        ret += "' displayText='" + displayText;
     }

     if (!iconName.isEmpty()) {
        ret += "' iconName='" + iconName;
     }

     if (!iconName.isEmpty() && !iconKey.isEmpty()) {
        ret += "' iconKey='" + iconKey;
     }

     return QStringLiteral("CrumbData(") + ret + QStringLiteral(")");
}

void CrumbData::setIconFromThemeConfig(QString iconName, QString iconKey)
{
    this->iconName = iconName;
    this->iconKey = iconKey;
}

class DFMCrumbInterfacePrivate {

public:
    DFMCrumbInterfacePrivate(DFMCrumbInterface *qq);

    QPointer<JobController> folderCompleterJobPointer;

    DFMCrumbInterface *q_ptr;

    Q_DECLARE_PUBLIC(DFMCrumbInterface)
};

DFMCrumbInterfacePrivate::DFMCrumbInterfacePrivate(DFMCrumbInterface *qq)
    : q_ptr(qq)
{

}


DFMCrumbInterface::DFMCrumbInterface(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMCrumbInterfacePrivate(this))
{

}

DFMCrumbInterface::~DFMCrumbInterface()
{

}

QList<CrumbData> DFMCrumbInterface::seprateUrl(const DUrl &url)
{
    QList<CrumbData> list;

    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    DUrlList urlList;
    urlList.append(url);
    urlList.append(info->parentUrlList());

    DAbstractFileInfoPointer infoPointer;
    // Push urls into crumb list
    DUrlList::const_reverse_iterator iter = urlList.crbegin();
    while (iter != urlList.crend()) {
        const DUrl & oneUrl = *iter;

        QString displayText = oneUrl.fileName();
        // Check for possible display text.
        infoPointer = DFileService::instance()->createFileInfo(nullptr, oneUrl);
        if (infoPointer) {
            displayText = infoPointer->fileDisplayName();
        }
        CrumbData data(oneUrl, displayText);
        list.append(data);

        iter++;
    }

    return list;
}

DFMCrumbItem *DFMCrumbInterface::createCrumbItem(const CrumbData &data)
{
    return new DFMCrumbItem(data);
}

void DFMCrumbInterface::requestCompletionList(const DUrl &url)
{
    Q_D(DFMCrumbInterface);

    if (d->folderCompleterJobPointer) {
        d->folderCompleterJobPointer->disconnect();
        d->folderCompleterJobPointer->stopAndDeleteLater();
    }

    d->folderCompleterJobPointer = DFileService::instance()->getChildrenJob(this, url, QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
    if (!d->folderCompleterJobPointer) {
        return;
    }

    connect(d->folderCompleterJobPointer, &JobController::addChildrenList, this, [this](const QList<DAbstractFileInfoPointer> &infoList){
        QStringList list;
        for (const DAbstractFileInfoPointer &infoPointer : infoList) {
            list.append(infoPointer->fileName());
        }
        emit completionFound(list);
    }, Qt::DirectConnection);

    connect(d->folderCompleterJobPointer, &JobController::finished, this, [this](){
        emit completionListTransmissionCompleted();
    }, Qt::QueuedConnection);

    d->folderCompleterJobPointer->setTimeCeiling(1000);
    d->folderCompleterJobPointer->setCountCeiling(100);
    d->folderCompleterJobPointer->start();
}

void DFMCrumbInterface::cancelCompletionListTransmission()
{
    Q_D(DFMCrumbInterface);

    if (d->folderCompleterJobPointer && d->folderCompleterJobPointer) {
        d->folderCompleterJobPointer->stopAndDeleteLater();
    }
}

DFM_END_NAMESPACE
