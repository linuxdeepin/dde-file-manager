/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "dfmcrumbbar.h"

#include "controllers/jobcontroller.h"

#include "dfileservices.h"
#include "dfileinfo.h"

#include <QPointer>

DFM_BEGIN_NAMESPACE

/*!
 * \class CrumbData
 * \inmodule dde-file-manager-lib
 *
 * \brief CrumbData is a class which providing data for DFMCrumbInterface
 *
 * \sa DFMCrumbInterface
 */

/*!
 * \fn CrumbData::CrumbData(DUrl url, QString displayText, QString iconName, QString iconKey)
 *
 * \brief Construct function of CrumbData
 *
 * \param url url of a crumb item
 * \param displayText display text of a crumb item
 * \param iconName the icon name of a crumb item (if it have)
 * \param iconKey the icon key of a crumb item
 */
CrumbData::CrumbData(DUrl url, QString displayText, QString iconName, QString iconKey)
    : url(url)
    , iconName(iconName)
    , displayText(displayText)
    , iconKey(iconKey)
{

}


class DFMCrumbInterfacePrivate
{

public:
    explicit DFMCrumbInterfacePrivate(DFMCrumbInterface *qq);

    QPointer<JobController> folderCompleterJobPointer;
    DFMCrumbBar *crumbBar = nullptr;

    DFMCrumbInterface *q_ptr;

    Q_DECLARE_PUBLIC(DFMCrumbInterface)
};

DFMCrumbInterfacePrivate::DFMCrumbInterfacePrivate(DFMCrumbInterface *qq)
    : q_ptr(qq)
{

}


/*!
 * \class DFMCrumbInterface
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMCrumbInterface is the interface for crumb item management of DFMCrumbBar
 *
 * DFMCrumbInterface is the interface for crumb item management for DFMCrumbBar, and should
 * be implemented as a controller. Each different URL scheme got a corresponding controller
 * for creating crumb items. DFMCrumbManager holds all avaliable crumb controllers.
 *
 * DFMCrumbInterface provides all neccessary interfaces for crumb management and some URL
 * processing utils functions.
 *
 * \sa DFMCrumbBar, DFMCrumbManager
 */

DFMCrumbInterface::DFMCrumbInterface(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMCrumbInterfacePrivate(this))
{

}

DFMCrumbInterface::~DFMCrumbInterface()
{

}

void DFMCrumbInterface::processAction(DFMCrumbInterface::ActionType type)
{
    switch (type) {
    case EscKeyPressed:
    case ClearButtonPressed:
    case AddressBarLostFocus:
        crumbBar()->hideAddressBar();
        break;
    default:
        break;
    }
}

/*!
 * \brief The crumb bar behavior when crumb bar url changed.
 *
 * \param url The url which crumb bar changed to.
 */
void DFMCrumbInterface::crumbUrlChangedBehavior(const DUrl url)
{
    crumbBar()->hideAddressBar();
    crumbBar()->updateCrumbs(url);
}

/*!
 * \brief Get the crumb bar instance.
 *
 * \return The crumb bar instance
 */
DFMCrumbBar *DFMCrumbInterface::crumbBar()
{
    Q_D(DFMCrumbInterface);

    return d->crumbBar;
}

/*!
 * \brief Set the crumb bar instance
 *
 * \param crumbBar The crumb bar instance pointer
 */
void DFMCrumbInterface::setCrumbBar(DFMCrumbBar *crumbBar)
{
    Q_D(DFMCrumbInterface);

    d->crumbBar = crumbBar;
}

/*!
 * \brief Seprate Url into a list of CrumbData for creating crumb items.
 *
 * \param url The url to be seprated.
 *
 * \return a list of CrumbData for creating crumb items.
 */
QList<CrumbData> DFMCrumbInterface::seprateUrl(const DUrl &url)
{
    QList<CrumbData> list;

    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    if (!info) {
        // blumia: it's possible if the url is not a file url at all, so we are not able
        //         to create a fileinfo for a non-file url. In this case we just return
        //         the empty list.
        return list;
    }

    DUrlList urlList;
    urlList.append(url);
    urlList.append(info->parentUrlList());

    DAbstractFileInfoPointer infoPointer;
    // Push urls into crumb list
    DUrlList::const_reverse_iterator iter = urlList.crbegin();
    while (iter != urlList.crend()) {
        const DUrl &oneUrl = *iter;

        QString displayText = oneUrl.fileName();
        // Check for possible display text.
        infoPointer = DFileService::instance()->createFileInfo(nullptr, oneUrl);
        if (infoPointer) {
            displayText = infoPointer->fileDisplayName();
        }
        CrumbData data(oneUrl, displayText);
        list.append(data);

        ++iter;
    }

    return list;
}

/*!
 * \brief Start request a completion list for address bar auto-completion.
 *
 * \param url The base url need to be completed.
 *
 * Since completion list can be long, so we need do async completion. Calling this
 * function will start a completion request and the completion list item will be sent
 * via signal completionFound. When user no longer need current completion list and
 * the transmission isn't completed, you should call cancelCompletionListTransmission.
 * When transmission completed, it will send completionListTransmissionCompleted signal.
 *
 * \sa completionFound, completionListTransmissionCompleted, cancelCompletionListTransmission
 */
void DFMCrumbInterface::requestCompletionList(const DUrl &url)
{
    Q_D(DFMCrumbInterface);

    if (d->folderCompleterJobPointer) {
        d->folderCompleterJobPointer->disconnect();
        d->folderCompleterJobPointer->stopAndDeleteLater();
        d->folderCompleterJobPointer->setParent(nullptr);
    }

    d->folderCompleterJobPointer = DFileService::instance()->getChildrenJob(this, url, QStringList(), QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags, true, false);
    d->folderCompleterJobPointer->setParent(this);
    if (!d->folderCompleterJobPointer) {
        return;
    }

    connect(d->folderCompleterJobPointer, &JobController::addChildrenList, this, [this](const QList<DAbstractFileInfoPointer> &infoList) {
        QStringList list;
        for (const DAbstractFileInfoPointer &infoPointer : infoList) {
            if (infoPointer.data())
                list.append(infoPointer->fileName());
        }
        emit completionFound(list);
    }, Qt::DirectConnection);

    connect(d->folderCompleterJobPointer, &JobController::finished, this, [this]() {
        emit completionListTransmissionCompleted();
    }, Qt::QueuedConnection);

    d->folderCompleterJobPointer->setTimeCeiling(1000);
    d->folderCompleterJobPointer->setCountCeiling(500);
    d->folderCompleterJobPointer->start();
}

/*!
 * \brief Cancel the started completion list transmission.
 *
 * \sa completionFound, completionListTransmissionCompleted, requestCompletionList
 */
void DFMCrumbInterface::cancelCompletionListTransmission()
{
    Q_D(DFMCrumbInterface);

    if (d->folderCompleterJobPointer) {
        d->folderCompleterJobPointer->stop();
    }
}

/*!
 * \fn virtual bool DFMCrumbInterface::supportedUrl(DUrl url)
 *
 * \brief Check if the given \a url is supported in this controller.
 */

DFM_END_NAMESPACE
