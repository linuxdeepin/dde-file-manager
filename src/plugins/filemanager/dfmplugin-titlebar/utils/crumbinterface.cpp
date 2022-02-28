/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "crumbinterface.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

CrumbInterface::CrumbInterface(QObject *parent)
    : QObject(parent)
{
}

void CrumbInterface::setKeepAddressBar(bool keep)
{
    keepAddr = keep;
}

void CrumbInterface::processAction(CrumbInterface::ActionType type)
{
    switch (type) {
    case kEscKeyPressed:
    case kClearButtonPressed:
        emit hideAddressBar(keepAddr);
        break;
    case kAddressBarLostFocus:
        if (!keepAddr)
            emit hideAddressBar(keepAddr);
        break;
    }
}

void CrumbInterface::crumbUrlChangedBehavior(const QUrl &url)
{
    if (keepAddr) {
        emit keepAddressBar(url);
    } else {
        emit hideAddrAndUpdateCrumbs(url);
    }
}

bool CrumbInterface::supportedUrl(const QUrl &url)
{
    if (supportedUrlFunc)
        return supportedUrlFunc(url);

    return false;
}

QList<CrumbData> CrumbInterface::seprateUrl(const QUrl &url)
{
    if (seprateUrlFunc)
        return seprateUrlFunc(url);

    // default method
    QList<CrumbData> list;
    QList<QUrl> urls;
    urls.push_back(url);
    UrlRoute::urlParentList(url, &urls);

    for (int count = urls.size() - 1; count >= 0; count--) {
        QUrl curUrl { urls.at(count) };
        QStringList pathList { curUrl.path().split("/") };
        CrumbData data { curUrl, pathList.isEmpty() ? "" : pathList.last() };
        if (UrlRoute::isRootUrl(curUrl))
            data.iconName = UrlRoute::icon(curUrl.scheme()).name();
        list.append(data);
    }

    return list;
}

/*!
 * TODO(zhangs): virtual ?
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
void CrumbInterface::requestCompletionList(const QUrl &url)
{
    if (folderCompleterJobPointer) {
        folderCompleterJobPointer->disconnect();
        folderCompleterJobPointer->stopAndDeleteLater();
        folderCompleterJobPointer->setParent(nullptr);
    }
    folderCompleterJobPointer = new TraversalDirThread(url, QStringList(),
                                                       QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    folderCompleterJobPointer->setParent(this);
    if (folderCompleterJobPointer.isNull())
        return;

    connect(folderCompleterJobPointer.data(), &TraversalDirThread::updateChildren, this,
            &CrumbInterface::onUpdateChildren, Qt::DirectConnection);

    connect(folderCompleterJobPointer.data(), &TraversalDirThread::finished, this,
            [this]() {
                emit completionListTransmissionCompleted();
            },
            Qt::QueuedConnection);
    // TODO(liyigang): imple follow interfaces
    //    folderCompleterJobPointer->setTimeCeiling(1000);
    //    folderCompleterJobPointer->setCountCeiling(500);
    folderCompleterJobPointer->start();
}

/*!
 * TODO(zhangs): virtual ?
 * \brief Cancel the started completion list transmission.
 *
 * \sa completionFound, completionListTransmissionCompleted, requestCompletionList
 */
void CrumbInterface::cancelCompletionListTransmission()
{
    if (folderCompleterJobPointer)
        folderCompleterJobPointer->stop();
}

void CrumbInterface::registewrSupportedUrlCallback(const supportedUrlCallback &func)
{
    supportedUrlFunc = func;
}

void CrumbInterface::registerSeprateUrlCallback(const seprateUrlCallback &func)
{
    seprateUrlFunc = func;
}

void CrumbInterface::onUpdateChildren(const QList<QUrl> &urlList)
{
    QStringList list;

    for (const QUrl &url : urlList) {
        auto infoPointer = InfoFactory::create<AbstractFileInfo>(url);
        if (infoPointer.data())
            list.append(infoPointer->fileName());
    }
    emit completionFound(list);
}
